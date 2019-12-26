#pragma once

#include <string_view>

#include <cstdio>
#include <cstdint>



class FileWriter
{
public:
	enum class file_type_e : uint8_t { TEXT, BINARY };

	FileWriter(FileWriter&&)                  = delete;
	FileWriter(FileWriter const&)             = delete;
	FileWriter& operator= (FileWriter&&)      = delete;
	FileWriter& operator= (FileWriter const&) = delete;

	FileWriter(std::string_view name, file_type_e type = file_type_e::BINARY);
	~FileWriter();

	void Reset(std::string_view name, file_type_e type = file_type_e::BINARY);

	void SetFlushing(bool is_need)                { m_need_flush = is_need; }
	bool IsFlushing() const                       { return m_need_flush; }
	void SetBufferSize(std::size_t size);

	void Write(std::string_view);
	void Write(uint8_t const* data, std::size_t size);
	void Write(char ch);

private:
	void Close();

	std::string_view  m_name;
	bool              m_need_flush = false;
	FILE*             m_file       = nullptr;
};

