#include "WorkerManager.hpp"

#include <thread>
#include <chrono>
#include <algorithm>

#include "common/Logger.hpp"



namespace {

using result_t = WorkerManager::result_t;

} // namespace


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
		m_workers.emplace_back(this, block_num);
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
		HandleBatchOfResults(m_resultsBatchSize);
	} //if (not IsAborting())
	else
	{
		if (m_wasFinished) { m_isAborting = false; }
	}
}



void
WorkerManager::AddResult(result_t& result)
{
	// The `memory_order`s are taken from
	// [here](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange).
	result_t* old_last = m_last_res.load();
	while (not m_last_res.compare_exchange_weak(old_last, &result,
	                                            std::memory_order_release,
	                                            std::memory_order_relaxed))
	{}

	if (old_last)
	{
		old_last->SetNext(&result);
	}
	else
	{
		m_head_res = &result;
	}
}



result_t*
WorkerManager::MakeFreeAndGetNext(result_t& res)
{
	result_t* next = res.GetNext();
	res.MakeFree();
	return next;
}



void
WorkerManager::HandleBatchOfResults(std::size_t batch_size)
{
	if (not m_head_res || batch_size == 0) { return; }

	//LOG_D("%s: batch_size = %zu", __FUNCTION__, batch_size);
	result_t* act_last = m_last_res.load();
	std::size_t i = 0;
	while (i != batch_size && m_head_res != act_last)
	{
		SaveResult(*m_head_res);
		m_head_res = MakeFreeAndGetNext(*m_head_res);
		++i;
	}
}



void
WorkerManager::SaveResult(result_t& res)
{
	auto const  bnum = res.GetBlockNum();
	auto const& hash = res.GetHash();
	m_out.Write(&bnum, sizeof(bnum), 1);
	m_out.Write(hash.data(), hash.size());
}



// NOTE: The function must be called only in the single thread environment,
// i.e. when no sub threads.
void
WorkerManager::HandleUnsavedResults()
{
	LOG_D("%s: start", __FUNCTION__);
	auto* last_one = m_last_res.load(std::memory_order_relaxed);
	if (not last_one) { return; }
	while (m_head_res != last_one)
	{
		HandleBatchOfResults(m_resultsBatchSize);
	}
	SaveResult(*m_head_res);
	m_head_res = MakeFreeAndGetNext(*m_head_res);
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

