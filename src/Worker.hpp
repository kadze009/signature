#pragma once

#include <memory>
#include <vector>
#include <future>
#include <exception>

#include <cstdint>

#include "common/Pool.hpp"
#include "common/FileReader.hpp"
#include "algo/IHasher.hpp"



class WorkerManager;
class Worker;



class WorkerResult : public ItemPool
{
public:
	using hash_t = std::vector<uint8_t>;

	std::uint64_t GetBlockNum() const            { return m_blockNum; }
	hash_t const& GetHash() const                { return m_hash; }

	// Used by IThreadProcessor
	void EndOfHandle() const                     { m_next = nullptr; Release(); }
	void SetNext(WorkerResult const* next) const { m_next = next; }
	WorkerResult const* GetNext() const          { return m_next; }

private:
	friend class Worker;

	void SetBlockNum(std::uint64_t v)            { m_blockNum = v; }
	hash_t& RefHash()                            { return m_hash; }

	std::uint64_t    m_blockNum = 0;
	hash_t           m_hash;

	mutable WorkerResult const* m_next = nullptr;
};



class Worker
{
public:
	using hasher_t  = std::unique_ptr<algo::IHasher>;
	using readbuf_t = std::vector<uint8_t>;

	Worker(Worker const&)             = delete;
	Worker& operator= (Worker const&) = delete;

	Worker(WorkerManager& mgr, std::uint64_t block_num);
	Worker(Worker&&)             = default;
	Worker& operator= (Worker&&) = default;
	~Worker()                    = default;

	std::uint64_t GetBlockNum() const    { return m_blockNum; }
	hasher_t const& GetHasher() const    { return m_hasher; }
	bool IsNeedStop() const              { return m_isNeedStop; }
	void SetStop()                       { m_isNeedStop = true; }

	void RunAsync();
	bool IsRunning() const noexcept      { return m_isRunning; }
	bool HasError() const noexcept       { return static_cast<bool>(m_exceptPtr); }
	void ThrowError() const;

private:
	using future_t = std::future<void>; // void - result of `Run` method

	void Run() noexcept;
	void DoWork();

	WorkerManager*       m_mgr;
	hasher_t             m_hasher;
	future_t             m_future;
	FileReader           m_in;

	static constexpr std::size_t INIT_RESULTS_SIZE = 64;
	static constexpr std::size_t INC_RESULTS_POOL  = 32;
	Pool<WorkerResult>&  m_results;
	readbuf_t            m_readBuffer;

	std::exception_ptr   m_exceptPtr;
	std::uint64_t        m_blockNum;
	bool                 m_isNeedStop = false;
	bool                 m_isRunning = false;
};

