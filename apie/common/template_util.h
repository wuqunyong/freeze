#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>


namespace apie {

// A list of types
template <typename... P>
struct List {};

namespace internal {

// A helper class template which decides if the TestType occurs in the Tuple
// For example, OccursInTuple<int, std::tuple<float, float>>::value == false,
// and OccursInTuple<int, std::tuple<float, int>>::value == true. Not intended
// to be used directly.

// First declare the template which always takes two parameters.
template <typename TestType, typename Tuple>
class OccursInTuple;

// In the special case where the tuple is empty, the result is false.
template <typename TestType>
class OccursInTuple<TestType, std::tuple<>> : public std::false_type {};

// If the list is not empty, the result is true if TestType equals the first in
// the list, or  TestType occurs in the rest of the list.
template <typename TestType, typename First, typename... List>
class OccursInTuple<TestType, std::tuple<First, List...>>
    : public absl::disjunction<
          std::is_same<TestType, First>,
          OccursInTuple<TestType, typename std::tuple<List...>>> {};

// The class HasDuplicates. Defines ::value as true in case the given list has
// a duplicate, false otherwise.
template <typename... Args>
class HasDuplicates;

// Empty list has no duplicates.
template <>
class HasDuplicates<> : public std::false_type {};

// Non-empty list has a duplicate if the first appears in the rest, or if the
// rest has a duplicate.
template <typename First, typename... List>
class HasDuplicates<First, List...>
    : public absl::disjunction<
          OccursInTuple<First, typename std::tuple<List...>>,
          HasDuplicates<List...>> {};

// The class IndexOf. Defines ::value as zero-based index of first element of
// type T in the List.
template <typename T, typename List>
struct IndexOf;
template <typename T, typename... Elements>
struct IndexOf<T, List<T, Elements...>>
    : public std::integral_constant<std::size_t, 0> {};
template <typename T, typename E, typename... Elements>
struct IndexOf<T, List<E, Elements...>>
    : public std::integral_constant<std::size_t,
                                    1 + IndexOf<T, List<Elements...>>::value> {
};

}  // namespace internal
}  // namespace apie
