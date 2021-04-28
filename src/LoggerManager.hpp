#pragma once

#include <atomic>
#include <string_view>
#include <thread>

#include <cstdint>
#include <cstdio>

#include "common/LoggerMessage.hpp"
#include "common/Singletone.hpp"
#include "common/MpocQueue.hpp"
#include "common/MpocQueueProducer.hpp"
#include "common/FileWriter.hpp"
#include "common/PoolStorage.hpp"



class LoggerManager : public Singletone<LoggerManager>
{
public:
	using msg_queue_t    = std::shared_ptr<MpocQueue>;
	using pool_storage_t = PoolStorage<LoggerMessage>;
	using msg_pool_t     = pool_storage_t::pool_t;

	static constexpr uint32_t DEFAULT_QUEUE_POLLING_MS = 300;
	static constexpr size_t   INIT_MSG_POOL_SIZE       = 64;
	static constexpr size_t   INC_MSG_POOL_VAL         = 32;

	LoggerManager();
	~LoggerManager();

	void NotThreadSafe_NewLogfile(std::string_view filename);
	void NotThreadSafe_SetLogfile(std::string_view filename);
	std::string_view GetLogfile() const       { return m_out.GetName(); }

	void SetFlushing(bool need_flush)         { m_out.SetFlushing(need_flush); }
	bool IsFlushing() const                   { return m_out.IsFlushing(); }

	size_t LogProducerCount() const noexcept    { return m_queue->ProducerCount(); }
	MpocQueueProducer NewLogProducer() noexcept { return m_queue->NewProducer(); }
	msg_pool_t& NewMessagePool() noexcept
	{
		return m_pool_storage.Allocate(INIT_MSG_POOL_SIZE, INC_MSG_POOL_VAL);
	}

	void StartHandleMessagesInSeparateThread();
	void HandleUnprocessed() noexcept;

private:
	void HandleMessages() noexcept;
	void PrintMessage(std::string_view);
	void NotThreadSafe_InternalPrint(char const* format, ...) noexcept;

private:
	pool_storage_t       m_pool_storage;
	FileWriter           m_out;
	msg_queue_t          m_queue;
	std::thread          m_msg_handler;
	bool                 m_need_stop_handling {false};
	bool                 m_is_handling {false};
};

