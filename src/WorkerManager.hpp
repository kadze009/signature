#pragma once

#include <atomic>

#include "common/Config.hpp"
#include "common/FileWriter.hpp"
#include "Worker.hpp"


class WorkerManager
{
public:
	using result_t = WorkerResult const;

	static constexpr std::size_t DEFAULT_RESULTS_BATCH_SIZE = 128;

	WorkerManager(WorkerManager&&)                 = delete;
	WorkerManager(WorkerManager const&)            = delete;
	WorkerManager& operator=(WorkerManager&&)      = delete;
	WorkerManager& operator=(WorkerManager const&) = delete;

	WorkerManager(Config&);
	~WorkerManager() = default;

	void Start() noexcept;
	void DoWork() noexcept;
	bool WasFinished() const noexcept              { return m_wasFinished; }
	bool IsAborting() const noexcept               { return m_isAborting; }
	void StartAborting() noexcept;
	Config const& GetConfig() const                { return m_cfg; }

	void AddResult(result_t& res);
	void HandleBatchOfResults(std::size_t batch_size);

	void HandleUnsavedResults();
	bool HasUnsaved() const noexcept               { return m_head_res != nullptr; }


private:
	result_t* MakeFreeAndGetNext(result_t&);
	void SaveResult(result_t&);

	Worker* FindFailedWorker();
	bool AreAllWorkersStop() const;

	result_t*              m_head_res  = nullptr;
	std::atomic<result_t*> m_last_res  = nullptr;

	Config&               m_cfg;
	FileWriter            m_out;
	std::vector<Worker>   m_workers;
	std::size_t           m_resultsBatchSize = DEFAULT_RESULTS_BATCH_SIZE;
	bool                  m_wasFinished = false;
	bool                  m_isAborting  = false;
};

