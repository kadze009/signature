#include "LoggerManager.hpp"



namespace {
using msg_t = LoggerManager::msg_t;
} // namespace



void
LoggerManager::AddMessage(msg_t& msg)
{
	if (not IsSyncMode())
	{
		PushMessageBack(msg);
	}
	else
	{
		std::lock_guard<print_mutex_t> _lg(m_print_mutex);
		PrintMessage(msg);
		msg.MakeFree();
	}
}


void
LoggerManager::PushMessageBack(msg_t& msg)
{
	// The `memory_order`s are taken from
	// [here](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange).
	msg_t* old_last = m_last_msg.load(std::memory_order_relaxed);
	while (not m_last_msg.compare_exchange_weak(old_last, &msg,
	                                            std::memory_order_release,
	                                            std::memory_order_relaxed))
	{}

	if (old_last)
	{
		old_last->SetNext(&msg);
	}
	else
	{
		m_head_msg = &msg;
	}
}


void
LoggerManager::PrintMessage(msg_t& msg)
{
	*m_out << msg.GetSV() << '\n';
	if (IsFlushing()) { m_out->flush(); }
}


msg_t*
LoggerManager::MakeFreeAndGetNext(msg_t& msg)
{
	msg_t* next = msg.GetNext();
	msg.MakeFree();
	return next;
}


void
LoggerManager::PrintBatchOfMessages(std::size_t batch_size)
{
	if (not m_head_msg || batch_size == 0) { return; }

	msg_t* act_last = m_last_msg.load(std::memory_order_relaxed);
	std::size_t i = 0;

	do
	{
		while (m_head_msg != act_last)
		{
			PrintMessage(*m_head_msg);
			m_head_msg = MakeFreeAndGetNext(*m_head_msg);
			++i;
			if (i == batch_size) { return; }
		}
	}
	while(not m_last_msg.compare_exchange_weak(act_last, nullptr,
	                                           std::memory_order_release,
	                                           std::memory_order_relaxed));

	PrintMessage(*act_last);
	// Expected that the last node of list has `nullptr` as a result of `GetNext`.
	m_head_msg = MakeFreeAndGetNext(*act_last);
}


void
LoggerManager::SetLogfile(std::string_view filename)
{

}

