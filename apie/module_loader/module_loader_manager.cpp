#include "apie/module_loader/module_loader_manager.h"

namespace apie {
namespace module_loader {

	ModuleLoaderManager::ModuleLoaderManager()
	{
		if (!m_init)
		{
			m_init = true;
			apie::hook::APieModuleObj(ModuleLoaderManager::APieModuleHookHandler);
		}

		m_timeOut = std::chrono::milliseconds(1000 * 120);
		m_stepDuration = std::chrono::milliseconds(1000);
	}

	ModuleLoaderManager::~ModuleLoaderManager()
	{
	}

	void ModuleLoaderManager::checkStartFinish()
	{
		m_timeOut -= m_stepDuration;

		bool bResult = true;
		for (auto& elems : m_loader)
		{
			if (!elems.second->getHookReady(hook::HookPoint::HP_Start))
			{
				bResult = false;
				break;
			}
		}

		if (bResult)
		{
			apie::hook::HookRegistrySingleton::get().triggerHook(hook::HookPoint::HP_Ready);
		}
		else
		{
			if (m_timeOut.count() > 0)
			{
				auto cb = std::bind(&ModuleLoaderManager::checkStartFinish, this);
				auto ptrTimer = apie::event_ns::EphemeralTimerMgrSingleton::get().createEphemeralTimer(cb);
				ptrTimer->enableTimer(m_stepDuration.count());
			} 
			else
			{
				std::stringstream ss;
				ss << "hook start check ready timeout";
				PANIC_ABORT(ss.str().c_str());
			}
		}
	}

	status::Status ModuleLoaderManager::hookHandler(hook::HookPoint point)
	{
		apie::status::Status curState(apie::status::StatusCode::OK, "");

		std::vector<std::shared_ptr<ModuleLoaderBase>> loaderVec;
		for (auto& elems : m_loader)
		{
			loaderVec.push_back(elems.second);
		}

		auto myCmp = [](std::shared_ptr<ModuleLoaderBase>& lhs, std::shared_ptr<ModuleLoaderBase>& rhs) {
			return lhs->getPriority() < rhs->getPriority();
		};
		std::sort(loaderVec.begin(), loaderVec.end(), myCmp);

		if (point == hook::HookPoint::HP_Exit)
		{
			std::reverse(loaderVec.begin(), loaderVec.end());
		}

		for (auto& elems : loaderVec)
		{
			switch (point)
			{
			case apie::hook::HookPoint::HP_Init:
			{
				auto status = elems->init();
				if (!status.ok())
				{
					return status;
				}
				else
				{
					elems->setHookReady(point);
				}
				break;
			}
			case apie::hook::HookPoint::HP_Start:
			{
				auto status = elems->start();
				if (!status.ok())
				{
					if (status.isAsync())
					{
						curState.setErrorCode(status::StatusCode::OK_ASYNC);
						continue;
					}
					return status;
				}
				else
				{
					elems->setHookReady(point);
				}
				break;
			}
			case apie::hook::HookPoint::HP_Ready:
			{
				auto status = elems->ready();
				if (!status.ok())
				{
					return status;
				}
				else
				{
					elems->setHookReady(point);
				}
				break;
			}
			case apie::hook::HookPoint::HP_Exit:
			{
				auto status = elems->exit();
				if (!status.ok())
				{
					//nothing
				}
				else
				{
					elems->setHookReady(point);
				}
				break;
			}
			default:
			{
				curState.setErrorCode(status::StatusCode::UNKNOWN);
				return curState;
			}
			}
		}

		if (curState.isAsync() && point == apie::hook::HookPoint::HP_Start)
		{
			auto cb = std::bind(&ModuleLoaderManager::checkStartFinish, this);
			auto ptrTimer = apie::event_ns::EphemeralTimerMgrSingleton::get().createEphemeralTimer(cb);
			ptrTimer->enableTimer(m_stepDuration.count());
		}
		return curState;
	}

	apie::status::Status ModuleLoaderManager::APieModuleHookHandler(hook::HookPoint point)
	{
		return module_loader::ModuleLoaderMgrSingleton::get().hookHandler(point);
	}
}
}