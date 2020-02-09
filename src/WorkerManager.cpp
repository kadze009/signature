#include "WorkerManager.hpp"

#include <thread>
#include <chrono>
#include <algorithm>
#include <istream>

#include "common/Logger.hpp"



WorkerManager::WorkerManager(Config& config)
	: m_cfg(config)
	, m_out(m_cfg.GetOutputFile(), FileWriter::file_type_e::TEXT)
{
	std::uint64_t const block_size = m_cfg.GetBlockSizeKB() * 1024;
	std::uint64_t blocks_count = m_cfg.GetInputFileSize() / block_size;
	if (m_cfg.GetInputFileSize() % block_size != 0)
	{
		++blocks_count;
	}
	// -1 because the main thread does not calculate hash
	std::uint64_t worker_num = std::min(m_cfg.GetThreadsNum() - 1, blocks_count);

	m_workers.reserve(worker_num);
	for (std::uint64_t block_num = 0; block_num < worker_num; ++block_num)
	{
		m_workers.emplace_back(*this, block_num);
	}
	LOG_I("%s: create %zu Workers and will be processed %zu blocks",
	      __FUNCTION__, m_workers.size(), blocks_count);

	// -1 because block counter start from 0
	m_cfg.SetLastBlockNum(blocks_count - 1);
	m_cfg.SetBlocksShift(m_workers.size());

	// -1 because one block has read this thread and next blocks should skip
	m_cfg.SetFileBytesShift(block_size * (m_workers.size() - 1));

	LOG_I("%s: final configuration:\n%s", __FUNCTION__, m_cfg.toString());
}



void
WorkerManager::Start() noexcept
{
	m_wasFinished = false;

	LOG_I("%s: start %zu Workers", __FUNCTION__, m_workers.size());
	for (Worker& w : m_workers) { w.RunAsync(); }
	LOG_D("%s: all Workers were started", __FUNCTION__);
}



void
WorkerManager::DoWork() noexcept
{
	if (m_wasFinished) { return; }

	m_wasFinished = AreAllWorkersStop();

	if (not IsAborting())
	{
		if (not m_wasFinished)
		{
			if (Worker* failed_worker = FindFailedWorker(); failed_worker)
			{
				try { failed_worker->ThrowError(); }
				catch (std::exception const& ex)
				{
					LOG_E("%s: detect faild Worker with BLOCK #%zu: %s",
						  __FUNCTION__,
						  failed_worker->GetBlockNum(),
						  ex.what());
				}
				StartAborting();
				return;
			}
		}
		else
		{
			LOG_I("%s: all Workers were finish", __FUNCTION__);
		}
		HandleBatchOfItems(m_resultsBatchSize);
	} //if (not IsAborting())
	else
	{
		if (m_wasFinished) { m_isAborting = false; }
	}
}



void
WorkerManager::HandleItem(WorkerResult const& res)
{
	auto const  bnum = res.GetBlockNum();
	auto const& hash = res.GetHash();
	m_out.Write(&bnum, sizeof(bnum), 1);
	m_out.Write(hash.data(), hash.size());
}



void
WorkerManager::StartAborting() noexcept
{
	m_isAborting = true;
	LOG_W("%s: start aborting", __FUNCTION__);
	for (Worker& w : m_workers)
	{
		if (w.IsRunning()) { w.SetStop(); }
	}
}



bool
WorkerManager::AreAllWorkersStop() const
{
	auto it = std::find_if(m_workers.begin(), m_workers.end(),
		[](Worker const& w) { return w.IsRunning(); });
	return it == m_workers.end();
}



Worker*
WorkerManager::FindFailedWorker()
{
	auto it = std::find_if(m_workers.begin(), m_workers.end(),
		[](Worker const& w) { return not w.IsRunning() and w.HasError(); });
	return it != m_workers.end() ? &*it : nullptr;
}

