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

namespace apie {

	class DbLoadComponent;

	//namespace internal {

	//	template <typename T>
	//	inline T const& DBDefaultValue() {
	//		static auto const* const kDefaultValue = new T{};
	//		return *kDefaultValue;
	//	}

	//}  // namespace internal

	class DbLoadComponent : public std::enable_shared_from_this<DbLoadComponent> {
	private:
		template <typename T>
		using ValueTypeT = typename T::Type;

		using ReadyCb = std::function<void(apie::status::Status, std::shared_ptr<apie::DbLoadComponent> loader)>;

		/// Constructs an empty instance.
		DbLoadComponent() = default;

	public:
		friend std::shared_ptr<DbLoadComponent> CreateDBLoaderPtr();

		enum E_LoadingState
		{
			ELS_None = 0,
			ELS_Loading = 1,
			ELS_Failure = 2,
			ELS_Success = 3,
		};

		~DbLoadComponent()
		{

		}

		DbLoadComponent(DbLoadComponent const& rhs) {
			for (auto const& kv : rhs.m_) m_.emplace(kv.first, kv.second->clone());
		}
		DbLoadComponent& operator=(DbLoadComponent const& rhs) {
			DbLoadComponent tmp(rhs);
			std::swap(m_, tmp.m_);
			return *this;
		}
		DbLoadComponent(DbLoadComponent&&) = default;
		DbLoadComponent& operator=(DbLoadComponent&&) = default;

		/**
		 * Sets option `T` to the value @p v and returns a reference to `*this`.
		 *
		 * @code
		 * struct FooOption {
		 *   using Type = int;
		 * };
		 * auto opts = DbLoadComponent{}.set<FooOption>(123);
		 * @endcode
		 *
		 * @tparam T the option type
		 * @param v the value to set the option T
		 */
		template <typename T>
		DbLoadComponent& set(ValueTypeT<T> v) {
			m_[typeid(T)] = std::make_unique<Data<T>>(std::move(v));
			return *this;
		}

		/**
		 * Returns true IFF an option with type `T` exists.
		 *
		 * @tparam T the option type
		 */
		template <typename T>
		bool has() const {
			return m_.find(typeid(T)) != m_.end();
		}

		/**
		 * Erases the option specified by the type `T`.
		 *
		 * @tparam T the option type
		 */
		template <typename T>
		void unset() {
			m_.erase(typeid(T));
		}

		/**
		 * Returns a reference to the value for `T`, or a value-initialized default
		 * if `T` was not set.
		 *
		 * This method will always return a reference to a valid value of the correct
		 * type for option `T`, whether or not `T` has actually been set. Use
		 * `has<T>()` to check whether or not the option has been set.
		 *
		 * @code
		 * struct FooOption {
		 *   using Type = std::set<std::string>;
		 * };
		 * DbLoadComponent opts;
		 * std::set<std::string> const& x = opts.get<FooOption>();
		 * assert(x.empty());
		 * assert(!x.has<FooOption>());
		 *
		 * opts.set<FooOption>({"foo"});
		 * assert(opts.get<FooOption>().size() == 1);
		 * @endcode
		 *
		 * @tparam T the option type
		 */
		template <typename T>
		ValueTypeT<T> const& get() const {
			auto const it = m_.find(typeid(T));
			//if (it == m_.end()) return internal::DBDefaultValue<ValueTypeT<T>>();
			if (it == m_.end())
			{
				std::stringstream ss;
				ss << "DbLoadComponent get type:" << typeid(T).name() << " not exist";
				throw std::logic_error(ss.str());
			}
			auto const* value = it->second->data_address();
			return *reinterpret_cast<ValueTypeT<T> const*>(value);
		}

		/**
		 * Returns a reference to the value for option `T`, setting the value to @p
		 * init_value if necessary.
		 *
		 * @code
		 * struct BigOption {
		 *   using Type = std::set<std::string>;
		 * };
		 * DbLoadComponent opts;
		 * std::set<std::string>& x = opts.lookup<BigOption>();
		 * assert(x.empty());
		 *
		 * x.insert("foo");
		 * opts.lookup<BigOption>().insert("bar");
		 * assert(x.size() == 2);
		 * @endcode
		 *
		 * @tparam T the option type
		 * @param value the initial value to use if `T` is not set (optional)
		 */
		template <typename T>
		ValueTypeT<T>& lookup() {
			auto p = m_.find(typeid(T));
			if (p == m_.end()) 
			{
				//p = m_.emplace(typeid(T), std::make_unique<Data<T>>(std::move(value)))
				//	.first;
				std::stringstream ss;
				ss << "DbLoadComponent lookup type:" << typeid(T).name() << " not exist";
				throw std::logic_error(ss.str());
			}
			auto* v = p->second->data_address();
			return *reinterpret_cast<ValueTypeT<T>*>(v);
		}


		void clear()
		{
			m_.clear();
			m_loading.clear();
			m_ready = false;
		}

	public:

		template <typename T>
		DbLoadComponent& setState(E_LoadingState v) {
			m_loading[typeid(T)] = v;

			switch (v)
			{
			case apie::DbLoadComponent::ELS_Failure:
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
			case apie::DbLoadComponent::ELS_Success:
			{
				if (m_ready)
				{
					break;
				}

				bool bDone = true;
				for (const auto& elems : m_loading)
				{
					if (elems.second == apie::DbLoadComponent::ELS_Loading)
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

		void loadFromDb(::rpc_msg::CHANNEL server, ReadyCb cb)
		{
			this->m_cb = cb;

			auto self = this->shared_from_this();
			for (auto& elems : m_)
			{
				elems.second->loadFromDb(self, server);
			}
		}
	private:

		// The type-erased data holder of all the option values.
		class DataHolder {
		public:
			virtual ~DataHolder() = default;
			virtual void const* data_address() const = 0;
			virtual void* data_address() = 0;
			virtual void loadFromDb(std::shared_ptr<apie::DbLoadComponent> loader, ::rpc_msg::CHANNEL server) = 0;
			virtual std::unique_ptr<DataHolder> clone() const = 0;
		};

		// The data holder for all the option values.
		template <typename T>
		class Data : public DataHolder {
		public:
			explicit Data(ValueTypeT<T> v) : value_(std::move(v)) {}
			~Data() override = default;

			void const* data_address() const override { return &value_; }
			void* data_address() override { return &value_; }
			void loadFromDb(std::shared_ptr<apie::DbLoadComponent> loader, ::rpc_msg::CHANNEL server) override
			{
				value_.loadFromDb(loader, server);
			}
			std::unique_ptr<DataHolder> clone() const override {
				return std::make_unique<Data<T>>(*this);
			}

		private:
			ValueTypeT<T> value_;
		};

		// Note that (1) `typeid(T)` returns a `std::type_info const&`, but that
		// implicitly converts to a `std::type_index`, and (2) `std::hash<>` is
		// specialized for `std::type_index` to use `std::type_index::hash_code()`.
		std::unordered_map<std::type_index, std::unique_ptr<DataHolder>> m_;


		std::unordered_map<std::type_index, E_LoadingState> m_loading;

		ReadyCb m_cb;
		bool m_ready = false;
	};


	inline std::shared_ptr<DbLoadComponent> CreateDBLoaderPtr()
	{
		return std::shared_ptr<DbLoadComponent>(new DbLoadComponent());
	}

}

