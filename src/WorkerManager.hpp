#pragma once

#include "common/Singletone.hpp"



class WorkerManager : public Singletone<WorkerManager>
{
public:
	void Init() noexcept;
	void Run()  noexcept;

private:

};

