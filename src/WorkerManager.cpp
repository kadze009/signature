#include "WorkerManager.hpp"

#include "common/Logger.hpp"



WorkerManager::WorkerManager(Config& config)
	: m_cfg(config)
{
	m_cfg.SetBytesShift(
		m_cfg.GetThreadsNum() * m_cfg.GetBlockSizeKB() * 1024);

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

