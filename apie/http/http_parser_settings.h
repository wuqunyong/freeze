#pragma once

#include <string>

#include "http_parser.h"

#include "apie/http/http_common.h"
#include "apie/common/noncopyable.h"

namespace apie
{
	class HttpParserSettings
	{
	public:
		HttpParserSettings();

		static void initSettings(struct http_parser_settings& settings);

		static int onMessageBegin(struct http_parser*);
		static int onUrl(struct http_parser*, const char*, size_t);
		static int onStatus(struct http_parser*, const char*, size_t);
		static int onHeaderField(struct http_parser*, const char*, size_t);
		static int onHeaderValue(struct http_parser*, const char*, size_t);
		static int onHeadersComplete(struct http_parser*);
		static int onBody(struct http_parser*, const char*, size_t);
		static int onMessageComplete(struct http_parser*);
	};
}
