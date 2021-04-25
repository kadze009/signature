#pragma once

#include <cstdint>

#include <atomic>



template<typename T>
class IDeferedQueue
{
public:
	using value_type = T;

	IDeferedQueue()  = default;
	virtual ~IDeferedQueue() = default;

	IDeferedQueue(IDeferedQueue&&)                 = delete;
	IDeferedQueue(IDeferedQueue const&)            = delete;
	IDeferedQueue& operator=(IDeferedQueue&&)      = delete;
	IDeferedQueue& operator=(IDeferedQueue const&) = delete;

	virtual void HandleItem(value_type const&) = 0;

	void AddItem(value_type const& item)
	{
		// The `memory_order`s are taken from
		// [here](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange).
		value_type const* old_last = m_last_node.load();
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

		value_type const* act_last = m_last_node.load();
		value_type const* tmp_node = nullptr;
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
	// i.e. when there are no workers threads.
	// NOTE: this function can't be executed in ~IDeferedQueue because the
	// order of execution of this method by IDeferedQueue derived classes is
	// important. For example: LoggerManager has to be destucted the last for
	// managing messages from another IDeferedQueue derived classes.
	//
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
		m_last_node = nullptr;
	}

private:
	value_type const*              m_head_node = nullptr;
	std::atomic<value_type const*> m_last_node = nullptr;
};
