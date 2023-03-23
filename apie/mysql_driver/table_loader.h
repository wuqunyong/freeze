// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
#pragma once

#include <set>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <memory>
#include <functional> 

#include "apie/mysql_driver/db_load_component.h"
#include "apie/proto/init.h"

namespace apie {

	using DbFuntor = std::function<void(std::shared_ptr<apie::DbLoadComponent>)>;


	template <typename T>
	struct SingleRowLoader {
		using TableType = T;
		using LoaderType = SingleRowLoader<T>;

		SingleRowLoader(uint64_t id ) :
			m_tableType(id)
		{

		}

		template <typename... Args>
		SingleRowLoader(Args &&... args) :
			m_tableType(std::forward<Args>(args)...)
		{

		}

		void loadFromDb(std::shared_ptr<apie::DbLoadComponent> loader, ::rpc_msg::CHANNEL server)
		{
			if (!isInit())
			{
				initServer(server);
			}

			loader->setState<LoaderType>(DbLoadComponent::ELS_Loading);
			auto ptrCb = [this, loader](apie::status::Status status, TableType& data, uint32_t iRows) {
				if (!status.ok())
				{
					loader->setState<LoaderType>(DbLoadComponent::ELS_Failure);
					return;
				}

				if (iRows != 0)
				{
					this->m_optData = data;
				}

				loader->setState<LoaderType>(DbLoadComponent::ELS_Success);
			};
			apie::LoadFromDb<TableType>(server, m_tableType, ptrCb);
		}

		bool isInit()
		{
			return m_initServer;
		}

		void initServer(::rpc_msg::CHANNEL server)
		{
			m_initServer = true;
			m_server = server;
		}

		::rpc_msg::CHANNEL getServer()
		{
			return m_server;
		}

		std::optional<TableType>& getData()
		{
			return m_optData;
		}

	private:
		TableType m_tableType;
		std::optional<TableType> m_optData;

		bool m_initServer = false;
		::rpc_msg::CHANNEL m_server;

	};


	template <typename T>
	struct MultiRowLoader {
		using TableType = T;
		using LoaderType = MultiRowLoader<T>;

		MultiRowLoader(uint64_t id) :
			m_tableType(id)
		{

		}

		template <typename... Args>
		MultiRowLoader(Args &&... args) :
			m_tableType(std::forward<Args>(args)...)
		{

		}

		void markFilter(const std::vector<uint8_t>& index)
		{
			m_tableType.markFilter(index);
		}

		void loadFromDb(std::shared_ptr<apie::DbLoadComponent> loader, ::rpc_msg::CHANNEL server)
		{
			if (!isInit())
			{
				initServer(server);
			}

			loader->setState<LoaderType>(DbLoadComponent::ELS_Loading);
			auto ptrCb = [this, loader](status::Status status, std::vector<TableType>& data) {
				if (!status.ok())
				{
					loader->setState<LoaderType>(DbLoadComponent::ELS_Failure);
					return;
				}

				m_vecData = data;
				loader->setState<LoaderType>(DbLoadComponent::ELS_Success);
			};
			apie::LoadFromDbByFilter<TableType>(server, m_tableType, ptrCb);
		}

		bool isInit()
		{
			return m_initServer;
		}

		void initServer(::rpc_msg::CHANNEL server)
		{
			m_initServer = true;
			m_server = server;
		}

		::rpc_msg::CHANNEL getServer()
		{
			return m_server;
		}

		std::vector<TableType>& getData()
		{
			return m_vecData;
		}


	private:
		TableType m_tableType;
		std::vector<TableType> m_vecData;

		bool m_initServer = false;
		::rpc_msg::CHANNEL m_server;

	};


	template <typename T>
	struct AllRowLoader {
		using TableType = T;
		using LoaderType = AllRowLoader<T>;

		AllRowLoader(uint64_t id) :
			m_tableType(id)
		{

		}

		template <typename... Args>
		AllRowLoader(Args &&... args) :
			m_tableType(std::forward<Args>(args)...)
		{

		}

		void loadFromDb(std::shared_ptr<apie::DbLoadComponent> loader, ::rpc_msg::CHANNEL server)
		{
			if (!isInit())
			{
				initServer(server);
			}

			loader->setState<LoaderType>(DbLoadComponent::ELS_Loading);
			auto ptrCb = [this, loader](status::Status status, std::vector<TableType>& data) {
				if (!status.ok())
				{
					loader->setState<LoaderType>(DbLoadComponent::ELS_Failure);
					return;
				}

				m_vecData = data;
				loader->setState<LoaderType>(DbLoadComponent::ELS_Success);
			};
			apie::LoadFromDbByQueryAll<TableType>(server, m_tableType, ptrCb);
		}

		bool isInit()
		{
			return m_initServer;
		}

		void initServer(::rpc_msg::CHANNEL server)
		{
			m_initServer = true;
			m_server = server;
		}

		::rpc_msg::CHANNEL getServer()
		{
			return m_server;
		}

		std::vector<TableType>& getData()
		{
			return m_vecData;
		}


	private:
		TableType m_tableType;
		std::vector<TableType> m_vecData;

		bool m_initServer = false;
		::rpc_msg::CHANNEL m_server;

	};
} 

