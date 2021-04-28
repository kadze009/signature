#include "MpocQueueProducer.hpp"
#include "MpocQueue.hpp"
#include "MpocQueueItem.hpp"



MpocQueueProducer::MpocQueueProducer(queue_t queue) noexcept
	: m_queue(queue)
	//, m_need_dereg(true)
{
	queue->RegisterProducer(*this);
}


MpocQueueProducer::MpocQueueProducer(MpocQueueProducer&& o) noexcept
	: m_queue(o.m_queue)
	, m_need_dereg(o.m_need_dereg)
{
	o.m_need_dereg = false;
}


MpocQueueProducer& MpocQueueProducer::operator=(MpocQueueProducer&& o) noexcept
{
	if (this != &o)
	{
		m_queue      = o.m_queue;
		m_need_dereg = o.m_need_dereg;
		o.m_need_dereg = false;
	}
	return *this;
}


MpocQueueProducer::~MpocQueueProducer()
{
	if (not m_need_dereg) { return; }
	if (queue_t queue = m_queue.lock())
	{
		queue->DeregisterProducer(*this);
	}
}


bool MpocQueueProducer::push(MpocQueueItem& item) noexcept
{
	if (queue_t queue = m_queue.lock())
	{
		queue->push(item);
		return true;
	}
	return false;
}
