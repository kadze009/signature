#pragma once

#include <mutex>
#include <atomic>
#include <string_view>

#include <cstdint>
#include <cstdio>

#include "Singletone.hpp"
#include "LoggerMessage.hpp"
#include "FileWriter.hpp"



//TODO:
//    * (AZ) **The problem:** what is happend when the first thread calls
//      `PrintBatchOfMessages` and the second thread calls `SetLogfile` or
//      `NewLogfile`?
//      **Temp solution:** I expect that these methods call only one thread.
//      **Bad solution:** save the thread ID of the main thread and check it
//      before execution some methods. If a thread ID is different with saved
//      then it's the error of developer and can `throw 666;`
//      (uncathchable exception)
//
class LoggerManager : public Singletone<LoggerManager>
{
public:
	using msg_t = LoggerMessage const;

	LoggerManager();

	void NewLogfile(std::string_view filename);
	void SetLogfile(std::string_view filename);
	std::string_view GetLogfile() const    { return m_out.GetName(); }

	void SetSyncMode(bool need_sync)       { m_needSync = need_sync; }
	bool IsSyncMode() const                { return m_needSync; }

	void SetFlushing(bool need_flush)      { m_out.SetFlushing(need_flush); }
	bool IsFlushing() const                { return m_out.IsFlushing(); }

	void AddMessage(msg_t&);
	void HandleBatchOfResults(std::size_t batch_size);
	void HandleUnsavedResults();
	bool HasUnsaved() const noexcept       { return m_head_msg != nullptr; }

private:
	void PushMessageBack(msg_t&);
	void PrintMessage(msg_t&);
	void PrintMessage(std::string_view);
	msg_t* MakeFreeAndGetNext(msg_t&);

	bool m_needSync  = true;

	using print_mutex_t = std::mutex;
	print_mutex_t m_print_mutex;
	
	msg_t*              m_head_msg  = nullptr;
	std::atomic<msg_t*> m_last_msg  = nullptr;

	FileWriter m_out;
};

