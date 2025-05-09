#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <optional>

#include "apie/pub_sub/pubsub.h"
#include "apie/singleton/threadsafe_singleton.h"
#include "apie/network/logger.h"

namespace apie {
namespace pubsub {


class PubSubManager {
public:
	using TopicMap = std::unordered_map<uint32_t, std::shared_ptr<PubSubBase>>;

	PubSubManager();
	virtual ~PubSubManager();

	template <typename T>
	bool publish(uint32_t topic, const typename PubSub<T>::MessageType& msg);

	template <typename T>
	std::optional<uint32_t> subscribe(uint32_t topic, const typename PubSub<T>::Callback& callback);

	template <typename T>
	void unsubscribe(uint32_t topic, uint32_t callback_id);

	template <typename T>
	std::shared_ptr<PubSub<T>> getObj(uint32_t topic);

	template <typename T>
	std::shared_ptr<PubSub<T>> getOrCreateObj(uint32_t topic);


private:
	PubSubManager(const PubSubManager&) = delete;
	PubSubManager& operator=(const PubSubManager&) = delete;

	TopicMap topics_;
	uint32_t subscribe_id_ = 0;
};

template <typename T>
bool PubSubManager::publish(uint32_t topic, const typename PubSub<T>::MessageType& msg) {
	auto obj = getOrCreateObj<T>(topic);
	if (obj == nullptr)
	{
		std::stringstream ss;                                                                                                                                  \
		ss << "publish | topic: " << topic << " | type:" << typeid(T).name() << "not match";

		ASYNC_PIE_LOG(PIE_ERROR, "{}", ss.str());
		return false;
	}

	obj->publish(msg);

	return true;
}

template <typename T>
std::optional<uint32_t> PubSubManager::subscribe(uint32_t topic, const typename PubSub<T>::Callback& callback) {
	auto obj = getOrCreateObj<T>(topic);
	if (obj == nullptr)
	{
		std::stringstream ss;                                                                                                                                  \
		ss << "subscribe | topic: " << topic << " | type:" << typeid(T).name() << "not match";                                                                                     \
		PANIC_ABORT(ss.str().c_str());

		return std::nullopt;
	}

	++subscribe_id_;

	bool result = obj->subscribe(subscribe_id_, callback);
	if (!result)
	{
		return std::nullopt;
	}

	return subscribe_id_;
}

template <typename T>
void PubSubManager::unsubscribe(uint32_t topic, uint32_t callback_id)
{
	auto obj = getOrCreateObj<T>(topic);
	if (obj == nullptr)
	{
		std::stringstream ss;                                                                                                                                  \
		ss << "unsubscribe | topic: " << topic << " | type:" << typeid(T).name() << "not match";      
		
		ASYNC_PIE_LOG(PIE_ERROR, "{}", ss.str());
		return;
	}

	obj->unsubscribe(callback_id);
}


template <typename T>
std::shared_ptr<PubSub<T>> PubSubManager::getObj(uint32_t topic)
{
	std::shared_ptr<PubSub<T>> obj = nullptr;

	auto search = topics_.find(topic);
	if (search != topics_.end())
	{
		obj = std::dynamic_pointer_cast<PubSub<T>>(search->second);
	}

	return obj;
}

template <typename T>
std::shared_ptr<PubSub<T>> PubSubManager::getOrCreateObj(uint32_t topic)
{
	std::shared_ptr<PubSub<T>> obj = nullptr;

	auto search = topics_.find(topic);
	if (search != topics_.end())
	{
		obj = std::dynamic_pointer_cast<PubSub<T>>(search->second);
	}
	else
	{
		obj = std::make_shared<PubSub<T>>(topic);
		topics_[topic] = obj;
	}

	return obj;
}


using PubSubManagerSingleton = apie::ThreadSafeSingleton<PubSubManager>;


}
}

