#include "FileReader.hpp"



FileReader::FileReader(std::string_view name, file_type_e type /* = file_type_e::BINARY */)
	: FileBase("stdin")
{
	this->Reset(name, type);
}


void
FileReader::Reset(std::string_view name, file_type_e type /* = file_type_e::BINARY */)
{
	char const* mode = nullptr;
	switch (type)
	{
	case file_type_e::TEXT:   mode = "r";  break;
	case file_type_e::BINARY: mode = "rb"; break;
	}

	FileBase::Reset(name, mode);
}


std::size_t
FileReader::Read(char* buf_data, std::size_t buf_size)
{
	return Read(reinterpret_cast<uint8_t*>(buf_data), buf_size);
}


std::size_t
FileReader::Read(uint8_t* buf_data, std::size_t buf_size)
{
	if (not buf_data or 0 == buf_size) { return 0; }
	return std::fread(buf_data, sizeof(*buf_data), buf_size, GetHandler());
}

