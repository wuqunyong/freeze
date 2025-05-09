#include "apie/redis_driver/redis_client.h"

#include "apie/network/command.h"
#include "apie/network/ctx.h"
#include "apie/network/logger.h"
#include "apie/rpc/client/rpc_client.h"
#include "apie/event/timer_impl.h"

namespace apie {

	RedisClient::RedisClient(Key key, const std::string &host, std::size_t port, const std::string &password, Cb cb) :
		m_key(key),
		m_host(host),
		m_port(port),
		m_password(password),
		m_cb(cb)
	{
		ASYNC_PIE_LOG(PIE_NOTICE, "Redis/RedisClient|ctor|key:{}-{}", (uint32_t)std::get<0>(key), std::get<1>(key));

		auto timerCb = [this]() {
			if (this->client().is_connected())
			{
				this->addReconnectTimer(3000);
				return;
			}

			std::stringstream ss;
			ss << "host:" << m_host << "|port:" << m_port << "|is_reconnecting:" << this->client().is_reconnecting();
			ASYNC_PIE_LOG(PIE_WARNING, "Redis/ReconnectTimer|{}", ss.str().c_str());

			if (this->getState() == RS_Closed)
			{
				try {
					auto ptrCb = this->getAdapterCb();
					m_client.connect(m_host, m_port, ptrCb);
					m_status = RS_Connect;
				}
				catch (std::exception& e) {
					std::stringstream ss;
					ss << "redis connect|Unexpected exception: " << e.what();

					PIE_LOG(PIE_ERROR, "{}: {}", "Exception", ss.str().c_str());
				}
			}

			this->addReconnectTimer(3000);
		};
		this->m_reconnectTimer = apie::CtxSingleton::get().getLogicThread()->dispatcher().createTimer(timerCb);
	}

	RedisClient::~RedisClient()
	{
		ASYNC_PIE_LOG(PIE_NOTICE, "Redis/RedisClient|destructor|key:{}-{}", (uint32_t)std::get<0>(m_key), std::get<1>(m_key));

		this->disableReconnectTimer();

		if (m_client.is_connected())
		{
			m_client.disconnect();
		}
	}

	void RedisClient::start()
	{
		if (m_started == 1)
		{
			return;
		}

		m_started = 1;

		std::weak_ptr<RedisClient> weak_this = shared_from_this();

		auto ptrCb = [weak_this](const std::string &host, std::size_t port, cpp_redis::client::connect_state status) {
			
			std::shared_ptr<RedisClient> shared_this = weak_this.lock();
			if (shared_this == nullptr) {
				std::stringstream ss;
				ss << "host:" << host << "|port:" << port << "|shared_this null";
				PANIC_ABORT("Redis/Redis_ConnectCb", PIE_CYCLE_DAY, PIE_ERROR, "%s", ss.str().c_str());
				return;
			}


			std::stringstream ss;
			ss << "host:" << host << "|port:" << port << "|status:" << (uint32_t)status;
			ASYNC_PIE_LOG(PIE_NOTICE, "Redis/Redis_ConnectCb|{}", ss.str().c_str());

			switch (status)
			{
			case cpp_redis::client::connect_state::dropped:
				break;
			case cpp_redis::client::connect_state::start:
			{
				break;
			}
			case cpp_redis::client::connect_state::sleeping:
				break;
			case cpp_redis::client::connect_state::ok:
			{
				shared_this->setState(RS_Auth);

				if (shared_this->getPassword().empty())
				{
					shared_this->setState(RS_Established);
					shared_this->getCb()(shared_this);
					return;
				}

				auto ptrAuth = [host, port, weak_this](cpp_redis::reply &reply) {
					std::shared_ptr<RedisClient> shared_this = weak_this.lock();
					if (shared_this == nullptr) {

						std::stringstream ss;
						ss << "host:" << host << "|port:" << port << "|auth shared_this null";
						ASYNC_PIE_LOG(PIE_ERROR, "Redis/Redis_ConnectCb|{}", ss.str().c_str());

						PANIC_ABORT(ss.str().c_str());
						return;
					}

					if (reply.is_error())
					{
						shared_this->setAuth(RA_Error);

						std::stringstream ss;
						ss << "redis reply:" << reply.error();
						PIE_LOG(PIE_ERROR, "Redis_Auth|{}", ss.str().c_str());

						PANIC_ABORT(ss.str().c_str());
						return;
					}

					if (reply.is_bulk_string() && reply.as_string() == "OK")
					{
						std::stringstream ss;
						ss << "redis reply:" << reply.as_string();
						ASYNC_PIE_LOG(PIE_NOTICE, "Redis/Redis_Auth|{}", ss.str().c_str());

						shared_this->setAuth(RA_Ok);
						shared_this->setState(RS_Established);
						shared_this->getCb()(shared_this);
					}
					else
					{
						std::stringstream ss;
						ss << "redis reply auth|type:" << (uint32_t)reply.get_type();
						if (reply.is_bulk_string())
						{
							ss << "string Value:" << reply.as_string();
						}
						PIE_LOG(PIE_ERROR, "Redis_Auth|{}", ss.str().c_str());

						PANIC_ABORT(ss.str().c_str());
					}
				};
				shared_this->client().auth(shared_this->getPassword(), ptrAuth);
				shared_this->client().commit();
				shared_this->setAuth(RA_Doing);
				break;
			}
			case cpp_redis::client::connect_state::failed:
				break;
			case cpp_redis::client::connect_state::lookup_failed:
				break;
			case cpp_redis::client::connect_state::stopped:
			{
				shared_this->setState(RS_Closed);
				shared_this->setAuth(RA_None);
				break;
			}
			default:
				break;
			}

			shared_this->getCb()(shared_this);
		};

		m_adapterCb = ptrCb;
		m_client.connect(m_host, m_port, ptrCb);
		m_status = RS_Connect;
	}

