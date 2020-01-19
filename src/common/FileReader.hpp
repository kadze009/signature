#pragma once

#include "FileBase.hpp"



class FileReader : public virtual FileBase
{
public:
	enum class file_type_e : uint8_t { TEXT, BINARY };

	FileReader(FileReader const&)             = delete;
	FileReader& operator= (FileReader const&) = delete;

	FileReader(std::string_view name, file_type_e type = file_type_e::BINARY);
	FileReader(FileReader&&)             = default;
	FileReader& operator= (FileReader&&) = default;
	~FileReader()                        = default;

	void Reset(std::string_view name, file_type_e type = file_type_e::BINARY);

	std::size_t Read(char* buf_data, std::size_t buf_size);
	std::size_t Read(uint8_t* buf_data, std::size_t buf_size);
	void SkipNextBytes(std::uintmax_t offset);
};

