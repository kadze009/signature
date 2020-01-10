#pragma once

#include <stdexcept>

#include <cstdint>

#include "Singletone.hpp"
#include "LoggerMessage.hpp"
#include "Pool.hpp"
#include "StringFormer.hpp"



#define LOG_E(FMT, ...) \
	Logger::RefInstance().LogErr(__FILE__, __LINE__, FMT, __VA_ARGS__)
#define LOG_W(FMT, ...) \
	Logger::RefInstance().LogWrn(__FILE__, __LINE__, FMT, __VA_ARGS__)
#define LOG_I(FMT, ...) \
	Logger::RefInstance().LogInf(__FILE__, __LINE__, FMT, __VA_ARGS__)
#define LOG_D(FMT, ...) \
	Logger::RefInstance().LogDbg(__FILE__, __LINE__, FMT, __VA_ARGS__)

#define THROW_ERROR(FMT, ...) \
	do { \
		LOG_E("{THROW_ERROR} " FMT, __VA_ARGS__); \
		constexpr std::size_t MAX_SIZE = 256; \
		char buffer[MAX_SIZE]; \
		throw std::runtime_error( \
			StringFormer(buffer, MAX_SIZE)(FMT, __VA_ARGS__) \
			.c_str()); \
	} while(false)



class Logger : public SingletoneThreadLocal<Logger>
{
public:
	enum log_level_e : uint8_t
	{
		DEBUG,
		INFO,
		WARNING,
		ERROR,
	};

	void LogErr(char const* filename, std::size_t line, char const* fmt, ...);
	void LogWrn(char const* filename, std::size_t line, char const* fmt, ...);
	void LogInf(char const* filename, std::size_t line, char const* fmt, ...);
	void LogDbg(char const* filename, std::size_t line, char const* fmt, ...);

private:
	void LogMessage(
		log_level_e    lvl,
		char const*    filename,
		std::size_t    line_n,
		char const*    fmt,
		va_list        vlist);

	static constexpr std::size_t INIT_POOL_SIZE = 50;
	static constexpr std::size_t INC_POOL_SIZE  = 16;
	Pool<LoggerMessage>    m_pool {INIT_POOL_SIZE, INC_POOL_SIZE};
};

char const* toString(Logger::log_level_e);

