#include "Worker.hpp"

#include "common/Logger.hpp"
#include "algos/HasherFactory.hpp"
#include "WorkerManager.hpp"



Worker::Worker(WorkerManager* mgr, std::uint64_t block_num)
	: m_mgr(mgr)
	, m_in(m_mgr->GetConfig().GetInputFile(), FileReader::file_type_e::BINARY)
	, m_blockNum(block_num)
{
	Config const& cfg = m_mgr->GetConfig();

	m_hasher = algo::HasherFactory::Create(*cfg.GetInitAlgo());
	m_readBuffer.resize(cfg.GetReadBufferSize());
}



std::size_t
Worker::ReadData(std::uintmax_t offset)
{
	m_in.SkipBytes(offset);
	return m_in.Read(m_readBuffer.data(), m_readBuffer.size());
}



std::uint64_t
Worker::NextBlockNumber()
{
	return m_blockNum += m_mgr->GetConfig().GetBlockNumShift();
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
	std::uintmax_t const block_size = cfg.GetBlockSizeKB() * 1024;
	std::uintmax_t const file_size  = cfg.GetInputFileSize();
	std::uintmax_t cur_offset = 0;
	std::uintmax_t remains    = 0;
	std::size_t read_bytes    = 0;

	while ((cur_offset = m_blockNum * block_size) < file_size)
	{
		LOG_I("%s: Start calculate BLOCK #%zu (offset=%zu)",
		      __FUNCTION__, m_blockNum, cur_offset);

		remains = block_size;
		if (IsNeedStop())
		{
			LOG_W("%s: Detect 'stop' sign. Abort calculation BLOCK #%zu.",
			      __FUNCTION__, m_blockNum);
			break;
		}

		m_hasher->Init(*cfg.GetInitAlgo());
		while (remains > 0)
		{
			if ((read_bytes = ReadData(cur_offset)) != 0)
			{
				if (remains >= read_bytes)
				{
					remains -= read_bytes;
					m_hasher->Update(m_readBuffer.data(), read_bytes);
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

		LOG_I("%s: Finish calculate BLOCK #%zu", __FUNCTION__, m_blockNum);
		NextBlockNumber();
	}
}



void
Worker::ThrowError() const
{
	if (HasError()) { std::rethrow_exception(m_exceptPtr); }
}

