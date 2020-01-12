#include "WorkerManager.hpp"

#include "common/Logger.hpp"



WorkerManager::WorkerManager(Config& config)
	: m_cfg(config)
{
	std::size_t threads_num = m_cfg.GetThreadsNum();
	if (not m_cfg.NeedUseMainThread())
	{
		m_cfg.SetThreadsNum(--threads_num);
	}
	m_cfg.SetBytesShift(threads_num * m_cfg.GetBlockSizeKB() * 1024);

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

