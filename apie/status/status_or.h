#pragma once

#include <type_traits>
#include <utility>
#include <optional>

#include "apie/status/status.h"
#include "apie/status/status_code_enum.h"
#include "apie/common/enum_to_int.h"

namespace apie {
namespace status {

	/**
		* Holds a value or a `Status` indicating why there is no value.
		*
		* `StatusOr<T>` represents either a usable `T` value or a `Status` object
		* explaining why a `T` value is not present. Typical usage of `StatusOr<T>`
		* looks like usage of a smart pointer, or even a std::optional<T>, in that you
		* first check its validity using a conversion to bool (or by calling
		* `StatusOr::ok()`), then you may dereference the object to access the
		* contained value. It is undefined behavior (UB) to dereference a
		* `StatusOr<T>` that is not "ok". For example:
		*
		* @code
		* StatusOr<Foo> foo = FetchFoo();
		* if (!foo) {  // Same as !foo.ok()
		*   // handle error and probably look at foo.status()
		* } else {
		*   foo->DoSomethingFooey();  // UB if !foo
		* }
		* @endcode
		*
		* Alternatively, you may call the `StatusOr::value()` member function,
		* which is defined to throw an exception if there is no `T` value, or crash
		* the program if exceptions are disabled. It is never UB to call
		* `.value()`.
		*
		* @code
		* StatusOr<Foo> foo = FetchFoo();
		* foo.value().DoSomethingFooey();  // May throw/crash if there is no value
		* @endcode
		*
		* Functions that can fail will often return a `StatusOr<T>` instead of
		* returning an error code and taking a `T` out-param, or rather than directly
		* returning the `T` and throwing an exception on error. StatusOr is used so
		* that callers can choose whether they want to explicitly check for errors,
		* crash the program, or throw exceptions. Since constructors do not have a
		* return value, they should be designed in such a way that they cannot fail by
		* moving the object's complex initialization logic into a separate factory
		* function that itself can return a `StatusOr<T>`. For example:
		*
		* @code
		* class Bar {
		*  public:
		*   Bar(Arg arg);
		*   ...
		* };
		* StatusOr<Bar> MakeBar() {
		*   ... complicated logic that might fail
		*   return Bar(std::move(arg));
		* }
		* @endcode
		*
		* `StatusOr<T>` supports equality comparisons if the underlying type `T` does.
		*
		* TODO(...) - the current implementation is fairly naive with respect to `T`,
		*   it is unlikely to work correctly for reference types, types without default
		*   constructors, arrays.
		*
		* @tparam T the type of the value.
		*/
	template <typename T>
	class StatusOr final {
	public:
		/**
			* A `value_type` member for use in generic programming.
			*
			* This is analogous to that of `std::optional::value_type`.
			*/
		using value_type = T;

		/**
			* Initializes with an error status (UNKNOWN).
			*/
		StatusOr() : StatusOr(MakeDefaultStatus()) {}

		StatusOr(StatusOr const&) = default;
		StatusOr& operator=(StatusOr const&) = default;
		// NOLINTNEXTLINE(performance-noexcept-move-constructor)
		StatusOr(StatusOr&& other)
			: status_(std::move(other.status_)), value_(std::move(other.value_)) {
			other.status_ = MakeDefaultStatus();
		}
		// NOLINTNEXTLINE(performance-noexcept-move-constructor)
		StatusOr& operator=(StatusOr&& other) {
			status_ = std::move(other.status_);
			value_ = std::move(other.value_);
			other.status_ = MakeDefaultStatus();
			return *this;
		}

		/**
			* Creates a new `StatusOr<T>` holding the error condition @p rhs.
			*
			* @par Post-conditions
			* `ok() == false` and `status() == rhs`.
			*
			* @param rhs the status to initialize the object.
			* @throws std::invalid_argument if `rhs.ok()`. If exceptions are disabled the
			*     program terminates via `google::cloud::Terminate()`
			*/
			// NOLINTNEXTLINE(google-explicit-constructor)
		StatusOr(Status rhs) : status_(std::move(rhs)) {
			if (status_.ok()) {
				throw std::logic_error("ThrowInvalidArgument");
			}
		}

