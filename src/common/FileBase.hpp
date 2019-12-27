#pragma once

#include <string_view>

#include <cstdio>
#include <cstdint>



class FileBase
{
public:
	FileBase(FileBase&&)                  = delete;
	FileBase(FileBase const&)             = delete;
	FileBase& operator= (FileBase&&)      = delete;
	FileBase& operator= (FileBase const&) = delete;

	void SetFlushing(bool is_need)   { m_need_flush = is_need; }
	bool IsFlushing() const          { return m_need_flush; }
	void FlushIfNeeded();

	void SetBufferSize(std::size_t);
	std::string_view GetName() const { return m_name; }

protected:
	FileBase(std::string_view, char const* mode = nullptr);
	~FileBase();

	FILE* GetHandler()               { return m_file; }
	void Close();
	void Reset(std::string_view, char const* mode);

private:
	std::string_view m_name;
	FILE*            m_file;
	bool             m_need_flush = false;
};

