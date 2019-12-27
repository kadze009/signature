#pragma once

#include "FileBase.hpp"


class FileWriter : public virtual FileBase
{
public:
	enum class file_type_e : uint8_t { TEXT, BINARY };

	FileWriter(FileWriter&&)                  = delete;
	FileWriter(FileWriter const&)             = delete;
	FileWriter& operator= (FileWriter&&)      = delete;
	FileWriter& operator= (FileWriter const&) = delete;
	~FileWriter()                             = default;

	FileWriter(std::string_view name, file_type_e type = file_type_e::BINARY);

	void Reset(std::string_view name, file_type_e type = file_type_e::BINARY);

	void Write(std::string_view);
	void Write(uint8_t const* data, std::size_t size);
	void Write(char ch);
};

