#include "thread_pool_default_impl.h"

size_t thread_pool_default_impl::get_count()
{
	unique_lock<mutex> lock(_sync_obj);

	return _threads.size();
}
