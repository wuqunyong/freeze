#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include <unordered_map>
#include <coroutine>

#include "apie/proto/init.h"
#include "apie/status/status_or.h"
#include "apie/rpc/init.h"

namespace apie {
namespace co_traits {

	template <typename Request, typename Response>
	class CoAwaitable : public std::enable_shared_from_this<CoAwaitable<Request, Response>>
	{
	public:
		using RequestType = Request;
		using ResponseType = Response;
		using ResponseTypePtr = std::shared_ptr<Response>;


		CoAwaitable(const ::rpc_msg::CHANNEL& server, uint32_t opcode, const RequestType& request)
			: m_server(server),
			m_opcode(opcode),
			m_request(request)
		{

		}

		~CoAwaitable()
		{

		}

		bool await_ready() 
		{
			return false;
		}

		decltype(auto) await_resume()
		{
			return m_response;
		}

		bool await_suspend(std::coroutine_handle<> h)
		{
			auto self = this->shared_from_this();
			auto rpcCB = [self, h](const apie::status::Status& status, const ResponseTypePtr& response) {
				if (!status.ok())
				{
					self->m_response = status;
					h.resume();
					return;
				}

				self->m_response = *response;
				h.resume();
			};
			auto bResult = apie::rpc::RPC_Call<RequestType, ResponseType>(m_server, m_opcode, m_request, rpcCB);
			if (!bResult)
			{
				return false;
			}

			return true;
		}

	private:
		::rpc_msg::CHANNEL m_server;
		uint32_t m_opcode;
		RequestType m_request;
		apie::status::StatusOr<ResponseType> m_response;
	};

}
}

template<typename Request, typename Response>
auto MakeCoAwaitable(const ::rpc_msg::CHANNEL& server, const uint32_t opcode, const Request& request)
{
	return std::make_shared<apie::co_traits::CoAwaitable<Request, Response>>(server, opcode, request);
}