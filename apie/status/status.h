#pragma once

#include <string>

#include "apie/status/status_code_enum.h"


namespace apie {
namespace status {


class Status {
public:
	Status()
		: code_(StatusCode::OK)
	{

	}

	Status(StatusCode code, const std::string& error_message)
		: code_(code), 
		error_message_(error_message)
	{
	}

	void setErrorCode(StatusCode code)
	{
		code_ = code;
	}

	StatusCode error_code() const { return code_; }

	std::string error_message() const { return error_message_; }

	bool ok() const { return code_ == StatusCode::OK; }
	bool isAsync() const { return code_ == StatusCode::OK_ASYNC; }

private:
	StatusCode code_;
	std::string error_message_;
};


}
}


