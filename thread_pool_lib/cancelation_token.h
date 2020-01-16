#ifndef _CANCELATION_TOKEN_
#define _CANCELATION_TOKEN_

#include <memory>
#include <exception>
#include <mutex>

using namespace std;

class cancelation_token_source;

class cancel_exception : public exception
{
};

class cancelation_token
{
public:
	cancelation_token() : _canceled{ false }
	{
	}

	bool canceled()
	{
		lock_guard<mutex> lock(_sync_obj);

		return _canceled;
	}

	void throw_if_canceled()
	{
		lock_guard<mutex> lock(_sync_obj);

		if (_canceled)
		{
			throw cancel_exception();
		}
	}

protected:

	void set_canceled()
	{
		lock_guard<mutex> lock(_sync_obj);

		_canceled = true;
	}

private:

	bool _canceled;

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
