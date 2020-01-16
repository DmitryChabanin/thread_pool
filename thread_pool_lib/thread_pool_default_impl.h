#ifndef _THREAD_POOL_DEFAULT_IMPL_
#define _THREAD_POOL_DEFAULT_IMPL_

#include <cstdint>
#include <thread>
#include <future>
#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include "work_item_base.h"
#include "cancelation_token.h"

using namespace std;

class thread_pool_default_impl
{
public:
	thread_pool_default_impl(uint8_t count) : _count{ count }, _shutting_down{false}
	{
		for (uint8_t i = 0; i < count; ++i)
		{
			_threads.push_back(thread(fn, this));
		}
	}

	size_t get_count();

	template<typename TResult, typename ... TArgs>
	shared_future<TResult> run(function<TResult(TArgs...)> fn, TArgs&& ... args)
	{
		auto item = make_shared<work_item<TResult, TArgs...>>(fn, forward<TArgs>(args) ...);

		enqueue(item);

		return item->get_future();
	}

	template<typename TResult>
	shared_future<TResult> run(function<TResult(void)> fn)
	{
		auto item = make_shared<work_item<TResult>>(fn);

		enqueue(item);

		return item->get_future();
	}

	template<typename TResult, typename ... TArgs>
	void run(function<TResult(TArgs...)> fn, function<void(shared_future<TResult>)> callback, TArgs&& ... args)
	{
		auto item = make_shared<work_item_with_callback<TResult, TArgs...>>(fn, callback, forward<TArgs>(args) ...);

		enqueue(item);
	}

	template<typename TResult>
	void run(function<TResult(void)> fn, function<void(shared_future<TResult>)> callback)
	{
		auto item = make_shared<work_item_with_callback<TResult>>(fn, callback);

		enqueue(item);
	}

	~thread_pool_default_impl()
	{
		{
			unique_lock<mutex> lock(_sync_obj);

			_shutting_down = true;

			_job_sync.notify_all();
		}

		for(auto &t : _threads)
		{
			t.join();
		}
	}

private:
	uint8_t _count;

	mutex _sync_obj;
	condition_variable _job_sync;

	queue<shared_ptr<work_item_base>> _work_items_queue;
	vector<thread> _threads;

	bool _shutting_down;

	void enqueue(shared_ptr<work_item_base> work_item)
	{
		unique_lock<mutex> lock(_sync_obj);

		_work_items_queue.push(work_item);

		_job_sync.notify_one();
	}

	shared_ptr<work_item_base> dequeue()
	{
		unique_lock<mutex> lock(_sync_obj);

		while (!_shutting_down && _work_items_queue.empty())
		{
			_job_sync.wait(lock);
		}

		if (_shutting_down)
		{
			return nullptr;
		}

		auto work_item = _work_items_queue.front();
		_work_items_queue.pop();

		return work_item;
	}

	static void fn(thread_pool_default_impl* _this)
	{
		while (true)
		{
			shared_ptr<work_item_base> work_item = _this->dequeue();

			if (work_item == nullptr)
			{
				return;
			}

			work_item->run();
		}
	}
};

#endif