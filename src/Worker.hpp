#pragma once

#include <memory>
#include <vector>
#include <future>
#include <exception>

#include <cstdint>

#include "common/Pool.hpp"
#include "common/FileReader.hpp"
#include "common/MpocQueueItem.hpp"
#include "common/MpocQueueProducer.hpp"
#include "algo/IHasher.hpp"



class WorkerManager;
class Worker;



class WorkerResult : public PoolItem, public MpocQueueItem
{
public:
	using hash_t = std::vector<uint8_t>;

	std::uint64_t GetBlockNum() const noexcept { return m_blockNum; }
	hash_t const& GetHash() const noexcept     { return m_hash; }

private:
	friend class Worker;

	void SetBlockNum(std::uint64_t v) noexcept { m_blockNum = v; }
	hash_t& RefHash() noexcept                 { return m_hash; }

private:
	std::uint64_t    m_blockNum = 0;
	hash_t           m_hash;

	mutable WorkerResult const* m_next = nullptr;
};



class Worker
{
public:
	using wp_pool_t = std::weak_ptr<Pool<WorkerResult>>;
	using hasher_t  = std::unique_ptr<algo::IHasher>;
	using readbuf_t = std::vector<uint8_t>;

	Worker(Worker const&)             = delete;
	Worker& operator= (Worker const&) = delete;

	Worker(WorkerManager& mgr, std::uint64_t block_num);
	Worker(Worker&&)             = default;
	Worker& operator= (Worker&&) = default;
	~Worker()                    = default;

	std::uint64_t GetBlockNum() const noexcept { return m_blockNum; }
	hasher_t const& GetHasher() const noexcept { return m_hasher; }
	bool IsNeedStop() const noexcept           { return m_isNeedStop; }
	void SetStop() noexcept                    { m_isNeedStop = true; }

	bool RunAsync() noexcept;
	bool IsRunning() const noexcept      { return m_isRunning; }
	bool HasError() const noexcept       { return static_cast<bool>(m_exceptPtr); }
	void ThrowError() const;

private:
	void Run() noexcept;
	void DoWork();
	void ThrowRuntimeError(char const* format, ...) const;

private:
	using future_t = std::future<void>; // void - result of `Run` method

	WorkerManager*       m_mgr;
	hasher_t             m_hasher;
	future_t             m_future;
	FileReader           m_in;

	wp_pool_t            m_results;
	MpocQueueProducer    m_producer;
	readbuf_t            m_readBuffer;

	std::exception_ptr   m_exceptPtr;
	std::uint64_t        m_blockNum;
	bool                 m_isNeedStop = false;
	bool                 m_isRunning = false;
};

