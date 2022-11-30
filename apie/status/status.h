#pragma once

#include <string>

#include "apie/status/status_code_enum.h"


namespace apie {
namespace status {


class Status {
public:
	Status()
		: code_(StatusCode::OK), error_message_()
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

	void setErrorMessage(const std::string& msg)
	{
		error_message_ = msg;
	}

	void setHasMore(bool flag)
	{
		has_more_ = flag;
	}

	bool ok() const { return code_ == StatusCode::OK; }
	StatusCode code() const { return code_; }
	std::string message() const { return error_message_; }

	bool hasMore() const { return has_more_; }
	bool isAsync() const { return code_ == StatusCode::OK_ASYNC; }

private:
	StatusCode code_;
	std::string error_message_;
	bool has_more_ = false;
};


}
}


