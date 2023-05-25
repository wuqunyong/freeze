#include "logic/role/component_role_base.h"

namespace apie {


Component_RoleBase::Component_RoleBase(uint64_t roleId)
	: m_roleId(roleId)
{

}

void Component_RoleBase::loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
{
	m_server = server;

	if (ptrLoader->has<Single_RoleBase_Loader>())
	{
		auto& optData = ptrLoader->lookup<Single_RoleBase_Loader>().getData();
		m_dbData = optData;

		if (m_dbData.has_value())
		{
			m_dbData.value().SetDbProxyServer(m_server);
		}
	}
}

void Component_RoleBase::loadFromDbDone()
{
}

void Component_RoleBase::saveToDb()
{
	std::time_t iCurTime = std::time(nullptr);

	if (m_dbData.has_value())
	{
		m_dbData.value().set_offline_time(iCurTime);
		m_dbData.value().Update();
	}
}

void Component_RoleBase::initCreate(DoneFunctor functorObj)
{
	if (m_dbData.has_value())
	{
		auto cb = [functorObj](apie::status::Status status, bool result, uint64_t affectedRows) {
			if (!status.ok())
			{
				functorObj(false);
			}
			else
			{
				functorObj(true);
			}
		};

		functorObj(true);

		std::time_t iCurTime = std::time(nullptr);
		m_dbData.value().set_login_time(iCurTime);
		m_dbData.value().Update(nullptr);
	}
	else
	{
		auto cb = [functorObj](apie::status::Status status, bool result, uint64_t affectedRows, uint64_t insertId) {
			if (!status.ok())
			{
				functorObj(false);
			}
			else
			{
				functorObj(true);
			}
		};
		apie::dbt_role::role_base_AutoGen dbObj(m_roleId);

		std::time_t iCurTime = std::time(nullptr);

		dbObj.set_game_id(0);
		dbObj.set_register_time(iCurTime);
		dbObj.set_login_time(iCurTime);
		dbObj.SetDbProxyServer(m_server);
		dbObj.Insert(cb);

	}
}

void Component_RoleBase::addLevel()
{
	if (!m_dbData.has_value())
	{
		return;
	}

	auto iCurLevel = m_dbData.value().get_level();
	m_dbData.value().set_level(iCurLevel + 1);
	m_dbData.value().Update();
}

}

