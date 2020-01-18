#include "FileWriter.hpp"

#include <cstring>

#include "Logger.hpp"



FileWriter::FileWriter(std::string_view name, file_type_e type)
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
FileWriter::Write(char ch)
{
	std::fputc(ch, GetHandler());
	FlushIfNeeded();
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
		THROW_ERROR("Unexpexted was written less bytes: exp=%zu, act=%zu. Error: %s",
		            size, written, std::strerror(errno));
	}
	FlushIfNeeded();
}


void
FileWriter::Write(void const* data, std::size_t elem_size, std::size_t elem_count)
{
	auto const* bin_data = static_cast<uint8_t const*>(data);
	std::size_t bin_size = elem_size * elem_count;
	Write(bin_data, bin_size);
}

