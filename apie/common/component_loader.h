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
#include "apie/event/timer_impl.h"

namespace apie {

template <typename KeyType, typename TupleT>
class ComponentLoader : public std::enable_shared_from_this<ComponentLoader<KeyType, TupleT>>
{
public:
	template <typename U>
	using ValueTypeT = typename U::Type;

	using WrapperType = TupleT;

	using type = ComponentLoader; // using injected-class-name
	using ReadyCb = std::function<void(apie::status::Status, std::shared_ptr<type> loader)>;

	enum E_LoadingState
	{
		ELS_None = 0,
		ELS_Loading = 1,
		ELS_Failure = 2,
		ELS_Success = 3,
	};


	template<class Key, class Tuple, std::size_t... Is>
	//friend static inline auto CreateComponentLoaderPtr(Key iId, const Tuple& t, std::index_sequence<Is...>);
	friend inline auto CreateComponentLoaderPtr(Key iId, const Tuple& t, std::index_sequence<Is...>);

	~ComponentLoader()
	{

	}

	KeyType getKey()
	{
		return m_id;
	}

	template <typename U>
	void Append(U moduleType)
	{
		m_options.set<U>(m_id);
		//m_modules.push_back(typeid(moduleType));
	}

	template <typename U>
	bool has() const
	{
		return m_options.has<U>();
	}

	template <typename U>
	ValueTypeT<U>& lookup()
	{
		if (!has<U>())
		{
			std::stringstream ss;
			ss << "CommonModuleLoader lookup " << typeid(U).name() << " : unregister";
			throw std::logic_error(ss.str());
		}

		return m_options.lookup<U>();
	}


	ComponentLoader& setState(std::type_index tIndex, E_LoadingState v) {
		m_loading[tIndex] = v;

		switch (v)
		{
		case ELS_Failure:
		{
			if (m_ready)
			{
				break;
			}
			m_ready = true;

			apie::event_ns::EphemeralTimerMgrSingleton::get().deleteEphemeralTimer(m_timerId);

			auto self = this->shared_from_this();
			apie::status::Status status(apie::status::StatusCode::LoadFromDbError, tIndex.name());
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

				apie::event_ns::EphemeralTimerMgrSingleton::get().deleteEphemeralTimer(m_timerId);

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

	void Meta_initCreate(ReadyCb cb, uint64_t interval = 60000)
	{
		this->m_cb = cb;

		m_loading.clear();
		m_ready = false;
		m_timerId = 0;

		initCreateImpl_Loading(m_wrapperType);
		initCreateImpl(m_wrapperType);

		if (m_ready)
		{
			return;
		}


		auto self = this->shared_from_this();
		auto ephemeralTimerCb = [self]() mutable
		{
			self->m_ready = true;

			apie::status::Status status(apie::status::StatusCode::TIMEOUT, "");
			self->m_cb(status, self);
		};
		auto ptrTimer = apie::event_ns::EphemeralTimerMgrSingleton::get().createEphemeralTimer(ephemeralTimerCb);
		ptrTimer->enableTimer(interval);

		m_timerId = ptrTimer->getId();
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
			this->setState(typeid(decltype(tObj)), ELS_Loading);

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
			auto functorObj = [self, tObj](bool bResult) {
				if (bResult)
				{
					self->setState(typeid(decltype(tObj)), ELS_Success);
				} 
				else
				{
					self->setState(typeid(decltype(tObj)), ELS_Failure);
				}
			};

			this->lookup<decltype(tObj)>().initCreate(functorObj);

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
	//std::vector<std::type_index> m_modules;
	apie::common::Options m_options;

	WrapperType m_wrapperType;

	std::unordered_map<std::type_index, E_LoadingState> m_loading;
	ReadyCb m_cb;
	bool m_ready = false;
	uint64_t m_timerId = 0;
};

template<class Key, class Tuple, std::size_t... Is>
static inline auto CreateComponentLoaderPtr(Key iId, const Tuple& t, std::index_sequence<Is...>)
{
	auto pInstance = std::shared_ptr<ComponentLoader<Key, Tuple>>(new ComponentLoader<Key, Tuple>(t, iId, std::get<Is>(t)...));
	return pInstance;
}


template<class Key, class Tuple>
auto MakeComponentLoader(Key iId, const Tuple& t)
{
	using Indices = std::make_index_sequence<
		std::tuple_size<typename std::decay<Tuple>::type>::value>;

	return CreateComponentLoaderPtr(iId, t, Indices());
}


}