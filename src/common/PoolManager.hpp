#pragma once

#include <forward_list>
#include <mutex>

#include "common/SharedPtrSingleton.hpp"
#include "common/Pool.hpp"



class PoolManager: public SharedPtrSingleton<PoolManager>
{
public:
	~PoolManager() = default;

	template<typename ItemT>
	Pool<ItemT>& NewPool(std::size_t init_size, std::size_t inc_size)
	{
		using mutex_t = std::mutex;
		static mutex_t mutex;

		std::lock_guard<mutex_t> lg(mutex);

		auto& pools = RefPools<ItemT>();
		pools.emplace_front(init_size, inc_size);
		auto& pool = pools.front();
		pool.MakeNonFree();
		return pool;
	}

	template<typename ItemT>
	auto& RefPools()
	{
		static std::forward_list<Pool<ItemT>> pools_list;
		return pools_list;
	}

private:
	friend class SharedPtrSingleton<PoolManager>;

	PoolManager() = default;
};

