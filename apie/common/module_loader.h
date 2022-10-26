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

namespace apie {

template <typename T>
class ModuleLoader : public std::enable_shared_from_this<ModuleLoader<T>>
{
public:
	template <typename T>
	using ValueTypeT = typename T::Type;

	using WrapperType = T;

	template<class Tuple, std::size_t... Is>
	friend static inline auto CreateModuleLoaderInstance(uint64_t iId, const Tuple& t, std::index_sequence<Is...>);

	~ModuleLoader()
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
	ValueTypeT<T>& lookup(ValueTypeT<T> value = {})
	{
		if (!has<T>())
		{
			throw std::exception("unregister");
		}

		return m_options.lookup<T>(value);
	}

	void loadFromDbLoader(std::shared_ptr<DbLoadComponent> ptrLoad)
	{
		loadFromDbLoadImpl(ptrLoad, m_wrapperType);
	}

	void saveToDb()
	{
		SaveToDbImpl(m_wrapperType);
	}

private:

	template <typename... Arg>
	ModuleLoader(WrapperType wrapperType, uint64_t iId, Arg&&... a) :
		m_wrapperType(wrapperType), m_id(iId)
	{
		AppendAll(std::forward<Arg&&>(a)...);
	}

	template <size_t I = 0, typename... Ts>
	constexpr void loadFromDbLoadImpl(std::shared_ptr<DbLoadComponent> ptrLoad, std::tuple<Ts...> tup)
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
			this->lookup<decltype(tObj)>().loadFromDbLoader(ptrLoad);

			// Going for next element.
			this->loadFromDbLoadImpl<I + 1>(ptrLoad, tup);
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


	ModuleLoader(const ModuleLoader&) = delete;
	ModuleLoader& operator=(const ModuleLoader&) = delete;


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

	uint64_t m_id = 0;
	std::vector<std::type_index> m_modules;
	apie::common::Options m_options;

	WrapperType m_wrapperType;
};

template<class Tuple, std::size_t... Is>
static inline auto CreateModuleLoaderInstance(uint64_t iId, const Tuple& t, std::index_sequence<Is...>)
{
	auto pInstance = std::shared_ptr<ModuleLoader<Tuple>>(new ModuleLoader<Tuple>(t, iId, std::get<Is>(t)...));
	return pInstance;
}

}