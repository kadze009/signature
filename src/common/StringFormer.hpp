#pragma once

#include <cstdio>
#include <cstdint>


class StringFormer
{
public:
	StringFormer(StringFormer&&)                 = delete;
	StringFormer(StringFormer const&)            = delete;
	StringFormer& operator=(StringFormer&&)      = delete;
	StringFormer& operator=(StringFormer const&) = delete;
	~StringFormer()                              = default;

	//TODO: check if (size == 0 or buffer == nullptr)
	StringFormer(char* buffer, std::size_t size)
		: m_start(buffer)
		, m_end(buffer + size)
		, m_free(buffer)
	{}

	int append(char const* format, ...)
	{
		va_list args;
		va_start(args, format);
		int res = append(format, args);
		va_end(args);
		return res;
	}

	int append(char const* format, va_list vlist)
	{
		if (not format || free_size() == 0) { return 0; }

		int printed = vsnprintf(m_free, free_size(), format, vlist);
		int res = std::min(printed, free_size());
		m_free += res;
		return res - (is_full() ? 1 : 0);
	}

	std::string_view sv() const    { return {m_start, size()}; }

	std::size_t size() const       { return m_free - m_start - (is_full() ? 1 : 0); }
	std::size_t capacity() const   { return m_end - m_start; }
	int free_size() const          { return m_end - m_free; }
	bool is_full() const           { return m_free == m_end; }
	bool empty() const             { return m_free == m_start; }

private:
	char* m_start;
	char* m_end;
	char* m_free;
};

