#ifndef _THREAD_POOL_

#define _THREAD_POOL

#include <cstdint>
#include <thread>
#include <future>
#include <functional>
#include <memory>

using namespace std;

template<typename TThreadPoolImpl>
class thread_pool : public TThreadPoolImpl
{
public:

	thread_pool() = delete;
	thread_pool(const thread_pool&) = delete;
	thread_pool& operator=(const thread_pool&) = delete;
	thread_pool(thread_pool&&) = delete;
	thread_pool& operator=(thread_pool&&) = delete;

	static shared_ptr<thread_pool<TThreadPoolImpl>> create(uint8_t count = 5)
	{
		static shared_ptr<thread_pool<TThreadPoolImpl>> pool{ new thread_pool<TThreadPoolImpl>{count} };

		return pool;
	}

	size_t get_count()
	{
		auto impl = static_cast<TThreadPoolImpl*>(this);
		return impl->get_count();
	}

	template<typename TResult, typename ... TArgs>
	shared_future<TResult> run(function<TResult(TArgs...)> fn, TArgs&& ... args)
	{
		auto impl = static_cast<TThreadPoolImpl*>(this);
		return impl->run(fn, forward<TArgs>(args) ...);
	}

	template<typename TResult>
	shared_future<TResult> run(function<TResult(void)> fn)
	{
		auto impl = static_cast<TThreadPoolImpl*>(this);
		return impl->run(fn);
	}

	template<typename TResult, typename ... TArgs>
	void run(function<TResult(TArgs...)> fn, function<void(shared_future<TResult>)> callback, TArgs&& ... args)
	{
		auto impl = static_cast<TThreadPoolImpl*>(this);
		impl->run(fn, callback, forward<TArgs>(args) ...);
	}

	template<typename TResult>
	void run(function<TResult(void)> fn, function<void(shared_future<TResult>)> callback)
	{
		auto impl = static_cast<TThreadPoolImpl*>(this);
		impl->run(fn, callback);
	}

protected:

	thread_pool(uint8_t count) : TThreadPoolImpl{ count }
	{
	}
};

#endif