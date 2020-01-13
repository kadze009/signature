#pragma once

#include "common/Config.hpp"
#include "Worker.hpp"


class WorkerManager
{
public:
	WorkerManager(WorkerManager&&)                 = delete;
	WorkerManager(WorkerManager const&)            = delete;
	WorkerManager& operator=(WorkerManager&&)      = delete;
	WorkerManager& operator=(WorkerManager const&) = delete;

	WorkerManager(Config&);
	~WorkerManager() = default;

	void Start() noexcept;
	void DoWork() noexcept;
	bool IsFinished() const noexcept               { return m_isFinished; }

	Config const& GetConfig() const                { return m_cfg; }

private:
	WorkerCtx* FindFailedWorkerCtx() const;
	bool IsAllWorkerStop() const;

	Config&    m_cfg;
	bool       m_isFinished = false;
};

