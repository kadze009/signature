#include "FileWriter.hpp"

#include <iostream>
#include <fstream>

#include <cstring>



FileWriter::FileWriter(std::string_view name, file_type_e type /* = file_type_e::BINARY */)
{
	Reset(name, type);
}

FileWriter::~FileWriter()
{
	DetachIfStandardStreams();
}


void
FileWriter::DetachIfStandardStreams() noexcept
{
	std::ostream* out = m_out.get();
	if (   &std::cout == out
	    or &std::cerr == out
	    or &std::clog == out)
	{
		m_out.release();
	}
}


void
FileWriter::Reset(std::string_view name, file_type_e type /* = file_type_e::BINARY */)
{
	constexpr std::string_view STANDARD_COUT_NAME   {"cout"};
	constexpr std::string_view STANDARD_STDOUT_NAME {"stdout"};
	constexpr std::string_view STANDARD_CERR_NAME   {"cerr"};
	constexpr std::string_view STANDARD_STDERR_NAME {"stderr"};
	constexpr std::string_view STANDARD_CLOG_NAME   {"clog"};

	auto mode = std::ios::out;
	switch (type)
	{
	case file_type_e::BINARY: mode = std::ios::binary; break;
	case file_type_e::TEXT:   break;
	}

	DetachIfStandardStreams();
	m_name.assign((not name.empty()) ? name : DEFAULT_NAME);

	if      (   STANDARD_CERR_NAME   == m_name
	         or STANDARD_STDERR_NAME == m_name)
	{
		m_out.reset(&std::cerr);
	}
	else if (   STANDARD_COUT_NAME   == m_name
	         or STANDARD_STDOUT_NAME == m_name)
	{
		m_out.reset(&std::cout);
	}
	else if (STANDARD_CLOG_NAME == m_name)
	{
		m_out.reset(&std::clog);
	}
	else
	{
		m_out = std::make_unique<std::ofstream>(m_name, mode);
	}
}


void
FileWriter::FlushIfNeeded()
{
	if (IsFlushing()) { m_out->flush(); }
}


void
FileWriter::Write(char ch)
{
	m_out->put(ch);
	FlushIfNeeded();
}


void
FileWriter::Write(std::string_view sv)
{
	Write(sv.data(), sv.size());
}


void
FileWriter::Write(uint8_t const* data, size_t size)
{
	Write(reinterpret_cast<char const*>(data), size);
}


void
FileWriter::Write(char const* data, size_t size)
{
	if (not data or size == 0) { return; }
	m_out->write(data, size);
	FlushIfNeeded();
}


void
FileWriter::Write(void const* data, size_t elem_size, size_t elem_count)
{
	auto const* bin_data = static_cast<char const*>(data);
	size_t bin_size = elem_size * elem_count;
	Write(bin_data, bin_size);
}


void
FileWriter::SetBufferSize(char* buff, size_t buff_size)
{
	if (not buff) { buff_size = 0; }
	m_out->rdbuf()->pubsetbuf(buff, buff_size);
}
