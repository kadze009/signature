#pragma once

#include <memory>



class MpocQueue;
class MpocQueueItem;


class MpocQueueProducer
{
public:
	using queue_t = std::shared_ptr<MpocQueue>;
	explicit MpocQueueProducer(queue_t queue) noexcept;
	MpocQueueProducer(MpocQueueProducer&&) noexcept;
	MpocQueueProducer& operator=(MpocQueueProducer&&) noexcept;
	~MpocQueueProducer();

	bool push(MpocQueueItem& item) noexcept;

private:
	std::weak_ptr<MpocQueue>    m_queue;
	bool                        m_need_dereg {true};
};
