#pragma once

#include <string>
#include <map>
#include <stdint.h>

#include "http_parser.h"

#include "apie/http/http_common.h"

namespace apie
{
    class HttpRequest
    {
    public:
        HttpRequest();
        ~HttpRequest();

        void clear();
        void parseUrl();
        std::string dump();

        std::string url;
        std::string body;

        std::string schema;
        
        std::string host;
        std::string path;
        std::string query;

        Headers_t headers;

        Params_t params;
        
        unsigned short majorVersion;
        unsigned short minorVersion;

        http_method method;

        uint16_t port;

		std::string xLogRequestid;

        void parseQuery();

		bool getQuery(std::string field, std::string &value);
		bool getHeaders(std::string field, std::string &value);
		std::string formatHeaders();
    };
        
}

