#include "logic/service_registry.h"

#include "module_handler/service_registry_module.h"

namespace apie {

std::string ServiceRegistry::moduleName()
{
	return "ServiceRegistry";
}

uint32_t ServiceRegistry::modulePrecedence()
{
	return 1;
}

ServiceRegistry::ServiceRegistry(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

apie::status::Status ServiceRegistry::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Service_Registry });
	if (!bResult)
	{
		return {apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	ServiceRegistryModule::init();

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status ServiceRegistry::start()
{
	DeclarativeBase::DBType dbType = DeclarativeBase::DBType::DBT_ConfigDb;
	DAOFactoryTypeSingleton::get().registerRequiredTable(dbType, dbt_configdb::service_node_AutoGen::getFactoryName(), dbt_configdb::service_node_AutoGen::createMethod);
	auto ptrDispatched = CtxSingleton::get().getLogicThread();
	if (ptrDispatched == nullptr)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "null ptrDispatched" };
	}

	auto requiredTableOpt = DAOFactoryTypeSingleton::get().getRequiredTable(dbType);
	if (!requiredTableOpt.has_value())
	{
		return { apie::status::StatusCode::OK, "" };
	}

	std::vector<std::string> tables;
	for (const auto& items : requiredTableOpt.value())
	{
		tables.push_back(items.first);
	}

	for (const auto& tableName : tables)
	{
		MysqlTable table;
		bool bSQL = ptrDispatched->getMySQLConnector().describeTable(tableName, table);
		if (bSQL)
		{
			auto ptrDaoBase = DAOFactoryTypeSingleton::get().createDAO(dbType, tableName);
			if (ptrDaoBase == nullptr)
			{
				std::stringstream ss;
				ss << "tableName:" << tableName << " not declare";

				return { apie::status::StatusCode::HOOK_ERROR, ss.str() };
			}

			ptrDaoBase->initMetaData(table);
			bool bResult = ptrDaoBase->isValid();
			if (!bResult)
			{
				std::stringstream ss;
				ss << "tableName:" << tableName << " isValid false";

				return { apie::status::StatusCode::HOOK_ERROR, ss.str() };
			}

			DAOFactoryTypeSingleton::get().addLoadedTable(dbType, tableName, table);
		}
		else
		{
			return { apie::status::StatusCode::HOOK_ERROR, ptrDispatched->getMySQLConnector().getError() };
		}
	}

	dbt_configdb::service_node_AutoGen node;
	auto iRealm = apie::CtxSingleton::get().getServerRealm();
	node.set_service_realm(iRealm);
	bool bResult = node.bindTable(DeclarativeBase::DBType::DBT_ConfigDb, dbt_configdb::service_node_AutoGen::getFactoryName());
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "bind table"};
	}

	std::vector<dbt_configdb::service_node_AutoGen> nodeList;
	auto status = syncLoadDbByFilter(ptrDispatched->getMySQLConnector(), node, nodeList);
	if (!status.ok())
	{
		return status;
	}

	for (const auto& elem : nodeList)
	{
		EndPoint key(elem.get_service_realm(), elem.get_service_type(), elem.get_service_id(), "");
		m_nodes[key] = elem;
	}

	auto findIte = m_nodes.find(apie::CtxSingleton::get().identify());
	if (findIte == m_nodes.end())
	{
		return { apie::status::StatusCode::HOOK_ERROR, "identify not exist" };
	}

	apie::LoadConfig<Mysql_ListenersConfig> listenConfig("listenConfig");
	bResult = listenConfig.load(findIte->second.get_listeners_config());
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid listenConfig" };
	}
	apie::CtxSingleton::get().addListeners(listenConfig);


	apie::LoadConfig<Mysql_NatsConfig> natsConfig("natsConfig");
	bResult = natsConfig.load(findIte->second.get_nats_config());
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid natsConfig" };
	}

	apie::CtxSingleton::get().addNatsConnections(natsConfig);



	m_id = "id_" + apie::CtxSingleton::get().launchTime();
	m_serviceTimeout = apie::CtxSingleton::get().getConfigs()->service_timeout;

	auto timerCb = [this]() {
		this->update();
		this->addUpdateTimer(1000);
	};
	this->m_updateTimer = apie::CtxSingleton::get().getLogicThread()->dispatcher().createTimer(timerCb);
	this->addUpdateTimer(1000);

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status ServiceRegistry::ready()
{
	ServiceRegistryModule::ready();

	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG(PIE_NOTICE, "ServerStatus|{}", ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status ServiceRegistry::exit()
{
	this->disableUpdateTimer();
	return { apie::status::StatusCode::OK, "" };
}

void ServiceRegistry::setHookReady(hook::HookPoint point)
{
	if (m_prtLoader->getHookReady(point))
	{
		return;
	}

	m_prtLoader->setHookReady(point);
}

void ServiceRegistry::addUpdateTimer(uint64_t interval)
{
	this->m_updateTimer->enableTimer(std::chrono::milliseconds(interval));
}

void ServiceRegistry::disableUpdateTimer()
{
	this->m_updateTimer->disableTimer();
}

std::map<uint64_t, RegisteredEndPoint>& ServiceRegistry::registered()
{
	return m_registered;
}

std::optional<dbt_configdb::service_node_AutoGen> ServiceRegistry::findNode(EndPoint key)
{
	auto findIte = m_nodes.find(key);
	if (findIte == m_nodes.end())
	{
		return std::nullopt;
	}

	return findIte->second;
}

void ServiceRegistry::update()
{
	this->checkTimeout();

	if (this->m_status == service_discovery::RS_Learning)
	{
		auto iDuration = apie::CtxSingleton::get().getConfigs()->service_learning_duration;

		auto iCurTime = apie::CtxSingleton::get().getCurSeconds();
		if (m_iStatusCheckTime == 0)
		{
			m_iStatusCheckTime = iCurTime;
		}

		if (iCurTime > m_iStatusCheckTime + iDuration)
		{
			this->m_status = service_discovery::RS_Forwarding;
			this->broadcast();
		}
	}
}

bool ServiceRegistry::updateInstance(uint64_t iSerialNum, const ::service_discovery::EndPointInstance& instance)
{
	auto curTime = apie::CtxSingleton::get().getCurSeconds();

	EndPoint point;
	point.realm = instance.realm();
	point.type = instance.type();
	point.id = instance.id();

	auto findPoint = m_pointMap.find(point);
	if (findPoint != m_pointMap.end())
	{
		return false;
	}

	//add
	m_pointMap[point] = iSerialNum;
	auto findIte = m_registered.find(iSerialNum);
	if (findIte == m_registered.end())
	{
		RegisteredEndPoint endPoint;
		endPoint.addTime = curTime;
		endPoint.modifyTime = curTime;
		endPoint.instance = instance;

		m_registered[iSerialNum] = endPoint;
	}
	else
	{
		findIte->second.modifyTime = curTime;
		findIte->second.instance = instance;
	}

	return true;
}


bool ServiceRegistry::updateNatsInstance(const ::service_discovery::EndPointInstance& instance) {

	auto curTime = apie::CtxSingleton::get().getCurSeconds();

	EndPoint point;
	point.realm = instance.realm();
	point.type = instance.type();
	point.id = instance.id();

	auto findPoint = m_pointMap.find(point);
	if (findPoint != m_pointMap.end())
	{
		return false;
	}

	//add
	m_pointMap[point] = 0;
	auto findIte = m_natsRegistered.find(point);
	if (findIte == m_natsRegistered.end())
	{
		RegisteredEndPoint endPoint;
		endPoint.addTime = curTime;
		endPoint.modifyTime = curTime;
		endPoint.instance = instance;

		m_natsRegistered[point] = endPoint;
	}
	else
	{
		findIte->second.modifyTime = curTime;
		findIte->second.instance = instance;
	}

	return true;
}

bool ServiceRegistry::updateHeartbeat(uint64_t iSerialNum)
{
	auto curTime = apie::CtxSingleton::get().getCurSeconds();

	auto findIte = m_registered.find(iSerialNum);
	if (findIte != m_registered.end())
	{
		findIte->second.modifyTime = curTime;

		return true;
	}

	return false;
}

bool ServiceRegistry::deleteBySerialNum(uint64_t iSerialNum)
{
	bool bResult = false;
	auto findIte = m_registered.find(iSerialNum);
	if (findIte != m_registered.end())
	{
		EndPoint point;
		point.realm = findIte->second.instance.realm();
		point.type = findIte->second.instance.type();
		point.id = findIte->second.instance.id();

		m_pointMap.erase(point);
		m_registered.erase(findIte);

		bResult = true;
	}

	return bResult;
}

void ServiceRegistry::checkTimeout()
{
	auto curTime = apie::CtxSingleton::get().getCurSeconds();

	std::vector<uint64_t> delSerial;
	for (const auto& items : m_registered)
	{
		if (curTime > items.second.modifyTime + m_serviceTimeout)
		{
			delSerial.push_back(items.first);
		}
	}

	bool bChanged = false;
	for (const auto& items : delSerial)
	{
		ServerConnection::sendCloseLocalServer(items);
		bool bTemp = GetModule<apie::ServiceRegistry>()->deleteBySerialNum(items);
		if (bTemp)
		{
			bChanged = true;
		}
	}

	if (bChanged)
	{
		GetModule<apie::ServiceRegistry>()->broadcast();
	}
}

void ServiceRegistry::broadcast()
{
	m_version++;

	::service_discovery::MSG_NOTICE_INSTANCE notice;
	notice.set_id(m_id);
	notice.set_version(m_version);
	notice.set_status(m_status);
	notice.set_mode(service_discovery::UM_Full);
	for (const auto& items : m_registered)
	{
		auto ptrAdd = notice.add_add_instance();
		*ptrAdd = items.second.instance;
	}

	for (const auto& items : m_registered)
	{
		MessageInfo info;
		info.iSessionId = items.first;
		info.iOpcode = ::opcodes::OPCODE_ID::OP_MSG_NOTICE_INSTANCE;
		apie::network::OutputStream::sendMsg(info, notice);
	}

	for (const auto& items : m_natsRegistered) {
		::rpc_msg::CHANNEL server;
		server.set_realm(items.second.instance.realm());
		server.set_type(items.second.instance.type());
		server.set_id(items.second.instance.id());
		server.set_actor_id(items.second.instance.actor_id());
		apie::rpc::RPC_CallNotify(server, ::opcodes::OPCODE_ID::OP_MSG_NOTICE_INSTANCE, notice);

		ASYNC_PIE_LOG(PIE_NOTICE, "Nats|notice|{}", items.second.instance.id());
	}

}


}

