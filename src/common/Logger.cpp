#include "Logger.hpp"

#include <cstdio>
#include <cstdarg>
#include <cstring>

#include "Config.hpp"
#include "LoggerManager.hpp"
#include "StringFormer.hpp"

#define FILE_PATH_SPLITTER '/'



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
	StringFormer sf{buffer.data(), buffer.size()};

	// TODO: generate current date and time with ms
	sf.append("%s  2019-12-24 15:15:15.123456",  toString(lvl));
	if (filename)
	{
		char const* short_name = std::strrchr(filename, FILE_PATH_SPLITTER);
		sf.append(" [%s:%zu]", short_name, line_n);
	}
	sf.append(": ");
	sf.append(fmt, vlist);

	LoggerManager::RefInstance().AddMessage(msg);
}

