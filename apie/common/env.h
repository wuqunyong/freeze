#pragma once

#include <optional>
#include <string>

namespace apie {
namespace common {

// Returns the value of an environment variable or empty.
std::optional<std::string> GetEnv(char const* variable);

// Sets environment variable `variable` to `value`.
void SetEnv(const char* variable, const char* value);

// Removes environment variable `variable`.
void UnsetEnv(const char* variable);

}  // namespace internal
}  // namespace tensorstore
