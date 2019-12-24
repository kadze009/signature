#pragma once

#include <deque>
#include <type_traits>
#include <algorithm>

#include <cstdint>



class ItemPool
{
public:
	void Allocate()       { m_is_free = false; }
	void Release() const  { m_is_free = true; }
	bool IsFree() const   { return m_is_free; }

private:
	mutable bool m_is_free = true;
};



template<typename T>
class Pool
{
public:
	static_assert(std::is_base_of_v<ItemPool, T>, 
	              "The items of a pool must be derived from the ItemPool class.");

	Pool(std::size_t init_size, std::size_t inc_pool_size)
		: m_inc_size(inc_pool_size)
	{
		m_pool.resize(init_size);
	}

	Pool(Pool&&)                 = delete;
	Pool(Pool const&)            = delete;
	Pool& operator=(Pool&&)      = delete;
	Pool& operator=(Pool const&) = delete;
	~Pool()                      = default;

	T& allocate()
	{
		T* item = GetFree();
		if (not item)
		{
			std::size_t last_item = m_pool.size();
			m_pool.resize(last_item + m_inc_size);
			item = &m_pool[last_item];
		}
		static_cast<ItemPool*>(item)->Allocate();
		return *item;
	}

	std::size_t size() const { return m_pool.size(); }

private:
	T* GetFree()
	{
		auto it = std::find_if(m_pool.begin(), m_pool.end(),
		                       [](T const& item) { return item.IsFree(); });
		return it == m_pool.end() ? nullptr : &*it;
	}

	std::size_t   m_inc_size;
	std::deque<T> m_pool;
};

