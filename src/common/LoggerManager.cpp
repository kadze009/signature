#include "LoggerManager.hpp"

#include <array>

#include <ctime>

#include "Config.hpp"



namespace {
using msg_t = LoggerManager::msg_t;
} // namespace


LoggerManager::LoggerManager()
	: m_out("stdout", FileWriter::file_type_e::TEXT)
{
	m_out.SetBufferSize(0);
}


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
LoggerManager::PrintMessage(std::string_view msg)
{
	m_out.Write(msg);
	m_out.Write('\n');
}


void
LoggerManager::PrintMessage(msg_t& msg)
{
	PrintMessage(msg.GetSV());
}


msg_t*
LoggerManager::MakeFreeAndGetNext(msg_t& msg)
{
	msg_t* next = msg.GetNext();
	msg.MakeFree();
	return next;
}


void
LoggerManager::HandleBatchOfResults(std::size_t batch_size)
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
LoggerManager::NewLogfile(std::string_view filename)
{
	m_out.Reset(filename, FileWriter::file_type_e::TEXT);
	m_out.SetBufferSize(0);

	constexpr std::size_t DATE_TIME_BUF_SIZE = 32;
	std::array<char, DATE_TIME_BUF_SIZE> dt_buffer;
	dt_buffer.fill('\0');
	auto start_time = Config::start_clock_t::to_time_t(
	                      Config::RefInstance().GetStartDateTime());
	std::strftime(dt_buffer.data(), dt_buffer.size(),
	              "%F %T%z", std::localtime(&start_time));

	constexpr std::size_t MSG_SIZE = 256;
	std::array<char, MSG_SIZE> msg;
	auto const& version = Config::GetBuildVersion();
	PrintMessage(StringFormer(msg.data(), msg.size())
		("Application start: %s\n"
		 "Build version:     %d.%d (patch %06d)\n\n",
		 dt_buffer.data(),
		 version.major, version.minor, version.patch)
		.sv());
}


void
LoggerManager::SetLogfile(std::string_view filename)
{
	if (filename != m_out.GetName())
	{
		NewLogfile(filename);
	}
}


void
LoggerManager::HandleUnsavedResults()
{
	static constexpr std::size_t BATCH_SIZE = 128;
	LOG_D("%s: start", __FUNCTION__);
	while (HasUnsaved())
	{
		HandleBatchOfResults(BATCH_SIZE);
	}
	LOG_D("%s: stop", __FUNCTION__);
}
