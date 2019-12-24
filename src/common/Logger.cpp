#include "Logger.hpp"

#include <cstdio>
#include <cstdarg>

#include "Config.hpp"
#include "LoggerManager.hpp"



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
	LoggerMessage& msg = m_pool.allocate();

	auto& buffer = msg.RefContent();
	std::size_t prefix_len = 0;

	if (filename)
	{
		// TODO: generate current date and time with ms
		// TODO: print only filename
		prefix_len = snprintf(buffer.data(), buffer.size(),
		    "%s 2019-12-24 15:15:15.123456 [%s:%zu]: ",
		    toString(lvl), /*time,*/ filename, line_n);
	}
	char* start_print = buffer.data() + prefix_len;
	std::size_t left_size = buffer.size() - prefix_len;
	vsnprintf(start_print, left_size, fmt, vlist);

	LoggerManager::RefInstance().AddMessage(msg);
}

