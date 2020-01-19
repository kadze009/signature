#pragma once

#include <cstdint>

#include <atomic>



template<typename T>
class IThreadProcessor
{
public:
	using item_t = T;

	IThreadProcessor()  = default;
	~IThreadProcessor() = default;
	IThreadProcessor(IThreadProcessor&&)                 = delete;
	IThreadProcessor(IThreadProcessor const&)            = delete;
	IThreadProcessor& operator=(IThreadProcessor&&)      = delete;
	IThreadProcessor& operator=(IThreadProcessor const&) = delete;

	virtual void HandleItem(item_t const&) = 0;

	void AddItem(item_t const& item)
	{
		// The `memory_order`s are taken from
		// [here](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange).
		item_t const* old_last = m_last_node.load();
		while (not m_last_node.compare_exchange_weak(old_last, &item,
		                                             std::memory_order_release,
		                                             std::memory_order_relaxed))
		{}
		if (old_last)
		{
			old_last->SetNext(&item);
		}
		else
		{
			m_head_node = &item;
		}
	}


	void HandleBatchOfItems(std::size_t batch_size)
	{
		if (not m_head_node || batch_size == 0) { return; }

		item_t const* act_last = m_last_node.load();
		item_t const* tmp_node = nullptr;
		std::size_t i = 0;
		while (i != batch_size && m_head_node != act_last)
		{
			HandleItem(*m_head_node);
			tmp_node = m_head_node;
			m_head_node = m_head_node->GetNext();
			tmp_node->EndOfHandle();
			++i;
		}
	}


	// NOTE: The function must be called only in the single thread environment,
	// i.e. when no sub threads.
	void HandleUnprocessed(std::size_t batch_size = 128)
	{
		auto* last_node = m_last_node.load(std::memory_order_relaxed);
		if (not last_node) { return; }
		while (m_head_node != last_node)
		{
			HandleBatchOfItems(batch_size);
		}
		HandleItem(*m_head_node);
		m_head_node->EndOfHandle();
		m_head_node = nullptr;
	}

private:
	item_t const*              m_head_node = nullptr;
	std::atomic<item_t const*> m_last_node = nullptr;
};
