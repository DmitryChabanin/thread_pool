#ifndef _CANCELATION_TOKEN_
#define _CANCELATION_TOKEN_

#include <memory>
#include <exception>
#include <mutex>

using std::lock_guard;
using std::mutex;
using std::exception;
using std::shared_ptr;
using std::make_shared;

class cancelation_token_source;

class cancel_exception : public exception
{
};

class cancelation_token
{
public:
	cancelation_token() : _iscanceled{ false }
	{
	}

	bool is_canceled()
	{
		lock_guard<mutex> lock(_sync_obj);

		return _iscanceled;
	}

	void throw_if_canceled()
	{
		lock_guard<mutex> lock(_sync_obj);

		if (_iscanceled)
		{
			throw cancel_exception();
		}
	}

protected:

	void set_canceled()
	{
		lock_guard<mutex> lock(_sync_obj);

		_iscanceled = true;
	}

private:

	bool _iscanceled;

	mutex _sync_obj;

	friend class cancelation_token_source;
};

class cancelation_token_source
{
public:

	cancelation_token_source() : _token{ make_shared<cancelation_token>() }
	{
	}

	void cancel()
	{
		_token->set_canceled();
	}

	shared_ptr<cancelation_token> get_cancelation_token() const
	{
		return _token;
	}

private:
	shared_ptr<cancelation_token> _token;
};

#endif // _CANCELATION_TOKEN_
