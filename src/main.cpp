#include <iostream>
#include <thread>
#include <chrono>

#include "Config.hpp"
#include "LoggerManager.hpp"
#include "WorkerManager.hpp"



namespace
{
enum exit_codes_e : int
{
	SUCCESS = 0,
	CONFIG_PARSE_ERROR,
	WORKERS_START_ERROR,
	WORKERS_RUNTIME_ERROR,
};
} // namespace


int
main(int argc, char** argv)
{
	constexpr std::chrono::milliseconds POLLING_DELAY {100};
	std::ios::sync_with_stdio(false);
	auto& log_mgr = LoggerManager::RefInstance();

	auto& config = Config::RefInstance();
	if (not config.ParseArgs(argc, argv)) { return exit_codes_e::CONFIG_PARSE_ERROR; }

	//NOTE: this is a workaround for changing logfile because the current
	// implementation is not thread safe
	log_mgr.HandleUnprocessed();
	log_mgr.NotThreadSafe_SetLogfile(config.GetLogfile());
	log_mgr.StartHandleMessagesInSeparateThread();

	WorkerManager wrk_mgr(config);
	if (not wrk_mgr.Start()) { return exit_codes_e::WORKERS_START_ERROR; }
	return (wrk_mgr.DoWork())
		? exit_codes_e::SUCCESS
		: exit_codes_e::WORKERS_RUNTIME_ERROR;
}

