#include "service_init.h"

namespace apie {

apie::status::Status APieModuleObj(hook::HookPoint point)
{
	return module_loader::ModuleLoaderMgrSingleton::get().hookHandler(point);
}

}

