#include "apie/common/protobuf_factory.h"

namespace apie {
namespace message {
	
google::protobuf::Message* ProtobufFactory::createMessage(const std::string& typeName)
{
	google::protobuf::Message* message = NULL;
	const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	if (descriptor)
	{
		const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype)
		{
			message = prototype->New();
		}
	}
	return message;
}

}
}