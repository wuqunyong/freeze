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
class ComponentLoader : public std::enable_shared_from_this<ComponentLoader<KeyType, T>>
{
public:
	template <typename T>
	using ValueTypeT = typename T::Type;

	using WrapperType = T;

	using type = ComponentLoader;
	using ReadyCb = std::function<void(apie::status::Status, std::shared_ptr<type> loader)>;

	enum E_LoadingState
	{
		ELS_None = 0,
		ELS_Loading = 1,
		ELS_Failure = 2,
		ELS_Success = 3,
	};


	template<class Key, class Tuple, std::size_t... Is>
	friend static inline auto CreateComponentLoaderPtr(Key iId, const Tuple& t, std::index_sequence<Is...>);

	~ComponentLoader()
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

	template <typename T>
	ComponentLoader& setState(E_LoadingState v) {
		m_loading[typeid(T)] = v;

		switch (v)
		{
		case ELS_Failure:
		{
			if (m_ready)
			{
				break;
			}
			m_ready = true;

			auto self = this->shared_from_this();
			apie::status::Status status(apie::status::StatusCode::LoadFromDbError, typeid(T).name());
			m_cb(status, self);
			break;
		}
		case ELS_Success:
		{
			if (m_ready)
			{
				break;
			}

			bool bDone = true;
			for (const auto& elems : m_loading)
			{
				if (elems.second == ELS_Loading)
				{
					bDone = false;
					break;
				}
			}

			if (bDone)
			{
				m_ready = true;
				apie::status::Status status(apie::status::StatusCode::OK, "");
				auto self = this->shared_from_this();
				m_cb(status, self);
			}
			break;
		}
		default:
			break;
		}
		return *this;
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

	void Meta_initCreate(ReadyCb cb)
	{
		this->m_cb = cb;

		initCreateImpl_Loading(m_wrapperType);
		initCreateImpl(m_wrapperType);
	}

private:

	template <typename... Arg>
	ComponentLoader(WrapperType wrapperType, KeyType iId, Arg&&... a) :
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

	template <size_t I = 0, typename... Ts>
	constexpr void initCreateImpl_Loading(std::tuple<Ts...> tup)
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
			this->setState<decltype(tObj)>(ELS_Loading);

			// Going for next element.
			this->initCreateImpl_Loading<I + 1>(tup);
		}
	}

	template <size_t I = 0, typename... Ts>
	constexpr void initCreateImpl(std::tuple<Ts...> tup)
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

			auto self = this->shared_from_this();
			auto cbObj = [self, tObj](bool bResult) {
				if (bResult)
				{
					self->setState<decltype(tObj)>(ELS_Success);
				} 
				else
				{
					self->setState<decltype(tObj)>(ELS_Failure);
				}
			};

			this->lookup<decltype(tObj)>().initCreate(cbObj);

			// Going for next element.
			this->initCreateImpl<I + 1>(tup);
		}
	}


	ComponentLoader(const ComponentLoader&) = delete;
	ComponentLoader& operator=(const ComponentLoader&) = delete;


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

	std::unordered_map<std::type_index, E_LoadingState> m_loading;
	ReadyCb m_cb;
	bool m_ready = false;
};

template<class Key, class Tuple, std::size_t... Is>
static inline auto CreateComponentLoaderPtr(Key iId, const Tuple& t, std::index_sequence<Is...>)
{
	auto pInstance = std::shared_ptr<ComponentLoader<Key, Tuple>>(new ComponentLoader<Key, Tuple>(t, iId, std::get<Is>(t)...));
	return pInstance;
}


template<class Key, class Tuple, typename Indices = std::make_index_sequence<std::tuple_size<Tuple>::value>>
auto MakeComponentLoader(Key iId, const Tuple& t)
{
	return CreateComponentLoaderPtr(iId, t, Indices{});
}


}