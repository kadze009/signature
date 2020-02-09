#include "FileReader.hpp"

#include <fstream>
#include <iostream>
#include <limits>



FileReader::FileReader(char const* name)
	: m_name(name)
{
	if (not m_name) { m_name = DEFAULT_NAME.data(); }
	m_in.reset(DEFAULT_NAME == m_name
	           ? &std::cin
	           : new std::ifstream(m_name, std::ios::binary));
}


FileReader::~FileReader()
{
	ClearHandler();
}


void
FileReader::ClearHandler() noexcept
{
	std::istream* in = m_in.get();
	if (&std::cin == in) { m_in.release(); }
}


std::size_t
FileReader::Read(char* buf_data, std::size_t buf_size)
{
	if (not buf_data or 0 == buf_size) { return 0; }
	m_in->read(buf_data, buf_size); //NOTE: can throws exception
	return m_in->gcount();
}


std::size_t
FileReader::Read(uint8_t* buf_data, std::size_t buf_size)
{
	return Read(reinterpret_cast<char*>(buf_data), buf_size);
}


void
FileReader::SkipNextBytes(std::uintmax_t offset)
{
	if (offset == 0) { return; }
	m_in->seekg(offset, std::ios_base::cur);
}

