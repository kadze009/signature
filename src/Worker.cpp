#include "Worker.hpp"

#include "common/Logger.hpp"
#include "algos/HasherFactory.hpp"
#include "WorkerManager.hpp"



Worker::Worker(WorkerManager* mgr, std::size_t block_num)
	: m_mgr(mgr)
	, m_in(m_mgr->GetConfig().GetInputFile(), FileReader::file_type_e::BINARY)
	, m_blockNum(block_num)
{
	Config const& cfg = m_mgr->GetConfig();

	m_hasher = algo::HasherFactory::Create(*cfg.GetInitAlgo());
	m_readBuffer.resize(cfg.GetReadBufferSize());
}



bool
Worker::ReadData(std::uintmax_t offset)
{
	//TODO: implement
	return false;
}



std::size_t
Worker::NextBlockNumber()
{
	m_blockNum += GetMgr()->GetConfig().GetBlockNumShift();
	return m_blockNum;
}



void
Worker::run()
{
	Config const& cfg = m_mgr.GetConfig();
	std::uintmax_t const block_size = cfg.GetBlockSizeKB() * 1024;
	std::uintmax_t cur_offset = 0;
	std::uintmax_t remains    = 0;

	while ((cur_offset = m_blockNum * block_size) < cfg.GetInputFileSize())
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
			if (ReadData(cur_offset))
			{
				if (remains >= m_readBuffer.size())
				{
					remains -= m_readBuffer.size();
					m_hasher->Update(m_readBuffer.data(), m_readBuffer.size());
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
		// result = m_results.alloc();
		// result.SetBlockNum(m_blockNum);
		// m_hasher->Finish(result.RefHashBuf());
		// m_mgr.AddResult(result);
		LOG_I("%s: Stop calculate BLOCK #%zu", __FUNCTION__, m_blockNum);

		NextBlockNumber();
	}
	LOG_D("%s: Stop execution.", __FUNCTION__);
}

