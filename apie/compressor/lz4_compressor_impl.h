#pragma once

#include <lz4.h>
#include <lz4hc.h>
#include <lz4frame.h>

#include "apie/compressor/compressor.h"

namespace apie {
namespace compressor {


class LZ4CompressorImpl : public Compressor {
public:
	LZ4CompressorImpl();
	~LZ4CompressorImpl();

	// Compressor
	std::optional<std::string> compress(const std::string& data, int level) override;

private:
	std::optional<std::string> doCompress(const std::string& data, int level);
};

} // namespace Compressor
} // namespace Envoy
