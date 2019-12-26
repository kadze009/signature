#pragma once

#include <mutex>
#include <atomic>
#include <string_view>

#include <cstdint>
#include <cstdio>

#include "Singletone.hpp"
#include "LoggerMessage.hpp"



class LoggerManager : public Singletone<LoggerManager>
{
public:
	using msg_t = LoggerMessage const;

	void SetLogfile(std::string_view filename); //TODO

	void SetSyncMode(bool need_sync)       { m_needSync = need_sync; }
	bool IsSyncMode() const                { return m_needSync; }

	void SetFlushing(bool need_flush)      { m_needFlush = need_flush; }
	bool IsFlushing() const                { return m_needFlush; }

	void AddMessage(msg_t&);
	void PrintBatchOfMessages(std::size_t batch_size);

private:
	void PushMessageBack(msg_t&);
	void PrintMessage(msg_t&);
	msg_t* MakeFreeAndGetNext(msg_t&);


	bool m_needFlush = false;
	bool m_needSync  = true;

	using print_mutex_t = std::mutex;
	print_mutex_t m_print_mutex;
	
	LoggerMessage const*              m_head_msg  = nullptr;
	std::atomic<LoggerMessage const*> m_last_msg  = nullptr;

	std::FILE* m_out = stdout;
};

