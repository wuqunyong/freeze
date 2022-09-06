#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <tuple>

#include "apie.h"
#include "db_load_component.h"

#include "../../common/dao/model_user.h"
#include "../../common/dao/model_account.h"
#include "../../common/dao/model_role_extra.h"

namespace apie {


	using DbFuntor = std::function<void(std::shared_ptr<apie::DbLoadComponent>)>;


	template <typename T>
	struct SingleRowLoader {
		using TableType = T;
		using LoaderType = SingleRowLoader<T>;

		SingleRowLoader(uint64_t id = 0) :
			m_tableType(id)
		{

		}

		void loadFromDb(std::shared_ptr<apie::DbLoadComponent> loader, ::rpc_msg::CHANNEL server)
		{
			loader->setState<LoaderType>(DbLoadComponent::ELS_Loading);
			auto ptrCb = [this, loader](apie::status::Status status, TableType& data, uint32_t iRows) {
				if (!status.ok())
				{
					loader->setState<LoaderType>(DbLoadComponent::ELS_Failure);
					return;
				}

				if (iRows != 0)
				{
					this->m_data = data;
				}

				loader->setState<LoaderType>(DbLoadComponent::ELS_Success);
			};
			apie::LoadFromDb<TableType>(server, m_tableType, ptrCb);
		}

		TableType m_tableType;
		std::optional<TableType> m_data;
	};


	template <typename T>
	struct MultiRowLoader {
		using TableType = T;
		using LoaderType = MultiRowLoader<T>;

		MultiRowLoader(uint64_t id = 0) :
			m_tableType(id)
		{

		}

		void markFilter(const std::vector<uint8_t>& index)
		{
			m_tableType.markFilter(index);
		}

		void loadFromDb(std::shared_ptr<apie::DbLoadComponent> loader, ::rpc_msg::CHANNEL server)
		{
			loader->setState<LoaderType>(DbLoadComponent::ELS_Loading);
			auto ptrCb = [this, loader](status::Status status, std::vector<TableType>& data) {
				if (!status.ok())
				{
					loader->setState<LoaderType>(DbLoadComponent::ELS_Failure);
					return;
				}

				m_data = data;
				loader->setState<LoaderType>(DbLoadComponent::ELS_Success);
			};
			apie::LoadFromDbByFilter<TableType>(server, m_tableType, ptrCb);
		}

		TableType m_tableType;
		std::vector<TableType> m_data;
	};


	struct Single_ModelUser_Loader
	{
		using Type = SingleRowLoader<apie::ModelUser>;
	};

	struct Single_ModelRoleExtra_Loader
	{
		using Type = SingleRowLoader<apie::ModelRoleExtra>;
	};

	struct Multi_ModelUser_Loader
	{
		using Type = MultiRowLoader<apie::ModelUser>;
	};


	class TestModule10
	{
	public:
		TestModule10(std::string name, module_loader::ModuleLoaderBase* prtLoader);

		static std::string moduleName();
		static uint32_t modulePrecedence();

		apie::status::Status init();
		apie::status::Status start();
		apie::status::Status ready();
		apie::status::Status exit();

		void setHookReady(hook::HookPoint point);

	private:
		std::string m_name;
		module_loader::ModuleLoaderBase* m_prtLoader;
	};
}
