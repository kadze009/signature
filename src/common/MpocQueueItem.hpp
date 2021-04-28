#pragma once



class MpocQueue;

class MpocQueueItem
{
private:
	friend class MpocQueue;

	void next(MpocQueueItem* item) noexcept { m_next = item; }
	MpocQueueItem* next() noexcept          { return m_next; }

private:
	MpocQueueItem*   m_next {nullptr};
};
