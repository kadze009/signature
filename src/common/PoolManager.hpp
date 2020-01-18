#pragma once

#include <forward_list>
#include <mutex>

#include "Singletone.hpp"
#include "Pool.hpp"



class PoolManager: public Singletone<PoolManager>
{
public:
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

};

