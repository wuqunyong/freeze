#pragma once

#include <string>
#include <map>

#include "apie/http/http_common.h"

namespace apie
{
    class HttpResponse
    {
    public:
        HttpResponse();

        HttpResponse(int code, const Headers_t& headers = Headers_t(), const std::string& body = "");
        ~HttpResponse();    

        void clear()
        {
            statusCode = 200;
            body.clear();
            headers.clear();    
        }

        void setContentType(const std::string& contentType);
        void setKeepAlive(bool on);

        void enableDate();

        //generate http response text
        std::string dump();
   
        int statusCode;
        std::string body;

		std::string xLogRequestid;
        
        Headers_t headers;
    };
    
}

