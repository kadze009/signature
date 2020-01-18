#include <iostream>

#include "common/Config.hpp"
#include "common/LoggerManager.hpp"
#include "WorkerManager.hpp"


#include <chrono>
#include <thread>
#include "common/Logger.hpp"
#include "common/PoolManager.hpp"


int
main(int argc, char** argv)
{
	std::ios::sync_with_stdio(false);

	auto& config  = Config::RefInstance();
	auto& log_mgr = LoggerManager::RefInstance();

	if (not config.ParseArgs(argc, argv)) { return 1; }
	log_mgr.SetLogfile(config.GetLogfile());
	WorkerManager wrk_mgr(config);

	std::size_t log_batch_size = config.GetBatchSizeOfLogMessages();
	log_mgr.SetSyncMode(false);
	wrk_mgr.Start();
	do
	{
		log_mgr.HandleBatchOfResults(log_batch_size);
		wrk_mgr.DoWork();
	}
	while (not wrk_mgr.WasFinished());
	log_mgr.HandleUnsavedResults();
	wrk_mgr.HandleUnsavedResults();

	//zhaldak debug: start
	std::size_t lm_pools = 0;
	for (auto const& p : PoolManager::RefInstance().RefPools<LoggerMessage>())
	{
		++lm_pools;
		DEBUG("LoggerMessage pool[%zu].size = %zu", lm_pools, p.size());
	}
	DEBUG("LoggerMessage pools = %zu", lm_pools);

	std::size_t wr_pools = 0;
	for (auto const& p : PoolManager::RefInstance().RefPools<WorkerResult>())
	{
		++wr_pools;
		DEBUG("WorkerResult pool[%zu].size = %zu", wr_pools, p.size());
	}
	DEBUG("WorkerResult pools = %zu", wr_pools);
	//zhaldak debug: stop

	return 0;
}

