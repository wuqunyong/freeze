#pragma once

#include <string>

#include "apie/status/status_code_enum.h"


namespace apie {
namespace status {

class Status {
 public:
  /// Construct an OK instance.
  Status() : code_(StatusCode::OK) 
  {

  }

  /// Construct an instance with associated \a code and \a error_message.
  /// It is an error to construct an OK status with non-empty \a error_message.
  Status(StatusCode code, const std::string& error_message)
      : code_(code), error_message_(error_message) {}



  /// Return the instance's error code.
  StatusCode error_code() const { return code_; }

  /// Return the instance's error message.
  std::string error_message() const { return error_message_; }


  /// Is the status OK?
  bool ok() const { return code_ == StatusCode::OK ||  code_ == StatusCode::OK_ASYNC; }
  bool isAsync() const { return code_ == StatusCode::OK_ASYNC; }

 private:
  StatusCode code_;
  std::string error_message_;
};

}
}


