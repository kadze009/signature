#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <type_traits>

#include <cstdint>



class MpocQueueItem;
class MpocQueueProducer;


class MpocQueue : public std::enable_shared_from_this<MpocQueue>
{
public:
	using item_t     = MpocQueueItem;
	using producer_t = MpocQueueProducer;

	static constexpr uint32_t DEFAULT_TIMEOUT_MS  = 500;
	static constexpr uint32_t USE_DEFAULT_TIMEOUT = 0;

	static auto Allocate(uint32_t def_timeout_ms = DEFAULT_TIMEOUT_MS)
	{
		return std::shared_ptr<MpocQueue>(new MpocQueue{def_timeout_ms});
	}

	~MpocQueue()                           = default;
	MpocQueue(MpocQueue const&)            = delete;
	MpocQueue& operator=(MpocQueue const&) = delete;


	producer_t NewProducer() noexcept;
	size_t ProducerCount() const noexcept { return m_producer_count; }

	item_t const* front(uint32_t timeout_ms = USE_DEFAULT_TIMEOUT) const noexcept;
	item_t* front(uint32_t timeout_ms = USE_DEFAULT_TIMEOUT) noexcept
	{
		return const_cast<item_t*>(
			const_cast<MpocQueue const*>(this)->front(timeout_ms)
			);
	}

	item_t* pop(uint32_t timeout_ms = USE_DEFAULT_TIMEOUT);

	template <typename T>
	T* pop_as(uint32_t timeout_ms = USE_DEFAULT_TIMEOUT)
	{
		static_assert(!std::is_pointer_v<T>,        "T mustn't be a pointer");
		static_assert(std::is_base_of_v<item_t, T>, "T must base of MpocQueue::item_t");
		return static_cast<T*>(pop(timeout_ms));
	}

	bool empty() const noexcept { return nullptr == m_head.load(); }


private:
	explicit MpocQueue(uint32_t def_timeout_ms = DEFAULT_TIMEOUT_MS) noexcept
		: m_default_wait_ms(def_timeout_ms)
	{}

	void push(item_t&) noexcept;
	void RegisterProducer(producer_t const&) noexcept;
	void DeregisterProducer(producer_t const&) noexcept;

private:
	friend class MpocQueueProducer;

private:
	size_t                  m_producer_count {0};
	std::atomic<item_t*>    m_head {nullptr};
	std::atomic<item_t*>    m_tail {nullptr};
	uint32_t const          m_default_wait_ms;

	//NOTE: `mutable` for `front() const`
	mutable std::mutex                 m_lock;
	mutable std::condition_variable    m_cv;
};
