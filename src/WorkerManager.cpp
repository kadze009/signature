#include "WorkerManager.hpp"

#include "common/Logger.hpp"



WorkerManager::WorkerManager(Config& config)
	: m_cfg(config)
{
	// -1 because the main thread does not calculate hash
	m_cfg.SetBlockNumShift(m_cfg.GetThreadsNum() - 1);

	LOG_I("%s: final configuration:\n%s", __FUNCTION__, m_cfg.toString());
}



void
WorkerManager::Start() noexcept
{
	m_isFinished = false;

	//TODO: implement
}



void
WorkerManager::DoWork() noexcept
{
	if (IsFinished()) { return; }

	//TODO: implement
	m_isFinished = true;
}

