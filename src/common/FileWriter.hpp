#pragma once

#include "FileBase.hpp"


class FileWriter : public virtual FileBase
{
public:
	enum class file_type_e : uint8_t { TEXT, BINARY };

	FileWriter(FileWriter const&)             = delete;
	FileWriter& operator= (FileWriter const&) = delete;

	FileWriter(std::string_view name, file_type_e type = file_type_e::BINARY);
	FileWriter(FileWriter&&)             = default;
	FileWriter& operator= (FileWriter&&) = default;
	~FileWriter()                        = default;

	void Reset(std::string_view name, file_type_e type = file_type_e::BINARY);

	void Write(char ch);
	void Write(std::string_view);
	void Write(uint8_t const* data, std::size_t size);
	void Write(void const* data, std::size_t elem_size, std::size_t elem_count);
};

