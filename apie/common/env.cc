#include "apie/common/env.h"

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>

namespace apie {
namespace common {

std::optional<std::string> GetEnv(char const* variable) {
#if _WIN32
  // On Windows, std::getenv() is not thread-safe. It returns a pointer that
  // can be invalidated by _putenv_s(). We must use the thread-safe alternative,
  // which unfortunately allocates the buffer using malloc():
  char* buffer;
  std::size_t size;
  _dupenv_s(&buffer, &size, variable);
  std::unique_ptr<char, decltype(&free)> release(buffer, &free);
#else
  char* buffer = std::getenv(variable);
#endif  // _WIN32
  if (buffer == nullptr) {
    return std::optional<std::string>();
  }
  return std::optional<std::string>(std::string{buffer});
}

void SetEnv(const char* variable, const char* value) {
#if _WIN32
  ::_putenv_s(variable, value);
#else
  ::setenv(variable, value, 1);
#endif
}

void UnsetEnv(const char* variable) {
#if _WIN32
  ::_putenv_s(variable, "");
#else
  ::unsetenv(variable);
#endif
}

}  // namespace internal
}  // namespace tensorstore
