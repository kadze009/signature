#pragma once

#include <array>
#include <string_view>

#include <cstdint>

#include "Pool.hpp"
#include "MpocQueueItem.hpp"



class LoggerMessage : public PoolItem, public MpocQueueItem
{
public:
	LoggerMessage()                                = default;
	LoggerMessage(LoggerMessage&&)                 = delete;
	LoggerMessage(LoggerMessage const&)            = delete;
	LoggerMessage& operator=(LoggerMessage&&)      = delete;
	LoggerMessage& operator=(LoggerMessage const&) = delete;
	~LoggerMessage()                               = default;

	static constexpr std::size_t MSG_MAX_SIZE = 512;
	using content_t = std::array<char, MSG_MAX_SIZE>;

	content_t& content() noexcept        { return m_content; }
	std::string_view sv() const noexcept { return std::string_view(m_content.data()); }

private:
	content_t                       m_content;
	mutable LoggerMessage const*    m_next = nullptr;
};

