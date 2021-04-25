#pragma once

#include "common/IDeferedQueue.hpp"
#include "common/FileWriter.hpp"
#include "Config.hpp"
#include "Worker.hpp"


class WorkerManager : public IDeferedQueue<WorkerResult>
{
public:
	static constexpr std::size_t DEFAULT_RESULTS_BATCH_SIZE = 128;

	WorkerManager(WorkerManager&&)                 = delete;
	WorkerManager(WorkerManager const&)            = delete;
	WorkerManager& operator=(WorkerManager&&)      = delete;
	WorkerManager& operator=(WorkerManager const&) = delete;

	explicit WorkerManager(Config&);
	~WorkerManager();

	bool Start() noexcept;
	void DoWork() noexcept;
	bool WasFinished() const noexcept              { return m_wasFinished; }
	bool IsAborting() const noexcept               { return m_isAborting; }
	void StartAborting() noexcept;
	Config const& GetConfig() const noexcept       { return m_cfg; }

private:
	// IDeferedQueue
	void HandleItem(WorkerResult const&) override;

	Worker* FindFailedWorker() noexcept;
	bool AreAllWorkersStop() const noexcept;
	void StopAllWorkers() noexcept;

private:
	Config&               m_cfg;
	FileWriter            m_out;
	std::vector<Worker>   m_workers;
	std::size_t           m_resultsBatchSize = DEFAULT_RESULTS_BATCH_SIZE;
	bool                  m_wasFinished = false;
	bool                  m_isAborting  = false;
};

