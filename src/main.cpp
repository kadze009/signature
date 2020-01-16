#include <iostream>

#include "common/Config.hpp"
#include "common/LoggerManager.hpp"
#include "WorkerManager.hpp"



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

	return 0;
}

