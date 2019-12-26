#include "Logger.hpp"

#include <chrono>
#include <filesystem>

#include <cstdio>
#include <cstdarg>

#include "Config.hpp"
#include "LoggerManager.hpp"
#include "StringFormer.hpp"



// static
char const*
Logger::toString(log_level_e v)
{
	switch(v)
	{
	case log_level_e::DEBUG:   return "DBG";
	case log_level_e::INFO:    return "INF";
	case log_level_e::WARNING: return "WRN";
	case log_level_e::ERROR:   return "ERR";
	}
	return "UNKNOWN";
}


#define LM_BUILDER(NAME, LVL)                                        \
void Logger::NAME(char const* filename, std::size_t line, char const* fmt, ...) \
{                                                                    \
	if (LVL < Config::RefInstance().GetActualLogLevel()) { return; } \
	va_list args;                                                    \
	va_start(args, fmt);                                             \
	LogMessage(LVL, filename, line, fmt, args);                      \
	va_end(args);                                                    \
}
LM_BUILDER(LogErr, log_level_e::ERROR)
LM_BUILDER(LogWrn, log_level_e::WARNING)
LM_BUILDER(LogInf, log_level_e::INFO)
LM_BUILDER(LogDbg, log_level_e::DEBUG)
#undef LM_BUILDER


void
Logger::LogMessage(
	log_level_e    lvl,
	char const*    filename,
	std::size_t    line_n,
	char const*    fmt,
	va_list        vlist)
{
	auto const duration = Config::RefInstance().GetDurationSinceStart();
	LoggerMessage& msg = m_pool.allocate();

	auto& buffer = msg.RefContent();
	StringFormer sf{buffer.data(), buffer.size()};

	auto const sec  =   std::chrono::duration_cast<std::chrono::seconds>(duration);
	auto const usec =   std::chrono::duration_cast<std::chrono::microseconds>(duration)
	                  - std::chrono::duration_cast<std::chrono::microseconds>(sec);
	sf.append("%s %ld.%06ld",  toString(lvl), sec.count(), usec.count());
	if (filename)
	{
		sf.append(" [%s:%zu]", std::filesystem::path(filename).filename().c_str(), line_n);
	}
	sf.append(": ");
	sf.append(fmt, vlist);

	LoggerManager::RefInstance().AddMessage(msg);
}

