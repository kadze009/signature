#pragma once

#include "common/MpocQueue.hpp"
#include "common/MpocQueueProducer.hpp"
#include "common/FileWriter.hpp"
#include "common/PoolStorage.hpp"
#include "Config.hpp"
#include "Worker.hpp"


class WorkerManager
{
public:
	using pool_storage_t = PoolStorage<WorkerResult>;
	using result_pool_t  = pool_storage_t::wp_pool_t;
	using result_queue_t = std::shared_ptr<MpocQueue>;

	static constexpr size_t   INIT_RESULTS_SIZE          = 64;
	static constexpr size_t   INC_RESULTS_POOL           = 32;
	static constexpr uint32_t DEFAULT_QUEUE_POLLING_MS   = 1000;

	WorkerManager(WorkerManager&&)                 = delete;
	WorkerManager(WorkerManager const&)            = delete;
	WorkerManager& operator=(WorkerManager&&)      = delete;
	WorkerManager& operator=(WorkerManager const&) = delete;

	explicit WorkerManager(Config&);
	~WorkerManager();

	bool Start() noexcept;
	bool DoWork() noexcept;
	void SaveResult(WorkerResult const&);
	bool WasFinished() const noexcept              { return m_wasFinished; }
	bool IsAborting() const noexcept               { return m_isAborting; }
	void StartAborting() noexcept;
	Config const& GetConfig() const noexcept       { return m_cfg; }

	MpocQueueProducer NewResultProducer() noexcept { return m_results->NewProducer(); }
	result_pool_t NewResultPool() noexcept
	{
		return m_pool_storage.Allocate(INIT_RESULTS_SIZE, INC_RESULTS_POOL);
	}
	void HandleUnprocessed() noexcept;

private:
	Worker* FindFailedWorker() noexcept;
	bool AreAllWorkersStop() const noexcept;
	void StopAllWorkers() noexcept;

private:
	Config&               m_cfg;
	FileWriter            m_out;
	pool_storage_t        m_pool_storage;
	result_queue_t        m_results;
	std::vector<Worker>   m_workers;

	bool                  m_wasFinished = false;
	bool                  m_isAborting  = false;
};

