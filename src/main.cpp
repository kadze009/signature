#include <iostream>
#include <thread>
#include <chrono>

#include "Config.hpp"
#include "LoggerManager.hpp"
#include "WorkerManager.hpp"



// See common/Logger.hpp
#ifdef ENABLE_DEBUG
#include "common/PoolManager.hpp"
template<typename T>
void pool_stats(char const* name)
{
	DEBUG("=== Stats for %s ===", name);
	auto& pools = PoolManager::GetSpInstance()->RefPools<T>();
	std::size_t pool_counter = 0;
	for (auto& pool : pools)
	{
		++pool_counter;
		DEBUG("> #%02zu: size=%zu", pool_counter, pool.size());
	}
	DEBUG("===( count = %zu )===", pool_counter);
}
#define POOL_STATS(type) pool_stats<type>(#type)
#else
#define POOL_STATS(type) (void)0
#endif



int
main(int argc, char** argv)
{
	constexpr std::chrono::milliseconds POLLING_DELAY {100};
	std::ios::sync_with_stdio(false);
	auto& log_mgr = LoggerManager::RefInstance();

	auto& config = Config::RefInstance();
	if (not config.ParseArgs(argc, argv)) { return 1; }

	//NOTE: this is a workaround for changing logfile because the current
	// implementation is not thread safe
	log_mgr.HandleUnprocessed();
	log_mgr.NotThreadSafe_SetLogfile(config.GetLogfile());
	log_mgr.StartHandleMessagesInSeparateThread();

	WorkerManager wrk_mgr(config);
	if (not wrk_mgr.Start()) { return 2; }

	do
	{
		wrk_mgr.DoWork();
		std::this_thread::sleep_for(POLLING_DELAY);
	}
	while (not wrk_mgr.WasFinished());
	wrk_mgr.HandleUnprocessed();

	POOL_STATS(LoggerMessage);
	POOL_STATS(WorkerResult);
	return 0;
}

