#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <sstream>
#include <unordered_map>
#include <coroutine>

namespace apie {
namespace co_traits {


	class CoTask {
	public:
		class promise_type final
		{
		public:
			promise_type() noexcept
				: m_exception(nullptr)
			{
				m_iId = genId();
			}

			~promise_type()
			{

			}

			promise_type(const promise_type&) = delete;
			promise_type(promise_type&&) = delete;

			uint64_t get_id()
			{
				return m_iId;
			}

			auto get_return_object() noexcept 
			{ 
				return CoTask{ std::coroutine_handle<promise_type>::from_promise(*this) };
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

		private:
			uint64_t m_iId = 0;
			std::exception_ptr m_exception;

			static inline uint64_t s_iId = 0;
		};

		using Handle = std::coroutine_handle<promise_type>;

		explicit CoTask(const Handle coroutine) :
			m_coroutine{ coroutine }
		{
		}

		CoTask() :
			m_coroutine(nullptr)
		{

		}

		~CoTask() 
		{
			//if (m_coroutine) 
			//{
			//	if (!m_coroutine.done())
			//	{
			//		m_coroutine.destroy();
			//	}
			//}
		}

		CoTask(const CoTask&) = delete;
		CoTask& operator=(const CoTask&) = delete;

		CoTask(CoTask&& other) noexcept :
			m_coroutine{ other.m_coroutine }
		{
			other.m_coroutine = {};
		}
		CoTask& operator=(CoTask&& other) noexcept 
		{
			if (this != &other) 
			{
				if (m_coroutine) 
				{
					m_coroutine.destroy();
				}
				m_coroutine = other.m_coroutine;
				other.m_coroutine = {};
			}
			return *this;
		}

	private:
		Handle m_coroutine;
	};


}
}
