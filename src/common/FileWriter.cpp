#include "FileWriter.hpp"

#include <stdexcept>

#include <cstring>

#include "StringFormer.hpp"



FileWriter::FileWriter(std::string_view name, file_type_e type)
	: m_name(name)
{
	Reset(m_name, type);
}


FileWriter::~FileWriter()
{
	Close();
}


void
FileWriter::Close()
{
	if (m_file && m_file != stdout && m_file != stderr)
	{
		std::fclose(m_file);
		m_file = nullptr;
	}
}


void
FileWriter::Reset(std::string_view name, file_type_e type)
{
	constexpr std::size_t BUF_SIZE = 256;
	char buffer[BUF_SIZE];
	StringFormer err_fmt {buffer, BUF_SIZE};

	char const* mode = nullptr;
	switch (type)
	{
	case file_type_e::TEXT:   mode = "w"; break;
	case file_type_e::BINARY: mode = "wb"; break;
	}

	Close(); 
	
	if (name.empty())
	{
		throw std::runtime_error("File's name is empty.");
	}
	if (name == "stdout")
	{
		m_file = stdout;
		return;
	}
	else if (name == "stderr")
	{
		m_file = stderr;
		return;
	}

	m_file = std::fopen(name.data(), mode);
	if (not m_file)
	{
		throw std::runtime_error(
			err_fmt("Can not create file [%s]: %s",
			         name.data(), std::strerror(errno)).c_str());
	}
}


void
FileWriter::SetBufferSize(std::size_t size)
{
	int buf_mode = size == 0 ? _IONBF : _IOFBF;
	std::setvbuf(m_file, nullptr, buf_mode, size);
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

	std::size_t written = std::fwrite(data, sizeof(*data), size, m_file);
	if (written != size)
	{
		constexpr std::size_t BUF_SIZE = 256;
		char buffer[BUF_SIZE];

		throw std::runtime_error(StringFormer(buffer, BUF_SIZE)
			("Unexpexted was written less bytes: exp=%zu, act=%zu. Error: %s",
		     size, written, std::strerror(errno)).c_str());
	}
	if (m_need_flush) { std::fflush(m_file); }
}


void
FileWriter::Write(char ch)
{
	std::fputc(ch, m_file);
	if (m_need_flush) { std::fflush(m_file); }
}

