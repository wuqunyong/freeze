#include "apie/service/service_manager.h"

namespace apie {
namespace service {

	ServiceManager::ServiceManager()
	{

	}

	ServiceManager::~ServiceManager()
	{
		this->destroy();
	}

	std::optional<std::string> ServiceManager::getType(uint32_t opcode)
	{
		auto find_ite = type_.find(opcode);
		if (find_ite == type_.end())
		{
			return std::nullopt;
		}

		return find_ite->second;
	}

	void ServiceManager::destroy()
	{
		type_.clear();
		func_.clear();
		service_.clear();
	}

}  
}
