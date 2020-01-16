#include "pch.h"
#include "..\thread_pool_lib\thread_pool.h"
#include "..\thread_pool_lib\thread_pool_default_impl.h"

TEST(default_thread_pool_impl, PoolCreation)
{
	auto pool = thread_pool<thread_pool_default_impl>::create();
	EXPECT_EQ(pool->get_count(), 5);
}

TEST(default_thread_pool_impl, RunSimple1)
{
	auto pool = thread_pool<thread_pool_default_impl>::create();

	auto fn = []()->int
	{
		return 42;
	};

	auto feature = pool->run<int>(fn);

	EXPECT_EQ(feature.get(), 42);
}

TEST(default_thread_pool_impl, RunSimple2)
{
	auto pool = thread_pool<thread_pool_default_impl>::create();

	auto fn = [](int t)->int
	{
		return t;
	};

	auto feature = pool->run<int, int>(fn, 42);

	EXPECT_EQ(feature.get(), 42);
}

TEST(default_thread_pool_impl, RunSimple3)
{
	auto pool = thread_pool<thread_pool_default_impl>::create();

	function<int(int)> fn = [](int t)->int
	{
		return t;
	};

	auto feature = pool->run(fn, 42);

	EXPECT_EQ(feature.get(), 42);
}

TEST(default_thread_pool_impl, RunSimple4)
{
	auto pool = thread_pool<thread_pool_default_impl>::create();

	int j = 100;

	function<int(int)> fn = [j](int t)->int
	{
		return t + j;
	};

	auto feature = pool->run(fn, 42);

	EXPECT_EQ(feature.get(), 142);
}

TEST(default_thread_pool_impl, RunSimpleCallback)
{
	auto pool = thread_pool<thread_pool_default_impl>::create();

	int j = 100;

	function<int(int)> fn = [j](int t)->int
	{
		return t + j;
	};

	int k = 200;

	function<void(shared_future<int>)> callback = [k](shared_future<int> feature)
	{
		EXPECT_EQ(k, 200);
		EXPECT_EQ(feature.get(), 142);
	};

	pool->run(fn, callback, 42);
}