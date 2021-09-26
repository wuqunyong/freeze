#include "logic/role_tables_data.h"

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

RoleTablesData::~RoleTablesData()
{

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

		//CHANGED
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

}

