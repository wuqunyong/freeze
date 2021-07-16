#pragma once

#include <map>
#include <tuple> 
#include <optional>

#include <google/protobuf/message.h>

namespace apie {
namespace message {


class ProtobufFactory {
public:
	static google::protobuf::Message* createMessage(const std::string& typeName);
};


}
}
