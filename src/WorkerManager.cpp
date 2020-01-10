#include "WorkerManager.hpp"

#include <array>
#include <filesystem>
#include <system_error>

#include <ctime>

#include "common/Logger.hpp"
#include "common/LoggerManager.hpp"
#include "common/Config.hpp"


void
WorkerManager::Init() noexcept
{
	auto& log_mgr = LoggerManager::RefInstance();
	auto& config  = Config::RefInstance();

	log_mgr.SetLogfile(config.GetLogfile());
	constexpr std::size_t DATE_TIME_BUF_SIZE = 32;
	std::array<char, DATE_TIME_BUF_SIZE> dt_buffer;
	dt_buffer.fill('\0');
	auto start_time = Config::start_clock_t::to_time_t(config.GetStartDateTime());
	std::strftime(dt_buffer.data(), dt_buffer.size(),
	              "%F %T%z", std::localtime(&start_time));
	LOG_I("Application start: %s", dt_buffer.data());

	std::size_t threads_num = config.GetThreadsNum();
	if (not config.NeedUseMainThread())
	{
		config.SetThreadsNum(--threads_num);
	}
	config.SetBytesShift(threads_num * config.GetBlockSizeKB() * 1024);

	LOG_I("%s: final configuration:\n%s", __FUNCTION__, config.toString());
}


void
WorkerManager::Run() noexcept
{
	//TODO: implement

	auto& log_mgr = LoggerManager::RefInstance();
	auto& config  = Config::RefInstance();

	log_mgr.SetSyncMode(false);
	LOG_E("%s: change log mode", __FUNCTION__);

	// Start Workers

	// Repeat while exists at least one of Worker
	log_mgr.PrintBatchOfMessages(config.GetBatchSizeOfLogMessages());
	// 1. Check logs
	// 2. Check result for writing
	// 3. Check states of threads
}

