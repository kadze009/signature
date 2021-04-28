#pragma once

#include <deque>
#include <type_traits>
#include <algorithm>

#include <cstdint>



class PoolItem
{
public:
	void Allocate() noexcept      { m_is_free = false; }
	void Release() const noexcept { m_is_free = true; }
	bool IsFree() const noexcept  { return m_is_free; }

private:
	mutable bool m_is_free = true;
};



template<typename T>
class Pool
{
public:
	static_assert(std::is_base_of_v<PoolItem, T>,
	              "The items of a pool must be derived from the PoolItem class.");

	Pool(Pool const&)            = delete;
	Pool& operator=(Pool const&) = delete;

	Pool(std::size_t init_size, std::size_t inc_pool_size)
		: m_inc_size(inc_pool_size)
	{
		m_pool.resize(init_size);
	}
	Pool(Pool&&)             = default;
	Pool& operator= (Pool&&) = default;
	~Pool()                  = default;

	T& allocate()
	{
		T* item = GetFree();
		if (not item)
		{
			std::size_t last_item = m_pool.size();
			m_pool.resize(last_item + m_inc_size);
			item = &m_pool[last_item];
		}
		static_cast<PoolItem*>(item)->Allocate();
		return *item;
	}

	std::size_t size() const { return m_pool.size(); }

	void MakeNonFree()       { m_isFree = false; }
	void MakeFree()          { m_isFree = true; }
	bool IsFree() const      { return m_isFree; }

private:
	T* GetFree()
	{
		auto it = std::find_if(m_pool.begin(), m_pool.end(),
		                       [](T const& item) { return item.IsFree(); });
		return it == m_pool.end() ? nullptr : &*it;
	}

private:
	std::size_t   m_inc_size;
	std::deque<T> m_pool;
	bool          m_isFree   = true;
};

