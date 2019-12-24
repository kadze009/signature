#pragma once


template <typename T>
struct Singletone
{
	Singletone()                             = default;
	Singletone(Singletone&&)                 = delete;
	Singletone(Singletone const&)            = delete;
	Singletone operator= (Singletone&&)      = delete;
	Singletone operator= (Singletone const&) = delete;

	static T&       RefInstance() { static T instance; return instance; }
	static T const& GetInstance() { return RefInstance(); }
};




template <typename T>
struct SingletoneThreadLocal
{
	SingletoneThreadLocal()                                        = default;
	SingletoneThreadLocal(SingletoneThreadLocal&&)                 = delete;
	SingletoneThreadLocal(SingletoneThreadLocal const&)            = delete;
	SingletoneThreadLocal operator= (SingletoneThreadLocal&&)      = delete;
	SingletoneThreadLocal operator= (SingletoneThreadLocal const&) = delete;

	static T& RefInstance()
	{
		thread_local static T instance;
		return instance;
	}

	static T const& GetInstance() { return RefInstance(); }
};

