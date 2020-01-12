#pragma once

#include "common/Config.hpp"


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
	bool IsFinished() const                        { return m_isFinished; }

private:
	Config&    m_cfg;
	bool       m_isFinished = false;
};

