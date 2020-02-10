#pragma once

#include <mutex>
#include <atomic>
#include <string_view>

#include <cstdint>
#include <cstdio>

#include "common/LoggerMessage.hpp"
#include "common/Singletone.hpp"
#include "common/IThreadProcessor.hpp"
#include "common/FileWriter.hpp"



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
//    * (AZ) LoggerManager should take the pools of messages for Loggers like a
//      shared pointer objects and should wait while pools have not state 'FREE'
//      or a number of a pool owners is not ONE. It's need for `HandleUnprocessed`
//      calling. **WARNING**: possible freezing because LoggerManager starts
//      destroing but no one starts threads stoping.
//
class LoggerManager : public Singletone<LoggerManager>,
                      public IThreadProcessor<LoggerMessage>
{
public:
	LoggerManager();
	~LoggerManager();

	void AddMessage(LoggerMessage const&);

	void NewLogfile(char const* filename);
	void SetLogfile(char const* filename);
	std::string_view GetLogfile() const    { return m_out.GetName(); }

	void SetSyncMode(bool need_sync)       { m_needSync = need_sync; }
	bool IsSyncMode() const                { return m_needSync; }

	void SetFlushing(bool need_flush)      { m_out.SetFlushing(need_flush); }
	bool IsFlushing() const                { return m_out.IsFlushing(); }

private:
	// IThreadProcessor
	void HandleItem(LoggerMessage const&) override;

	void PrintMessage(std::string_view);

	using print_mutex_t = std::mutex;
	print_mutex_t    m_print_mutex;
	FileWriter       m_out;
	bool             m_needSync  = true;
};

