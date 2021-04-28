#include "WorkerManager.hpp"

#include <thread>
#include <chrono>
#include <algorithm>
#include <istream>

#include "common/Logger.hpp"



WorkerManager::WorkerManager(Config& config)
	: m_cfg(config)
	, m_out(m_cfg.GetOutputFile().c_str(), FileWriter::file_type_e::TEXT)
	, m_results(MpocQueue::Allocate(DEFAULT_QUEUE_POLLING_MS))
{
	uint64_t const block_size = m_cfg.GetBlockSizeKB() * 1024;
	uint64_t blocks_count = m_cfg.GetInputFileSize() / block_size;
	if (m_cfg.GetInputFileSize() % block_size != 0)
	{
		++blocks_count;
	}

	auto const worker_num = [&blocks_count](size_t const threads_num) -> uint64_t
	{
		//NOTE: `0 == threads_num` is the paranoia case because Config class
		// will check the value of the `threads_num`.
		if (1 == threads_num || 0 == threads_num) { return 1; }
		//NOTE: -1 because the main thread does not calculate hash
		return std::min(threads_num - 1, blocks_count);
	}(m_cfg.GetThreadsNum());

	m_workers.reserve(worker_num);
	for (uint64_t block_num = 0; block_num < worker_num; ++block_num)
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



WorkerManager::~WorkerManager()
{
	StopAllWorkers();
	HandleUnprocessed();
	LOG_D("%s: active workers = %zu", __FUNCTION__, m_results->ProducerCount());
}



bool
WorkerManager::Start() noexcept
{
	m_wasFinished = false;

	LOG_I("%s: start %zu Workers", __FUNCTION__, m_workers.size());
	for (Worker& w : m_workers)
	{
		if (not w.RunAsync())
		{
			LOG_E("%s: worker (block=%zu) didn't start. Start aborting...",
			      __FUNCTION__, w.GetBlockNum());
			StopAllWorkers();
			return false;
		}
	}
	LOG_D("%s: all Workers were started", __FUNCTION__);
	return true;
}



bool
WorkerManager::DoWork() noexcept
{
	bool was_error = false;
	uint64_t failed_worker_block_num = 0;
	std::string failed_worker_err_msg;

	//TODO: split the loop on two threads
	while (not m_wasFinished)
	{
		//Responcibility: save the workers' results
		if (WorkerResult* res = m_results->pop_as<WorkerResult>())
		{
			SaveResult(*res);
			res->Release();
		}

		//Responcibility: check health of the workres
		m_wasFinished = AreAllWorkersStop();
		if (not m_isAborting)
		{
			if (not m_wasFinished)
			{
				if (Worker* failed_worker = FindFailedWorker(); failed_worker)
				{
					try { failed_worker->ThrowError(); }
					catch (std::exception const& ex)
					{
						was_error = true;
						failed_worker_block_num = failed_worker->GetBlockNum();
						failed_worker_err_msg.assign(ex.what());
						LOG_E("%s: the Worker with BLOCK #%zu failed: %s",
							__FUNCTION__, failed_worker_block_num,
							failed_worker_err_msg.c_str());
					}
					StartAborting();
					continue;
				}
			}
			else
			{
				LOG_I("%s: all Workers were finished", __FUNCTION__);
			}
		} //if (not m_isAborting)
		else
		{
			if (m_wasFinished) { m_isAborting = false; }
		}
	}

	if(was_error)
	{
		LOG_E("%s: there was initial error: "
			"the Worker with BLOCK #%zu failed: %s",
			__FUNCTION__,
			failed_worker_block_num, failed_worker_err_msg.c_str());
	}
	return not was_error;
}


void
WorkerManager::SaveResult(WorkerResult const& res)
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
	LOG_D("%s: start aborting", __FUNCTION__);
	for (Worker& w : m_workers)
	{
		if (w.IsRunning()) { w.SetStop(); }
	}
}



bool
WorkerManager::AreAllWorkersStop() const noexcept
{
	return std::none_of(std::begin(m_workers), std::end(m_workers),
		[](Worker const& w) { return w.IsRunning(); });
}



Worker*
WorkerManager::FindFailedWorker() noexcept
{
	auto it = std::find_if(std::begin(m_workers), std::end(m_workers),
		[](Worker const& w) { return not w.IsRunning() and w.HasError(); });
	return (it != std::end(m_workers)) ? &*it : nullptr;
}


void
WorkerManager::StopAllWorkers() noexcept
{
	if (WasFinished())
	{
		LOG_D("%s: all workers are already stopped", __FUNCTION__);
		return;
	}
	LOG_D("%s: starts", __FUNCTION__);
	StartAborting();
	constexpr std::chrono::milliseconds POLLING_DELAY {50};
	do
	{
		std::this_thread::sleep_for(POLLING_DELAY);
		m_wasFinished = AreAllWorkersStop();
	}
	while (not m_wasFinished);
	LOG_D("%s: finishs", __FUNCTION__);
}


void
WorkerManager::HandleUnprocessed() noexcept
{
	if (not WasFinished())
	{
		LOG_E("%s: attempt to call when there is Non Finished statet.",
		      __FUNCTION__);
		return;
	}

	while (not m_results->empty())
	{
		WorkerResult* res = m_results->pop_as<WorkerResult>();
		SaveResult(*res); //NOTE: can throw an IO-related exception
		res->Release();
	}
}
