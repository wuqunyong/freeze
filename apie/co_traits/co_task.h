#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include <unordered_map>
#include <coroutine>
#include <map>

#include "apie/network/logger.h"

namespace apie {
namespace co_traits {

	struct CoroutineTracer
	{
		uint64_t iId = 0;
		uint64_t iCreateTime = 0;
	};

	class CoTask {
	public:
		class promise_type final
		{
		public:
			promise_type() noexcept
				: m_exception(nullptr)
			{
				m_iId = genId();
				addCoroutine(m_iId);
			}

			~promise_type()
			{
				removeCoroutine(m_iId);
			}

			promise_type(const promise_type&) = delete;
			promise_type(promise_type&&) = delete;

			uint64_t get_id()
			{
				return m_iId;
			}

			auto get_return_object() noexcept 
			{ 
				return CoTask{ m_iId };
			}

			//std::suspend_always
			std::suspend_never initial_suspend() noexcept 
			{ 
				return {};
			}

			std::suspend_never final_suspend() noexcept
			{ 
				return {}; 
			}

			void unhandled_exception() noexcept 
			{
				m_exception = std::current_exception();
			}

			void return_void() noexcept 
			{
			}

			//// Don't allow any use of 'co_await' inside the Generator
			//// coroutine.
			//template <typename U>
			//void await_transform(U&& value) = delete;

			void destroy() noexcept 
			{
				std::coroutine_handle<promise_type>::from_promise(*this).destroy();
			}

			void throw_if_exception() 
			{
				if (m_exception != nullptr) 
				{
					std::rethrow_exception(std::move(m_exception));
				}
			}

			bool is_complete() noexcept 
			{
				return std::coroutine_handle<promise_type>::from_promise(*this).done();
			}

			void resume() noexcept 
			{
				std::coroutine_handle<promise_type>::from_promise(*this).resume();
			}

			std::string to_string()
			{
				std::stringstream ss;
				ss << "PromiseId:" << m_iId;

				return ss.str();
			}
		public:
			static inline uint64_t genId()
			{
				s_iId++;
				return s_iId;
			}


			static inline void addCoroutine(uint64_t iId)
			{
				auto ite = s_mIdToTracer.find(iId);
				if (ite != s_mIdToTracer.end())
				{
					return;
				}

				uint64_t iCurTime = time(nullptr);

				CoroutineTracer tracer;
				tracer.iId = iId;
				tracer.iCreateTime = iCurTime;

				s_mIdToTracer[iId] = tracer;
				s_mTimeToId.insert(std::pair<uint64_t, uint64_t>(iCurTime, iId));
			}

			static inline void removeCoroutine(uint64_t iId)
			{
				auto ite = s_mIdToTracer.find(iId);
				if (ite == s_mIdToTracer.end())
				{
					return;
				}

				auto iTime = ite->second.iCreateTime;
				s_mIdToTracer.erase(ite);

				auto itePair = s_mTimeToId.equal_range(iTime);
				for (auto delIte = itePair.first; delIte != itePair.second; ++delIte)
				{
					if (delIte->second == iId)
					{
						s_mTimeToId.erase(delIte);
						return;
					}
				}
			}

		private:
			uint64_t m_iId = 0;
			std::exception_ptr m_exception;

			static inline uint64_t s_iId = 0;

			static inline std::multimap<uint64_t, uint64_t> s_mTimeToId = {};
			static inline std::map<uint64_t, CoroutineTracer>  s_mIdToTracer = {};
		};

		using Handle = std::coroutine_handle<promise_type>;

		explicit CoTask(uint64_t iId) :
			m_iId{ iId }
		{
		}

		CoTask()
		{

		}

		~CoTask() 
		{
		}

		CoTask(const CoTask&) = delete;
		CoTask& operator=(const CoTask&) = delete;

		CoTask(CoTask&& other) noexcept :
			m_iId{ other.m_iId }
		{
			other.m_iId = 0;
		}
		CoTask& operator=(CoTask&& other) noexcept 
		{
			if (this != &other) 
			{
				if (m_iId != 0)
				{
					//Todo
				}
				m_iId = other.m_iId;
				other.m_iId = 0;
			}
			return *this;
		}

	private:
		uint64_t m_iId = 0;
	};

}
}

using CoTaskVoid = apie::co_traits::CoTask;
