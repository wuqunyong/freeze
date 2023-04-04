#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <tuple>
#include <optional>
#include <typeinfo>
#include <typeindex>

#include "apie/common/options.h"
#include "apie/mysql_driver/db_load_component.h"
#include "apie/proto/init.h"

namespace apie {

template <typename KeyType, typename T>
class CommonModuleLoader : public std::enable_shared_from_this<CommonModuleLoader<KeyType, T>>
{
public:
	template <typename T>
	using ValueTypeT = typename T::Type;

	using WrapperType = T;

	template<class Key, class Tuple, std::size_t... Is>
	friend static inline auto CreateCommonModuleLoaderPtr(Key iId, const Tuple& t, std::index_sequence<Is...>);

	~CommonModuleLoader()
	{

	}

	template <typename T>
	void Append(T moduleType)
	{
		m_options.set<T>(m_id);
		m_modules.push_back(typeid(moduleType));
	}

	template <typename T>
	bool has() const
	{
		return m_options.has<T>();
	}

	template <typename T>
	ValueTypeT<T>& lookup()
	{
		if (!has<T>())
		{
			std::stringstream ss;
			ss << "CommonModuleLoader lookup " << typeid(T).name() << " : unregister";
			throw std::exception(ss.str().c_str());
		}

		return m_options.lookup<T>();
	}

	void Meta_loadFromDbLoader(const ::rpc_msg::CHANNEL& server, std::shared_ptr<DbLoadComponent> ptrLoad)
	{
		loadFromDbLoadImpl(server, ptrLoad, m_wrapperType);
	}

	void Meta_loadFromDbDone()
	{
		loadFromDbDoneImpl(m_wrapperType);
	}

	void Meta_saveToDb()
	{
		SaveToDbImpl(m_wrapperType);
	}

private:

	template <typename... Arg>
	CommonModuleLoader(WrapperType wrapperType, KeyType iId, Arg&&... a) :
		m_wrapperType(wrapperType), m_id(iId)
	{
		AppendAll(std::forward<Arg&&>(a)...);
	}

	template <size_t I = 0, typename... Ts>
	constexpr void loadFromDbLoadImpl(const ::rpc_msg::CHANNEL& server, std::shared_ptr<DbLoadComponent> ptrLoad, std::tuple<Ts...> tup)
	{
		// If we have iterated through all elements
		if constexpr (I == sizeof...(Ts))
		{
			// Last case, if nothing is left to
			// iterate, then exit the function
			return;
		}
		else
		{
			auto tObj = std::get<I>(tup);
			this->lookup<decltype(tObj)>().loadFromDbLoader(server, ptrLoad);

			// Going for next element.
			this->loadFromDbLoadImpl<I + 1>(server, ptrLoad, tup);
		}
	}

	template <size_t I = 0, typename... Ts>
	constexpr void loadFromDbDoneImpl(std::tuple<Ts...> tup)
	{
		// If we have iterated through all elements
		if constexpr (I == sizeof...(Ts))
		{
			// Last case, if nothing is left to
			// iterate, then exit the function
			return;
		}
		else
		{
			auto tObj = std::get<I>(tup);
			this->lookup<decltype(tObj)>().loadFromDbDone();

			// Going for next element.
			this->loadFromDbDoneImpl<I + 1>(tup);
		}
	}

	template <size_t I = 0, typename... Ts>
	constexpr void SaveToDbImpl(std::tuple<Ts...> tup)
	{
		// If we have iterated through all elements
		if constexpr (I == sizeof...(Ts))
		{
			// Last case, if nothing is left to
			// iterate, then exit the function
			return;
		}
		else
		{
			auto tObj = std::get<I>(tup);
			this->lookup<decltype(tObj)>().saveToDb();

			// Going for next element.
			this->SaveToDbImpl<I + 1>(tup);
		}
	}


	CommonModuleLoader(const CommonModuleLoader&) = delete;
	CommonModuleLoader& operator=(const CommonModuleLoader&) = delete;


	template <typename H, typename... Tail>
	void AppendAll(H&& head, Tail&&... a)
	{
		Append(std::forward<H>(head));
		AppendAll(std::forward<Tail>(a)...);
	}

	/// Terminate the recursion.
	void AppendAll()
	{
	}

	KeyType m_id;
	std::vector<std::type_index> m_modules;
	apie::common::Options m_options;

	WrapperType m_wrapperType;
};

template<class Key, class Tuple, std::size_t... Is>
static inline auto CreateCommonModuleLoaderPtr(Key iId, const Tuple& t, std::index_sequence<Is...>)
{
	auto pInstance = std::shared_ptr<CommonModuleLoader<Key, Tuple>>(new CommonModuleLoader<Key, Tuple>(t, iId, std::get<Is>(t)...));
	return pInstance;
}


template<class Key, class Tuple, typename Indices = std::make_index_sequence<std::tuple_size<Tuple>::value>>
auto MakeCommonModuleLoader(Key iId, const Tuple& t)
{
	return CreateCommonModuleLoaderPtr(iId, t, Indices{});
}


}