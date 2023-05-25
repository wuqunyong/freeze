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

#include "../../../common/dao/init.h"

namespace apie {


	struct Single_ModelUser_Loader
	{
		using Type = SingleRowLoader<apie::dbt_role::role_base_AutoGen>;
	};

	struct Single_ModelRoleExtra_Loader
	{
		using Type = SingleRowLoader<apie::dbt_role::role_extra_AutoGen>;
	};

	struct Single_ModelVarchars1_Loader
	{
		using Type = SingleRowLoader<apie::dbt_role::varchars1_AutoGen>;
	};


	struct Multi_ModelUser_Loader
	{
		using Type = MultiRowLoader<apie::dbt_role::role_base_AutoGen>;
	};

	struct Single_ModelAccount_Loader
	{
		using Type = SingleRowLoader<apie::dbt_account::account_AutoGen>;
	};

	struct All_ModelAccountName_Loader
	{
		using Type = AllRowLoader<apie::dbt_account::account_name_AutoGen>;
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



	class ModuleA
	{
	public:
		ModuleA(uint64_t iId = 0) :
			m_iId(iId)
		{

		}

		std::string toString()
		{
			return "ModuleA";
		}

		void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
		{
			std::cout << "ModuleA loadFromDbLoad" << std::endl;

			if (ptrLoader->has<Single_ModelUser_Loader>())
			{
				m_data1 = ptrLoader->get<Single_ModelUser_Loader>();
			}

			if (ptrLoader->has<Single_ModelRoleExtra_Loader>())
			{
				m_data2 = ptrLoader->get<Single_ModelRoleExtra_Loader>();
			}

			if (ptrLoader->has<Single_ModelVarchars1_Loader>())
			{
				m_data10 = ptrLoader->get<Single_ModelVarchars1_Loader>();
			}

			if (ptrLoader->has<Single_ModelAccount_Loader>())
			{
				m_data3 = ptrLoader->get<Single_ModelAccount_Loader>();

				m_ptrAccount = Single_ModelAccount_Loader::Type::TableType::Create(m_iId);
			}
		}

		void loadFromDbDone()
		{
			if (!m_data3.getData().has_value())
			{
				auto dbObj = Single_ModelAccount_Loader::Type::TableType(m_iId);
				InsertToDb<Single_ModelAccount_Loader::Type::TableType>(m_data3.getServer(), dbObj, nullptr);
			}

			std::cout << "ModuleA loadFromDbDone" << std::endl;
		}

		void saveToDb()
		{
			if (m_data3.getData().has_value())
			{
				m_data3.getData().value().set_modified_time(time(nullptr));
				//UpdateToDb<Single_ModelAccount_Loader::Type::TableType>(m_data3.getServer(), m_data3.getData().value(), nullptr);
				UpdateToDb(m_data3.getServer(), m_data3.getData().value(), nullptr);
			}

			std::cout << "ModuleA saveToDb" << std::endl;
		}

	private:
		uint64_t m_iId = 0;


		Single_ModelUser_Loader::Type m_data1;
		Single_ModelRoleExtra_Loader::Type m_data2;
		Single_ModelVarchars1_Loader::Type m_data10;

		Single_ModelAccount_Loader::Type m_data3;
		std::shared_ptr<apie::dbt_account::account_AutoGen> m_ptrAccount = nullptr;

	};

	struct TestModuleA
	{
		using Type = ModuleA;
	};


	class ModuleB
	{
	public:
		ModuleB(uint64_t iId = 0) :
			m_iId(iId)
		{

		}

		std::string toString()
		{
			std::stringstream ss;
			ss << "ModuleB" << ":" << m_iId << ":" << m_value;
			return ss.str();
		}

		void incrementValue()
		{
			m_value++;
		}

		void loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<apie::DbLoadComponent> ptrLoader)
		{
			std::cout << "ModuleB loadFromDbLoad" << std::endl;

			if (ptrLoader->has<Multi_ModelUser_Loader>())
			{
				m_data1 = ptrLoader->get<Multi_ModelUser_Loader>();
			}

			if (ptrLoader->has<All_ModelAccountName_Loader>())
			{
				m_data2 = ptrLoader->get<All_ModelAccountName_Loader>();
			}
		}

		void loadFromDbDone()
		{
			std::cout << "ModuleB loadFromDbDone" << std::endl;
		}

		void saveToDb()
		{
			std::cout << "ModuleB saveToDb" << std::endl;

			for (auto& elems : m_data2.getData())
			{
				auto name = elems.get_name() + "_test";
				elems.set_name(name);
				UpdateToDb(m_data2.getServer(), elems, nullptr);
			}
		}

	private:
		uint64_t m_iId = 0;
		uint64_t m_value = 0;

		Multi_ModelUser_Loader::Type m_data1;
		All_ModelAccountName_Loader::Type m_data2;
	};

	struct TestModuleB
	{
		using Type = ModuleB;
	};

}
