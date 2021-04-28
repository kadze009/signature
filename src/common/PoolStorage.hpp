#pragma once

#include <algorithm>
#include <forward_list>
#include <memory>

#include "Pool.hpp"



template <typename T>
class PoolStorage
{
public:
	using item_t      = T;
	using pool_t      = Pool<item_t>;
	using sp_pool_t   = std::shared_ptr<pool_t>;
	using wp_pool_t   = std::weak_ptr<pool_t>;
	using pool_list_t = std::forward_list<sp_pool_t>;

	PoolStorage()                         = default;
	~PoolStorage()                        = default;
	PoolStorage(PoolStorage&&)            = delete;
	PoolStorage& operator=(PoolStorage&&) = delete;

	wp_pool_t Allocate(size_t init_size, size_t inc_val)
	{
		sp_pool_t pool = FindFreePool();
		if (not pool)
		{
			m_pools.emplace_front(std::make_shared<pool_t>(init_size, inc_val));
			pool = m_pools.front();
		}
		pool->MakeNonFree();
		return wp_pool_t{pool};
	}

private:
	sp_pool_t FindFreePool() noexcept
	{
		auto it = std::find_if(std::begin(m_pools), std::end(m_pools),
			[](sp_pool_t const& pool){ return pool->IsFree(); });
		return (std::end(m_pools) != it) ? *it : sp_pool_t{};
	}

private:
	pool_list_t    m_pools;
};
