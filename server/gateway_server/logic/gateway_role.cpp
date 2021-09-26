#include "logic/gateway_role.h"
#include "logic/gateway_mgr.h"

namespace apie {


RoleTablesData::RoleTablesData(uint64_t roleId) :
	role_id(roleId),
	user(roleId),
	role_extra(roleId)
{
	server.set_realm(apie::Ctx::getThisChannel().realm());
	server.set_type(::common::EPT_DB_ROLE_Proxy);
	server.set_id(1);
}


void RoleTablesData::LoadFromDb(CallbackType cb)
{
	std::weak_ptr<RoleTablesData> weak_this = shared_from_this();

	auto multiCb = [weak_this, cb](const status::Status& status, auto& tupleData, auto& tupleRows) {
		if (!status.ok())
		{
			cb(status);
			return;
		}

		auto share_this = weak_this.lock();
		if (!share_this) 
		{
			status::Status newStatus;
			newStatus.setErrorCode(status::StatusCode::Obj_NotExist);
			cb(status);
			return;
		}

		std::tie(share_this->user, share_this->role_extra) = tupleData;

		auto doneCb = [weak_this, cb](const status::Status& status, const std::tuple<uint32_t, uint32_t>& insertRows) mutable {
			if (!status.ok())
			{
				cb(status);
				return;
			}
			
			auto share_this = weak_this.lock();
			if (!share_this)
			{
				status::Status newStatus;
				newStatus.setErrorCode(status::StatusCode::Obj_NotExist);
				cb(status);
				return;
			}

			bool bResult = share_this->onLoaded();
			if (bResult)
			{
				cb(status);
			} 
			else
			{
				status::Status newStatus;
				newStatus.setErrorCode(status::StatusCode::DB_LoadedError);
				cb(status);
			}
		};
		Insert_OnNotExists(share_this->server, tupleData, tupleRows, doneCb);
	};
	apie::Multi_LoadFromDb(multiCb, server, user, role_extra);
}

bool RoleTablesData::SaveToDb(bool bFlush)
{
	auto bResult = onBeforeSave();
	if (!bResult)
	{
		return bResult;
	}

	auto&& tupleData = std::make_tuple(user, role_extra);
	if (bFlush)
	{
		Update_OnForced(server, tupleData);
	} 
	else
	{
		Update_OnChanged(server, tupleData);
	}

	return true;
}

bool RoleTablesData::onLoaded()
{
	return true;
}

bool RoleTablesData::onBeforeSave()
{
	return true;
}

std::shared_ptr<GatewayRole> GatewayRole::createGatewayRole(uint64_t iRoleId, uint64_t iSerialNum)
{
	return std::make_shared<GatewayRole>(iRoleId, iSerialNum);
}


GatewayRole::GatewayRole(uint64_t iRoleId, uint64_t iSerialNum) :
	m_iRoleId(iRoleId),
	m_iSerialNum(iSerialNum)
{

}

GatewayRole::~GatewayRole()
{
	this->destroy();
}

void GatewayRole::destroy()
{

}

uint64_t GatewayRole::getSerailNum()
{
	return m_iSerialNum;
}

uint64_t GatewayRole::getRoleId()
{
	return m_iRoleId;
}

void GatewayRole::setMaskFlag(uint32_t iFlag)
{
	m_iMaskFlag = iFlag;
}

uint32_t GatewayRole::getMaskFlag()
{
	return m_iMaskFlag;
}

bool GatewayRole::addRequestPerUnit(uint64_t iValue)
{
	auto iCurTime = apie::CtxSingleton::get().getCurSeconds();
	uint32_t iLimit = apie::CtxSingleton::get().getConfigs()->limited.requests_per_unit;

	if (iCurTime > m_iRequestUnitExpiresAt)
	{
		this->resetRequestPerUnit();
	}

	if (m_iRequestUnitExpiresAt == 0)
	{
		uint32_t iDuration = apie::CtxSingleton::get().getConfigs()->limited.uint;
		m_iRequestUnitExpiresAt = iCurTime + iDuration;
	}

	m_iRequests += iValue;
	m_iRequestPerUnit += iValue;

	if (m_iRequestPerUnit > iLimit)
	{
		if (iLimit == 0)
		{
			return true;
		}

		std::stringstream ss;
		ss << "recv package out limited|userId:" << m_iRoleId << "|m_iRequestPerUnit:" << m_iRequestPerUnit;
		ASYNC_PIE_LOG("addRequestPerUnit", PIE_CYCLE_DAY, PIE_ERROR, ss.str().c_str());

		this->close();
		return false;
	}

	return true;
}

void GatewayRole::resetRequestPerUnit()
{
	m_iRequestPerUnit = 0;
	m_iRequestUnitExpiresAt = 0;
}

void GatewayRole::close()
{
	GatewayMgrSingleton::get().removeGateWayRole(m_iRoleId);
	ServerConnection::sendCloseLocalServer(m_iSerialNum);
}

}

