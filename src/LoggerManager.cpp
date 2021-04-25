#include "LoggerManager.hpp"

#include <array>
#include <functional>

#include <ctime>

#include "Config.hpp"



#ifdef RELEASE_BUILD
#define BUILD_TYPE_MSG "Release"
#else
#define BUILD_TYPE_MSG "Debug"
#endif



LoggerManager::LoggerManager()
	: m_out("stdout", FileWriter::file_type_e::TEXT)
{
	m_out.SetBufferSize(nullptr, 0);
}


LoggerManager::~LoggerManager()
{
	//HandleUnprocessed(); //TODO: fix me
}


void
LoggerManager::AddMessage(LoggerMessage const& msg)
{
	if (not IsSyncMode())
	{
		AddItem(msg);
	}
	else
	{
		std::lock_guard _lg(m_print_mutex);
		HandleItem(msg);
		msg.EndOfHandle();
	}
}


void
LoggerManager::PrintMessage(std::string_view msg)
{
	m_out.Write(msg);
	m_out.Write('\n');
}


void
LoggerManager::NewLogfile(std::string_view filename)
{
	m_out.Reset(filename, FileWriter::file_type_e::TEXT);
	m_out.SetBufferSize(nullptr, 0);

	constexpr std::size_t DATE_TIME_BUF_SIZE = 32;
	std::array<char, DATE_TIME_BUF_SIZE> dt_buffer;
	auto start_time = Config::start_clock_t::to_time_t(
	                      Config::RefInstance().GetStartDateTime());
	std::strftime(dt_buffer.data(), dt_buffer.size(),
	              "%F %T%z", std::localtime(&start_time));

	constexpr std::size_t MSG_SIZE = 256;
	std::array<char, MSG_SIZE> msg;
	auto const& version = Config::GetBuildVersion();
	PrintMessage(StringFormer(msg.data(), msg.size())
		("Application start: %s\n"
		 "Build type:        " BUILD_TYPE_MSG "\n"
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
LoggerManager::HandleItem(LoggerMessage const& msg)
{
	PrintMessage(msg.GetSV());
}

