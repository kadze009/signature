#include "Worker.hpp"

#include <algorithm>

#include "common/Logger.hpp"
#include "common/PoolManager.hpp"
#include "algo/HasherFactory.hpp"
#include "WorkerManager.hpp"



Worker::Worker(WorkerManager* mgr, std::uint64_t block_num)
	: m_mgr(mgr)
	, m_in(m_mgr->GetConfig().GetInputFile(), FileReader::file_type_e::BINARY)
	, m_results(PoolManager::RefInstance()
	            .NewPool<WorkerResult>(INIT_RESULTS_SIZE, INC_RESULTS_POOL))
	, m_blockNum(block_num)
{
	Config const& cfg = m_mgr->GetConfig();

	m_hasher = algo::HasherFactory::Create(*cfg.GetInitAlgo());
	m_readBuffer.resize(std::min(cfg.GetReadBufferSize(), cfg.GetBlockSizeKB()*1024));
}



void
Worker::RunAsync()
{
	LOG_D("%s: start async running", __FUNCTION__);
	m_isRunning = true;
	m_future = std::async(std::launch::async, &Worker::Run, this);
}



void
Worker::Run() noexcept
{
	LOG_D("%s: Start execution", __FUNCTION__);
	try
	{
		DoWork();
	}
	catch(...)
	{
		m_exceptPtr = std::current_exception();
	}

	LOG_D("%s: Stop execution. The results pool size = %zu",
	      __FUNCTION__, m_results.size());
	m_isRunning = false;
}



void
Worker::DoWork()
{
	Config const& cfg = m_mgr->GetConfig();
	std::uint64_t const last_block_num = cfg.GetLastBlockNum();
	std::uint64_t const blocks_shift   = cfg.GetBlocksShift();
	std::uintmax_t const bytes_shift   = cfg.GetFileBytesShift();
	std::uintmax_t const block_size    = cfg.GetBlockSizeKB() * 1024;

	if (m_blockNum > last_block_num) { return; }
	LOG_D("%s: shift file pointer on the %zuth block", __FUNCTION__, m_blockNum);
	m_in.SkipNextBytes(block_size * m_blockNum);

	std::uintmax_t remains = 0;
	std::size_t read_bytes = 0;
	while (m_blockNum <= last_block_num)
	{
		LOG_I("%s: Start calculate BLOCK #%zu", __FUNCTION__, m_blockNum);
		if (IsNeedStop())
		{
			LOG_W("%s: Detect 'stop' sign. Abort calculation BLOCK #%zu.",
			      __FUNCTION__, m_blockNum);
			break;
		}
		remains = block_size;
		m_hasher->Init(*cfg.GetInitAlgo());

		while (remains != 0)
		{
			read_bytes = m_in.Read(m_readBuffer.data(), m_readBuffer.size());
			if (read_bytes != 0)
			{
				if (remains > read_bytes)
				{
					m_hasher->Update(m_readBuffer.data(), read_bytes);
					remains -= read_bytes;
				}
				else
				{
					m_hasher->Update(m_readBuffer.data(), remains);
					remains = 0;
				}
			}
			else
			{
				m_hasher->Update(cfg.GetBlockFiller(), remains);
				remains = 0;
			}
		}
		WorkerResult& result = m_results.allocate();
		auto& hash_buf = result.RefHash();
		hash_buf.resize(m_hasher->ResultSize()); //TODO: resize each time?
		result.SetBlockNum(m_blockNum);
		m_hasher->Finish(hash_buf.data());
		m_mgr->AddResult(result);

		m_in.SkipNextBytes(bytes_shift);
		LOG_I("%s: Finish calculate BLOCK #%zu", __FUNCTION__, m_blockNum);
		m_blockNum += blocks_shift;
	}
}



void
Worker::ThrowError() const
{
	if (HasError()) { std::rethrow_exception(m_exceptPtr); }
}