		/**
			* Assigns the given non-OK Status to this `StatusOr<T>`.
			*
			* @throws std::invalid_argument if `status.ok()`. If exceptions are disabled
			*     the program terminates via `google::cloud::Terminate()`
			*/
		StatusOr& operator=(Status status) {
			*this = StatusOr(std::move(status));
			return *this;
		}

		/**
			* Assign a `T` (or anything convertible to `T`) into the `StatusOr`.
			*/
			// Disable this assignment if U==StatusOr<T>. Well, really if U is a
			// cv-qualified version of StatusOr<T>, so we need to apply std::decay<> to
			// it first.
		template <typename U = T>
		typename std::enable_if<  // NOLINT(misc-unconventional-assign-operator)
			!std::is_same<StatusOr, typename std::decay<U>::type>::value,
			StatusOr>::type&
			operator=(U&& rhs) {
			status_ = Status();
			value_ = std::forward<U>(rhs);
			return *this;
		}

		/**
			* Creates a new `StatusOr<T>` holding the value @p rhs.
			*
			* @par Post-conditions
			* `ok() == true` and `value() == rhs`.
			*
			* @param rhs the value used to initialize the object.
			*
			* @throws only if `T`'s move constructor throws.
			*/
			// NOLINTNEXTLINE(google-explicit-constructor)
		StatusOr(T&& rhs) : value_(std::move(rhs)) {}

		// NOLINTNEXTLINE(google-explicit-constructor)
		StatusOr(T const& rhs) : value_(rhs) {}

		bool ok() const { return status_.ok(); }
		explicit operator bool() const { return status_.ok(); }

		//@{
		/**
			* @name Deference operators.
			*
			* @warning Using these operators when `ok() == false` results in undefined
			*     behavior.
			*
			* @return All these return a (properly ref and const-qualified) reference to
			*     the underlying value.
			*/
		T& operator*()& { return *value_; }

		T const& operator*() const& { return *value_; }

		T&& operator*()&& { return *std::move(value_); }

		T const&& operator*() const&& { return *std::move(value_); }
		//@}

		//@{
		/**
			* @name Member access operators.
			*
			* @warning Using these operators when `ok() == false` results in undefined
			*     behavior.
			*
			* @return All these return a (properly ref and const-qualified) pointer to
			*     the underlying value.
			*/
		T* operator->()& { return &*value_; }

		T const* operator->() const& { return &*value_; }
		//@}

		//@{
		/**
			* @name Value accessors.
			*
			* @return All these member functions return a (properly ref and
			*     const-qualified) reference to the underlying value.
			*
			* @throws `RuntimeStatusError` with the contents of `status()` if the object
			*   does not contain a value, i.e., if `ok() == false`.
			*/
		T& value()& {
			CheckHasValue();
			return **this;
		}

		T const& value() const& {
			CheckHasValue();
			return **this;
		}

		T&& value()&& {
			CheckHasValue();
			return std::move(**this);
		}

		T const&& value() const&& {
			CheckHasValue();
			return std::move(**this);
		}
		//@}

		//@{
		/**
			* @name Status accessors.
			*
			* @return A reference to the contained `Status`.
			*/
		Status const& status() const& { return status_; }
		Status&& status()&& { return std::move(status_); }
		//@}

	private:
		static Status MakeDefaultStatus() {
			return Status{ apie::status::StatusCode::UNKNOWN, "default" };
		}

		void CheckHasValue() const& {
			if (!ok()) {
				std::stringstream ss;
				ss << "ErrorCode:" << apie::toUnderlyingType(status_.code()) << ", ErrorMsg:" << status_.message();

				throw std::logic_error(ss.str());
			}
		}

		// When possible, do not copy the status.
		void CheckHasValue()&& {
			if (!ok()) {
				std::stringstream ss;
				ss << "ErrorCode:" << apie::toUnderlyingType(status_.code()) << ", ErrorMsg:" << status_.message();

				throw std::logic_error(ss.str());
			}
		}

		Status status_;
		std::optional<T> value_;
	};


}
}
