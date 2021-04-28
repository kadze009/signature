#include "MpocQueue.hpp"
#include "MpocQueueItem.hpp"
#include "MpocQueueProducer.hpp"



MpocQueue::producer_t MpocQueue::NewProducer() noexcept
{
	return producer_t{shared_from_this()};
}


MpocQueue::item_t const* MpocQueue::front(uint32_t timeout_ms /*= USE_DEFAULT_TIMEOUT*/) const noexcept
{
	constexpr size_t ATTEMPTS_WO_LOCK = 8; //NOTE: magic number
	size_t attempt = ATTEMPTS_WO_LOCK;
	while (--attempt)
	{
		//TODO: there is an extra checking if `0 == m_producer_count`
		item_t const* head = m_head.load();
		if (head) { return head; }
	}
	if (USE_DEFAULT_TIMEOUT == timeout_ms) { timeout_ms = m_default_wait_ms; }

	std::unique_lock lock{m_lock};
	if (0 == m_producer_count) { return nullptr; }
	m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),
		[this]{ return m_head.load() or (0 == m_producer_count); });
	return m_head.load();
}


MpocQueue::item_t* MpocQueue::pop(uint32_t timeout_ms /*= USE_DEFAULT_TIMEOUT*/)
{
	item_t* act_head = front(timeout_ms);
	if (not act_head) { return nullptr; }

	item_t* act_tail = m_tail.load();
	if (act_head != act_tail)
	{
		m_head.store(act_head->next());
	}
	else
	{
		// The head and the tail are equal. Try forgetting the tail.
		if (m_tail.compare_exchange_strong(act_tail, nullptr))
		{
			// The tail forgetting is done.
			// Try foggeting the head. If no it means that someone
			// already set new head. It's OK.
			item_t* exp_head = act_head;
			m_head.compare_exchange_strong(exp_head, nullptr);
		}
		else
		{
			// The tail forgetting is fail. It means that the tail was changed
			// and someone add a new item to the head.
			m_head.store(act_head->next());
		}
	}
	act_head->next(nullptr);
	return act_head;
}


void MpocQueue::push(MpocQueue::item_t& item) noexcept
{
	// The `memory_order`s were taken from
	// [here](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange).
	item_t* old_tail = m_tail.load();
	while (not m_tail.compare_exchange_weak(old_tail, &item,
	                  std::memory_order_release, std::memory_order_relaxed))
	{}

	if (old_tail)
	{
		old_tail->next(&item);
	}
	else
	{
		// Expect that the m_head is nullptr
		m_head.store(&item);
		m_cv.notify_one();
	}
}


void MpocQueue::RegisterProducer(MpocQueue::producer_t const&) noexcept
{
	std::lock_guard lock{m_lock};
	++m_producer_count;
}


void MpocQueue::DeregisterProducer(MpocQueue::producer_t const&) noexcept
{
	std::unique_lock lock{m_lock};
	--m_producer_count;
	lock.unlock(); //NOTE: only this line isn't noexcept
	if (0 == m_producer_count) { m_cv.notify_one(); }
}
