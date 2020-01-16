#ifndef _WORK_ITEM_BASE_
#define _WORK_ITEM_BASE_

#include <future>
#include <functional>

#include "cancelation_token.h"

using namespace std;

class work_item_base
{
public:
	void run()
	{
		run_internal();
	}

protected:
	virtual void run_internal() = 0;
};

template<typename TResult, typename ... TArgs>
class work_item : public work_item_base
{
public:

	work_item(function<TResult(TArgs ...)> fn, TArgs&& ... args)
		: _fn{ bind(fn, forward<TArgs>(args) ...) }
	{
	}

	shared_future<TResult> get_future()
	{
		return shared_future<TResult> { _promise.get_future() };
	}

protected:

	void run_internal() override
	{
		try
		{
			_promise.set_value(_fn());
		}
		catch (const exception &e)
		{
			_promise.set_exception(make_exception_ptr(e));
		}
		catch (...)
		{
		}
	}

private:

	function<TResult()> _fn;
	promise<TResult> _promise;
};

template<typename ... TArgs>
class work_item<void, TArgs ...> : public work_item_base
{
public:

	work_item(function<void(TArgs ...)> fn, TArgs&& ... args)
		: _fn{ bind(fn, forward<TArgs>(args) ...) }
	{
	}

	shared_future<void> get_future()
	{
		return shared_future<void> { _promise.get_future() };
	}

protected:

	void run_internal() override
	{
		try
		{
			_fn();

			_promise.set_value();
		}
		catch (const exception &e)
		{
			_promise.set_exception(make_exception_ptr(e));
		}
		catch (...)
		{
		}
	}

private:

	function<void()> _fn;
	promise<void> _promise;
};

template<typename TResult, typename ... TArgs>
class work_item_with_callback : public work_item_base
{
public:

	work_item_with_callback(function<TResult(TArgs ...)> fn, function<void(shared_future<TResult>)> callback, TArgs&& ... args)
		: fn_{ bind(fn, forward<TArgs>(args) ...) }, callback_{ callback }
	{
	}

protected:

	shared_future<TResult> get_future()
	{
		return shared_future<TResult>{ promise_.get_future() };
	}

protected:

	void run_internal() override
	{
		try
		{
			promise_.set_value(fn_());

			callback_(get_future());
		}
		catch (const exception &e)
		{
			promise_.set_exception(make_exception_ptr(e));
		}
		catch (...)
		{
		}
	}

private:

	function<TResult()> fn_;
	function<void(shared_future<TResult>)> callback_;
	promise<TResult> promise_;
};

template<typename ... TArgs>
class work_item_with_callback<void, TArgs ...> : public work_item_base
{
public:

	work_item_with_callback(function<void(TArgs ...)> fn, function<void(shared_future<void>)> callback, TArgs&& ... args)
		: fn_{ bind(fn, forward<TArgs>(args) ...) }, callback_{callback}
	{
	}

protected:

	shared_future<void> get_future()
	{
		return shared_future<void>{ promise_.get_future() };
	}

	void run_internal() override
	{
		try
		{
			fn_();

			promise_.set_value();

			callback_(get_future());
		}
		catch (const exception &e)
		{
			promise_.set_exception(make_exception_ptr(e));
		}
		catch (...)
		{
		}
	}

private:

	function<void()> fn_;
	function<void(shared_future<void>)> callback_;
	promise<void> promise_;
};

#endif //_WORK_ITEM_BASE_