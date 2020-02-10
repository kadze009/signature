#include <iostream>
#include <thread>
#include <chrono>

#include "Config.hpp"
#include "LoggerManager.hpp"
#include "WorkerManager.hpp"



#ifdef ENABLE_DEBUG
#include "PoolManager.hpp"
template<typename T>
void pool_stats(char const* name)
{
	DEBUG("=== Stats for %s ===", name);
	auto& pools = PoolManager::RefInstance().RefPools<T>();
	std::size_t pool_counter = 0;
	for (auto& pool : pools)
	{
		++pool_counter;
		DEBUG("> #%02zu: size=%zu", pool_counter, pool.size());
	}
	DEBUG("===( count = %zu )===", pool_counter);
}
#endif



int
main(int argc, char** argv)
{
	constexpr std::chrono::milliseconds POLLING_DELAY {100};
	std::ios::sync_with_stdio(false);

	auto& config  = Config::RefInstance();
	auto& log_mgr = LoggerManager::RefInstance();

	if (not config.ParseArgs(argc, argv)) { return 1; }
	log_mgr.SetLogfile(config.GetLogfile().c_str());
	WorkerManager wrk_mgr(config);

	std::size_t log_batch_size = config.GetBatchSizeOfLogMessages();
	log_mgr.SetSyncMode(false);
	if (not wrk_mgr.Start()) { return 2; }

	do
	{
		log_mgr.HandleBatchOfItems(log_batch_size);
		wrk_mgr.DoWork();
		std::this_thread::sleep_for(POLLING_DELAY);
	}
	while (not wrk_mgr.WasFinished());
	wrk_mgr.HandleUnprocessed();
	log_mgr.HandleUnprocessed();

#ifdef ENABLE_DEBUG
	pool_stats<LoggerMessage>("LoggerMessage");
	pool_stats<WorkerResult>("WorkerResult");
#endif
	return 0;
}

