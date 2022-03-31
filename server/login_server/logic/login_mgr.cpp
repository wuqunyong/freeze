#include "logic/login_mgr.h"

#include "../../common/dao/model_account.h"
#include "../../common/dao/model_account_name.h"
#include "../../common/opcodes.h"

#include "module_handler/login_mgr_module.h"


namespace apie {

std::string LoginMgr::moduleName()
{
	return "LoginMgr";
}

uint32_t LoginMgr::modulePrecedence()
{
	return 1;
}

LoginMgr::LoginMgr(std::string name, module_loader::ModuleLoaderBase* prtLoader)
	: m_name(name),
	m_prtLoader(prtLoader)
{

}

apie::status::Status LoginMgr::init()
{
	auto bResult = apie::CtxSingleton::get().checkIsValidServerType({ ::common::EPT_Login_Server });
	if (!bResult)
	{
		return { apie::status::StatusCode::HOOK_ERROR, "invalid Type" };
	}

	LoginMgrModule::init();

	return {apie::status::StatusCode::OK, ""};
}

apie::status::Status LoginMgr::start()
{
	auto dbType = DeclarativeBase::DBType::DBT_Account;
	auto ptrReadyCb = [this](bool bResul, std::string sInfo, uint64_t iCallCount) {
		if (!bResul)
		{
			std::stringstream ss;
			ss << "CallMysqlDescTable|bResul:" << bResul << ",sInfo:" << sInfo << ",iCallCount:" << iCallCount;

			PANIC_ABORT(ss.str().c_str());
		}

		this->setHookReady(apie::hook::HookPoint::HP_Start);
	};

	::rpc_msg::CHANNEL server;
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ACCOUNT_Proxy);
	server.set_id(1);

	std::map<std::string, DAOFactory::TCreateMethod> loadTables;
	loadTables.insert(std::make_pair(ModelAccount::getFactoryName(), ModelAccount::createMethod));
	loadTables.insert(std::make_pair(ModelAccountName::getFactoryName(), ModelAccountName::createMethod));

	bool bResult = RegisterRequiredTable(server, dbType, loadTables, ptrReadyCb);
	if (bResult)
	{
		return { apie::status::StatusCode::OK_ASYNC, "" };
	}
	else
	{
		return { apie::status::StatusCode::HOOK_ERROR, "HR_Error" };
	}
}

apie::status::Status LoginMgr::ready()
{
	LoginMgrModule::ready();

	std::stringstream ss;
	ss << "Server Ready!";
	std::cout << ss.str() << std::endl;
	ASYNC_PIE_LOG("ServerStatus", PIE_CYCLE_DAY, PIE_NOTICE, ss.str().c_str());

	return { apie::status::StatusCode::OK, "" };
}

apie::status::Status LoginMgr::exit()
{
	return { apie::status::StatusCode::OK, "" };
}

void LoginMgr::setHookReady(hook::HookPoint point)
{
	m_prtLoader->setHookReady(point);
}

}

