#include <iostream>

#include "common/Config.hpp"
#include "WorkerManager.hpp"



int
main(int argc, char** argv)
{
	std::ios::sync_with_stdio(false);

	auto& config  = Config::RefInstance();
	auto& wrk_mgr = WorkerManager::RefInstance();

	if (not config.ParseArgs(argc, argv)) { return 1; }
	wrk_mgr.Init();
	wrk_mgr.Run();

	return 0;
}

