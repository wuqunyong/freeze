#pragma once

#include <string>
#include <optional>

#include "apie/common/pure.h"

namespace apie {
namespace decompressor {

/**
 * Allows decompressing data.
 */
class Decompressor {
public:
  virtual ~Decompressor() {}

  virtual std::optional<std::string> decompress(const std::string& in) PURE;
};

} // namespace Decompressor
} // namespace Envoy
