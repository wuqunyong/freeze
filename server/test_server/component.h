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

class Component;
namespace internal {

template <typename T>
inline T const& DefaultValue() {
  static auto const* const kDefaultValue = new T{};
  return *kDefaultValue;
}

}  // namespace internal


class Component {
 private:
  template <typename T>
  using ValueTypeT = typename T::Type;

 public:
  /// Constructs an empty instance.
  Component() = default;

  Component(Component const& rhs) {
    for (auto const& kv : rhs.m_) m_.emplace(kv.first, kv.second->clone());
  }
  Component& operator=(Component const& rhs) {
    Component tmp(rhs);
    std::swap(m_, tmp.m_);
    return *this;
  }
  Component(Component&&) = default;
  Component& operator=(Component&&) = default;

  /**
   * Sets option `T` to the value @p v and returns a reference to `*this`.
   *
   * @code
   * struct FooOption {
   *   using Type = int;
   * };
   * auto opts = Component{}.set<FooOption>(123);
   * @endcode
   *
   * @tparam T the option type
   * @param v the value to set the option T
   */
  template <typename T>
  Component& set(ValueTypeT<T> v) {
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
   * Component opts;
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
    if (it == m_.end()) return internal::DefaultValue<ValueTypeT<T>>();
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
   * Component opts;
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
  ValueTypeT<T>& lookup(ValueTypeT<T> value = {}) {
    auto p = m_.find(typeid(T));
    if (p == m_.end()) {
      p = m_.emplace(typeid(T), std::make_unique<Data<T>>(std::move(value)))
              .first;
    }
    auto* v = p->second->data_address();
    return *reinterpret_cast<ValueTypeT<T>*>(v);
  }

 private:

  // The type-erased data holder of all the option values.
  class DataHolder {
   public:
    virtual ~DataHolder() = default;
    virtual void const* data_address() const = 0;
    virtual void* data_address() = 0;
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
};

} 
