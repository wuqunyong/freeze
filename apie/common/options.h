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
namespace common {

class Options;
namespace internal {

template <typename T>
inline T const& DefaultValue() {
  static auto const* const kDefaultValue = new T{};
  return *kDefaultValue;
}

}  // namespace internal

/**
 * A class that holds option structs indexed by their type.
 *
 * An "Option" is any struct that has a public `Type` member typedef. By
 * convention they are named like "FooOption". Each library (e.g., spanner,
 * storage) may define their own set of options. Additionally, there are common
 * options defined that many libraries may use. All these options may be set in
 * a single `Options` instance, and each library will look at the options that
 * it needs.
 *
 * Here's an overview of this class's interface, but see the method
 * documentation below for details.
 *
 * - `.set<T>(x)`    -- Sets the option `T` to value `x`
 * - `.has<T>()`     -- Returns true iff option `T` is set
 * - `.unset<T>()`   -- Removes the option `T`
 * - `.get<T>()`     -- Gets a const-ref to the value of option `T`
 * - `.lookup<T>(x)` -- Gets a non-const-ref to option `T`'s value, initializing
 *                      it to `x` if it was not set (`x` is optional).
 *
 * @par Example:
 *
 * @code
 * struct FooOption {
 *   using Type = int;
 * };
 * struct BarOption {
 *   using Type = std::set<std::string>;
 * };
 * ...
 * Options opts;
 *
 * assert(opts.get<FooOption>() == 0);
 * opts.set<FooOption>(42);
 * assert(opts.get<FooOption>() == 42);
 *
 * // Inserts two elements directly into the BarOption's std::set.
 * opts.lookup<BarOption>().insert("hello");
 * opts.lookup<BarOption>().insert("world");
 *
 * std::set<std::string> const& bar = opts.get<BarOption>();
 * assert(bar == std::set<std::string>{"hello", "world"});
 * @endcode
 */
class Options {
 private:
  template <typename T>
  using ValueTypeT = typename T::Type;

 public:
  /// Constructs an empty instance.
  Options() = default;

  Options(Options const& rhs) {
    for (auto const& kv : rhs.m_) m_.emplace(kv.first, kv.second->clone());
  }
  Options& operator=(Options const& rhs) {
    Options tmp(rhs);
    std::swap(m_, tmp.m_);
    return *this;
  }
  Options(Options&&) = default;
  Options& operator=(Options&&) = default;

  /**
   * Sets option `T` to the value @p v and returns a reference to `*this`.
   *
   * @code
   * struct FooOption {
   *   using Type = int;
   * };
   * auto opts = Options{}.set<FooOption>(123);
   * @endcode
   *
   * @tparam T the option type
   * @param v the value to set the option T
   */
  template <typename T>
  Options& set(ValueTypeT<T> v) {
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
   * Options opts;
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
   * Options opts;
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

}  // namespace cloud
}  // namespace google