	void RedisClient::stop()
	{
		if (this->m_reconnectTimer->enabled())
		{
			this->disableReconnectTimer();
		}
	}

	void RedisClient::addReconnectTimer(uint64_t interval)
	{
		this->m_reconnectTimer->enableTimer(std::chrono::milliseconds(interval));
	}

	void RedisClient::disableReconnectTimer()
	{
		this->m_reconnectTimer->disableTimer();
	}

	RedisClient::RedisStatus RedisClient::getState()
	{
		return m_status;
	}

	void RedisClient::setState(RedisStatus status)
	{
		m_status = status;
	}

	cpp_redis::client& RedisClient::client()
	{
		return m_client;
	}

	RedisClient::Cb& RedisClient::getCb()
	{
		return m_cb;
	}

	RedisClient::AdapterCb& RedisClient::getAdapterCb()
	{
		return m_adapterCb;
	}

	std::string& RedisClient::getPassword()
	{
		return m_password;
	}

	RedisClient::Key RedisClient::getKey()
	{
		return m_key;
	}

	void RedisClient::setAuth(RedisAuth value)
	{
		m_auth = value;
	}

	cpp_redis::client::reply_callback_t RedisClient::WrapperFunc(cpp_redis::client::reply_callback_t cb)
	{
		auto wrapsCb = [cb](cpp_redis::reply& replyData) mutable {
			auto funObj = [cb, replyData]() mutable {
				cb(replyData);
			};
			apie::CtxSingleton::get().getLogicThread()->dispatcher().post(funObj);
		};

		return wrapsCb;
	}


	bool RedisClientFactory::registerClient(std::shared_ptr<RedisClient> ptrClient)
	{
		auto key = ptrClient->getKey();

		auto findIte = m_redisClient.find(key);
		if (findIte != m_redisClient.end())
		{
			return false;
		}

		m_redisClient[key] = ptrClient;
		ptrClient->start();
		ptrClient->addReconnectTimer(10000);

		return true;
	}

	void RedisClientFactory::destroy()
	{
		auto ite = m_redisClient.begin();
		while (ite != m_redisClient.end())
		{
			ite->second->stop();

			auto o = ite;
			++ite;
			m_redisClient.erase(o);
		}
	}

	std::shared_ptr<RedisClient> RedisClientFactory::getClient(RedisClient::Key key)
	{
		auto findIte = m_redisClient.find(key);
		if (findIte == m_redisClient.end())
		{
			return nullptr;
		}

		return findIte->second;
	}

	std::shared_ptr<RedisClient> RedisClientFactory::getConnectedClient(RedisClient::Key key)
	{
		auto ptrClient = this->getClient(key);
		if (ptrClient == nullptr)
		{
			return nullptr;
		}

		if (!ptrClient->client().is_connected())
		{
			return nullptr;
		}

		return ptrClient;
	}

	std::shared_ptr<RedisClient> RedisClientFactory::createClient(RedisClient::Key key, const std::string &host, std::size_t port, const std::string &password, RedisClient::Cb cb)
	{
		auto sharedPtr = std::make_shared<RedisClient>(key, host, port, password, cb);
		return sharedPtr;
	}

}