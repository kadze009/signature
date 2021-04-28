#pragma once

#include <forward_list>
#include <algorithm>

#include "Pool.hpp"



template <typename T>
class PoolStorage
{
public:
	using item_t = T;
	using pool_t = Pool<item_t>;
	using pool_list_t = std::forward_list<pool_t>;

	PoolStorage()                         = default;
	~PoolStorage()                        = default;
	PoolStorage(PoolStorage&&)            = delete;
	PoolStorage& operator=(PoolStorage&&) = delete;

	pool_t& Allocate(size_t init_size, size_t inc_val)
	{
		pool_t* pool = FindFreePool();
		if (nullptr == pool)
		{
			m_pools.emplace_front(init_size, inc_val);
			pool = &m_pools.front();
		}
		pool->MakeNonFree();
		return *pool;
	}

private:
	pool_t* FindFreePool() noexcept
	{
		auto it = std::find_if(std::begin(m_pools), std::end(m_pools),
			[](pool_t const& pool){ return pool.IsFree(); });
		return (std::end(m_pools) != it) ? &*it : nullptr;
	}

private:
	pool_list_t    m_pools;
};
