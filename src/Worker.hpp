#pragma once

#include <memory>
#include <vector>

#include <cstdint>

#include "common/FileReader.hpp"



namespace algo { struct IHasher; }
class WorkerManager;



class Worker
{
public:
	using hasher_t  = std::unique_ptr<algo::IHasher>;
	using readbuf_t = std::vector<uint8_t>;

	Worker(Worker&&)                 = delete;
	Worker(Worker const&)            = delete;
	Worker& operator=(Worker&&)      = delete;
	Worker& operator=(Worker const&) = delete;

	Worker(WorkerManager* mgr, std::size_t block_num);
	~Worker() = default;

	std::size_t GetBlockNum() const                { return m_blockNum; }
	hasher_t const& GetHasher() const              { return m_hasher; }
	bool IsNeedStop() const                        { return m_isNeedStop; }
	void SetStop()                                 { m_isNeedStop = true; }

	void run();

private:
	bool ReadData(std::uintmax_t offset);
	std::size_t NextBlockNumber();
	void SaveResult();


	WorkerManager*       m_mgr;
	hasher_t             m_hasher;
	FileReader           m_in;
	readbuf_t            m_readBuffer;
	std::size_t          m_blockNum;
	bool                 m_isNeedStop = false;
};

