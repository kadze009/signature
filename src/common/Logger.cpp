#include "Logger.hpp"

#include <chrono>
#include <filesystem>
#include <sstream>
#include <thread>

#include <cstdio>
#include <cstdarg>

#include "Config.hpp"
#include "LoggerManager.hpp"



#define ENABLE_COLOR

#ifdef ENABLE_COLOR
#define BASH_COLOR(NUM) << (char)033 << "[1;" << (NUM) << 'm'
#else
#define BASH_COLOR(NUM)
#endif



char const*
toString(Logger::log_level_e v)
{
	using log_level_e = Logger::log_level_e;
	switch(v)
	{
	//using enum Logger::log_level_e;    //NOTE: since C++20
	case log_level_e::DEBUG:   return "DBG";
	case log_level_e::INFO:    return "INF";
	case log_level_e::WARNING: return "WRN";
	case log_level_e::ERROR:   return "ERR";
	}
	return "UNKNOWN";
}



std::atomic_uint32_t Logger::m_counter = ATOMIC_VAR_INIT(0);

Logger::Logger()
	: m_pool(LoggerManager::RefInstance().NewMessagePool())
	, m_producer(LoggerManager::RefInstance().NewLogProducer())
{
	auto count_val = Logger::m_counter.fetch_add(1, std::memory_order_relaxed);
	std::ostringstream ss;
	ss
	   BASH_COLOR((count_val % 7) + 30)
	   << std::hex << std::this_thread::get_id()
	   BASH_COLOR(0);
	m_thread_id = ss.str();
}


Logger::~Logger()
{
	if (auto sp_pool = m_pool.lock())
	{
		sp_pool->MakeFree();
	}
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
	auto sp_pool = m_pool.lock();
	if (not sp_pool) { return; }
	LoggerMessage& msg = sp_pool->allocate();

	auto& buffer = msg.content();
	StringFormer sf{buffer.data(), buffer.size()};

	auto const sec  =   std::chrono::duration_cast<std::chrono::seconds>(duration);
	auto const usec =   std::chrono::duration_cast<std::chrono::microseconds>(duration)
	                  - std::chrono::duration_cast<std::chrono::microseconds>(sec);
	sf.append("%s (%s) %ld.%06ld [%s:%zu] ",
	          toString(lvl), m_thread_id.c_str(),
	          sec.count(), usec.count(),
	          filename, line_n);
	sf.append(fmt, vlist);

	m_producer.push(msg); //NOTE: ignore failure
}

