#include <iostream>

#include "common/Config.hpp"
#include "common/Logger.hpp"
#include "common/LoggerManager.hpp"



int main(int argc, char** argv)
{
	std::ios::sync_with_stdio(false);
	//if (not Config::RefInstance().ParseArgs(argc, argv)) { return 1; }

	LoggerManager::RefInstance().SetSyncMode(false);
	LoggerManager::RefInstance().SetFlushing(true);

	LOG_E("Hello [%s]", "1");
	LOG_W("Hello [%s]", "2");
	LOG_I("Hello [%s]", "3");
	LOG_D("Hello [%s]", "4");
	//TODO: LOG_E("From RUSSIA\nEEEE");
	LoggerManager::RefInstance().PrintBatchOfMessages(2);
	LoggerManager::RefInstance().PrintBatchOfMessages(2);
	LoggerManager::RefInstance().PrintBatchOfMessages(2);

	return 0;
}

