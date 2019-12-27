#include "FileWriter.hpp"

#include <stdexcept>

#include <cstring>

#include "StringFormer.hpp"



FileWriter::FileWriter(std::string_view name, file_type_e type)
	: FileBase("stdout")
{
	this->Reset(name, type);
}


void
FileWriter::Reset(std::string_view name, file_type_e type)
{
	char const* mode = nullptr;
	switch (type)
	{
	case file_type_e::TEXT:   mode = "w"; break;
	case file_type_e::BINARY: mode = "wb"; break;
	}

	FileBase::Reset(name, mode);
}


void
FileWriter::Write(std::string_view sv)
{
	Write(reinterpret_cast<uint8_t const*>(sv.data()), sv.size());
}


void
FileWriter::Write(uint8_t const* data, std::size_t size)
{
	if (not data or size == 0) { return; }

	std::size_t written = std::fwrite(data, sizeof(*data), size, GetHandler());
	if (written != size)
	{
		constexpr std::size_t BUF_SIZE = 256;
		char buffer[BUF_SIZE];

		throw std::runtime_error(StringFormer(buffer, BUF_SIZE)
		    ("Unexpexted was written less bytes: exp=%zu, act=%zu. Error: %s",
		     size, written, std::strerror(errno)).c_str());
	}
	FlushIfNeeded();
}


void
FileWriter::Write(char ch)
{
	std::fputc(ch, GetHandler());
	FlushIfNeeded();
}

