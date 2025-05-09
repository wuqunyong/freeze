#pragma once

#include <cstdint>
#include <type_traits>

namespace apie {

/**
 * Convert an int based enum to an int.
 */
template <typename T> constexpr uint32_t enumToInt(T val) { return static_cast<uint32_t>(val); }

/**
 * Convert an int based enum to a signed int.
 */
template <typename T> constexpr int32_t enumToSignedInt(T val) { return static_cast<int32_t>(val); }


template <typename T>
constexpr auto toUnderlyingType(T value) noexcept
{
	return static_cast<typename std::underlying_type<T>::type>(value);
}

}
