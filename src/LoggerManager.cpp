#include "LoggerManager.hpp"

#include <array>
#include <functional>
#include <utility>

#include <cstdarg>
#include <ctime>

#include "Config.hpp"



#ifdef RELEASE_BUILD
#define BUILD_TYPE_MSG "Release"
#else
#define BUILD_TYPE_MSG "Debug"
#endif



LoggerManager::LoggerManager()
	: m_out("stdout", FileWriter::file_type_e::TEXT)
	, m_queue(MpocQueue::Allocate(DEFAULT_QUEUE_POLLING_MS))
{
	m_out.SetBufferSize(nullptr, 0);
}


LoggerManager::~LoggerManager()
{
	if (m_is_handling)
	{
		m_need_stop_handling = true;
		if (m_msg_handler.joinable()) { m_msg_handler.join(); }
		m_is_handling = false;
	}
	HandleUnprocessed();
	//NotThreadSafe_InternalPrint(
	//	"%s: log_producer_count=%zu", __FUNCTION__, LogProducerCount());
}


void
LoggerManager::PrintMessage(std::string_view msg)
{
	m_out.Write(msg);
	m_out.Write('\n');
}


void
LoggerManager::NotThreadSafe_NewLogfile(std::string_view filename)
{
	m_out.Reset(filename, FileWriter::file_type_e::TEXT);
	m_out.SetBufferSize(nullptr, 0);

	constexpr std::size_t DATE_TIME_BUF_SIZE = 32;
	std::array<char, DATE_TIME_BUF_SIZE> dt_buffer;
	auto start_time = Config::start_clock_t::to_time_t(
	                      Config::RefInstance().GetStartDateTime());
	std::strftime(dt_buffer.data(), dt_buffer.size(),
	              "%F %T%z", std::localtime(&start_time));

	auto const& version = Config::GetBuildVersion();
	NotThreadSafe_InternalPrint(
		"Application start: %s\n"
		"Build type:        %s\n"
		"Build version:     %d.%d (patch %06d)\n\n",
		dt_buffer.data(), BUILD_TYPE_MSG,
		version.major, version.minor, version.patch);
}


void
LoggerManager::NotThreadSafe_SetLogfile(std::string_view filename)
{
	if (filename != m_out.GetName())
	{
		NotThreadSafe_NewLogfile(filename);
	}
}


void
LoggerManager::StartHandleMessagesInSeparateThread()
{
	if (std::exchange(m_is_handling, true))
	{
		LOG_E("%s: execute twice", __FUNCTION__);
		return;
	}
	m_need_stop_handling = false;
	m_msg_handler = std::thread(&LoggerManager::HandleMessages, this);
}


void
LoggerManager::HandleMessages() noexcept
{
	try
	{
		while (not m_need_stop_handling)
		{
			if (LoggerMessage* msg = m_queue->pop_as<LoggerMessage>())
			{
				PrintMessage(msg->sv());
				msg->Release();
			}
		}
	}
	catch (std::exception const& ex)
	{
		LOG_E("%s [FATAL]: unexpected exception: %s",
		      __FUNCTION__, ex.what());
	}
}


void
LoggerManager::NotThreadSafe_InternalPrint(char const* format, ...) noexcept
{
	constexpr std::size_t MSG_SIZE = 256;
	static std::array<char, MSG_SIZE> msg;
	static StringFormer fmt(msg.data(), msg.size());

	fmt.reset();
	va_list args;
	va_start(args, format);
	fmt.append(format, args);
	va_end(args);
	PrintMessage(fmt.sv());
}


void
LoggerManager::HandleUnprocessed() noexcept
{
	if (m_is_handling)
	{
		LOG_E("%s: attemp to call when Handling enable", __FUNCTION__);
		return;
	}

	while (not m_queue->empty())
	{
		LoggerMessage* msg = m_queue->pop_as<LoggerMessage>();
		PrintMessage(msg->sv());
		msg->Release();
	}
}
