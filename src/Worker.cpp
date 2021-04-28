#include "Worker.hpp"

#include <algorithm>

#include <cstdarg>

#include "common/Logger.hpp"
#include "algo/HasherFactory.hpp"
#include "WorkerManager.hpp"



Worker::Worker(WorkerManager& mgr, std::uint64_t block_num)
	: m_mgr(&mgr)
	, m_in(m_mgr->GetConfig().GetInputFile().c_str())
	, m_results(m_mgr->NewResultPool())
	, m_producer(m_mgr->NewResultProducer())
	, m_blockNum(block_num)
{
	Config const& cfg = m_mgr->GetConfig();

	m_hasher = algo::HasherFactory::Create(*cfg.GetInitAlgo());
	m_readBuffer.resize(std::min(cfg.GetReadBufferSize(), cfg.GetBlockSizeKB()*1024));
}



bool
Worker::RunAsync() noexcept
{
	LOG_D("%s: start async running", __FUNCTION__);
	bool res = false;
	m_isRunning = false;
	try
	{
		m_future = std::async(std::launch::async, &Worker::Run, this);
		res = true;
		m_isRunning = true;
	}
	catch (std::system_error const& ex)
	{
		LOG_E("%s: caught system_error: %s (error code=%s)",
		      __FUNCTION__, ex.what(), ex.code().message().c_str());
	}
	catch (std::bad_alloc const&)
	{
		LOG_E("%s: caught bad_alloc", __FUNCTION__);
	}
	return res;
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
		LOG_E("%s BLOCK[%zu]: catch an exception", __FUNCTION__, m_blockNum);
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
		if (not m_producer.push(result))
		{
			ThrowRuntimeError("%s: can't save the result. Abort execution.",
				__FUNCTION__);
		}

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


void Worker::ThrowRuntimeError(char const* format, ...) const
{
	constexpr std::size_t MSG_SIZE = 256;
	static std::array<char, MSG_SIZE> msg;
	static StringFormer fmt(msg.data(), msg.size());

	fmt.reset();
	va_list args;
	va_start(args, format);
	fmt.append("BLOCK[%zu] ", m_blockNum);
	fmt.append(format, args);
	va_end(args);
	throw std::runtime_error(fmt.c_str());
}
