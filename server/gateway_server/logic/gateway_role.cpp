#include "logic/gateway_role.h"
#include "logic/gateway_mgr.h"

namespace apie {

std::shared_ptr<GatewayRole> GatewayRole::createGatewayRole(uint64_t iRoleId, uint64_t iSerialNum)
{
	return std::make_shared<GatewayRole>(iRoleId, iSerialNum);
}


GatewayRole::GatewayRole(uint64_t iRoleId, uint64_t iSerialNum) :
	m_iRoleId(iRoleId),
	m_iSerialNum(iSerialNum)
{
	m_bSubNats = apie::event_ns::NatsManager::SubscribeChannelByRIdFromGW(iRoleId);
}

GatewayRole::~GatewayRole()
{
	this->destroy();
}

void GatewayRole::destroy()
{
	if (m_bSubNats)
	{
		apie::event_ns::NatsManager::UnsubscribeChannelByRIdFromGW(m_iRoleId);
		m_bSubNats = false;
	}
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
	APieGetModule<GatewayMgr>()->removeGateWayRole(m_iRoleId);

	ServerConnection::sendCloseLocalServer(m_iSerialNum);
}

}

