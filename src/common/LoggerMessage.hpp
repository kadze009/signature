#pragma once

#include <array>
#include <string_view>

#include <cstdint>

#include "Pool.hpp"



class LoggerMessage : public ItemPool
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

	content_t& RefContent()           { return m_content; }
	std::string_view GetSV() const    { return std::string_view(m_content.data()); }

	void MakeFree() const                         { m_next = nullptr; Release(); }
	void SetNext(LoggerMessage const* next) const { m_next = next; }
	LoggerMessage const* GetNext() const          { return m_next; }

private:
	content_t                       m_content;
	mutable LoggerMessage const*    m_next = nullptr;
};

