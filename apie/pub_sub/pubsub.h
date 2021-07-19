#pragma once

#include <stddef.h>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace apie {
namespace pubsub {

class PubSubBase {
public:
	virtual ~PubSubBase() = default;

	virtual uint32_t getTopic() = 0;
	virtual void unsubscribe(uint32_t callback_id) = 0;
};

template <typename T>
class PubSub : public PubSubBase {
	friend class PubSubManager;

public:
	using MessageType = T;
	using MessagePtr = std::shared_ptr<T>;
	using Callback = std::function<void(const MessagePtr&)>;
	using CallbackMap = std::unordered_map<uint32_t, Callback>;

	explicit PubSub(uint32_t topic);
	virtual ~PubSub();

	void publish(const MessageType& msg);

	bool subscribe(uint32_t callback_id, const Callback& callback);
	void unsubscribe(uint32_t callback_id) override;

	uint32_t getTopic() override;

private:
	void publish(const MessagePtr& msg);
	void notify(const MessagePtr& msg);

	uint32_t topic_;
	CallbackMap published_callbacks_;

};

template <typename T>
PubSub<T>::PubSub(uint32_t topic) :
	topic_(topic)
{
}

template <typename T>
PubSub<T>::~PubSub() {
}

template <typename T>
void PubSub<T>::publish(const MessageType& msg) {
	publish(std::make_shared<MessageType>(msg));
}

template <typename T>
void PubSub<T>::publish(const MessagePtr& msg)
{
	notify(msg);
}

template <typename T>
void PubSub<T>::notify(const MessagePtr& msg) {
	for (const auto& item : published_callbacks_)
	{
		item.second(msg);
	}
}

template <typename T>
bool PubSub<T>::subscribe(uint32_t callback_id, const Callback& callback)
{

	if (published_callbacks_.find(callback_id) != published_callbacks_.end())
	{
		return false;
	}

	published_callbacks_[callback_id] = callback;
	return true;
}

template <typename T>
void PubSub<T>::unsubscribe(uint32_t callback_id)
{
	published_callbacks_.erase(callback_id);
}

template <typename T>
uint32_t PubSub<T>::getTopic()
{
	return topic_;
}


}
}

