#pragma once

#include <mutex>
#include <atomic>
#include <string_view>

#include <cstdint>
#include <cstdio>

#include "common/LoggerMessage.hpp"
#include "common/Singletone.hpp"
#include "common/IDeferedQueue.hpp"
#include "common/FileWriter.hpp"



//TODO:
//    * **The problem:** what will happen if the first thread calls
//      `PrintBatchOfMessages` and the second thread calls `SetLogfile` or
//      `NewLogfile`?
//      **Temp solution:** I expect that these methods will be called only by
//      one thread.
//      **Bad solution:** save the thread ID of the main thread and check it
//      before execution some methods. If actual thread ID and saved thread ID
//      were different then it would be an developer error and the code would
//      `throw 666;` (uncathchable exception).
//
//    * LoggerManager should take the pools of messages for Loggers like a
//      shared pointer objects and should wait until pools are state 'FREE'
//      or a number of a pool owners is ONE. It needs for `HandleUnprocessed`
//      calling. **WARNING**: possible freezing because LoggerManager starts to
//      destroy itself but no one will start stopping threads.
//
class LoggerManager : public Singletone<LoggerManager>,
                      public IDeferedQueue<LoggerMessage>
{
public:
	LoggerManager();
	~LoggerManager();

	void AddMessage(LoggerMessage const&);

	void NewLogfile(std::string_view filename);
	void SetLogfile(std::string_view filename);
	std::string_view GetLogfile() const       { return m_out.GetName(); }

	void SetSyncMode(bool need_sync) noexcept { m_needSync = need_sync; }
	bool IsSyncMode() const noexcept          { return m_needSync; }

	void SetFlushing(bool need_flush)         { m_out.SetFlushing(need_flush); }
	bool IsFlushing() const                   { return m_out.IsFlushing(); }

private:
	// IDeferedQueue
	void HandleItem(LoggerMessage const&) override;

	void PrintMessage(std::string_view);

private:
	using print_mutex_t = std::mutex;
	print_mutex_t    m_print_mutex;
	FileWriter       m_out;
	bool             m_needSync  = true;
};

