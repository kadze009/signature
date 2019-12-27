#include "FileBase.hpp"

#include <stdexcept>

#include <cstring>

#include "StringFormer.hpp"



FileBase::FileBase(std::string_view name, char const* mode /* = nullptr */)
	: m_name(name)
{
	this->Reset(m_name, mode);
}


FileBase::~FileBase()
{
	Close();
}


void
FileBase::Close()
{
	if (   m_file
	    && m_file != stdout
	    && m_file != stderr
	    && m_file != stdin)
	{
		std::fclose(m_file);
		m_file = nullptr;
	}
}


void
FileBase::Reset(std::string_view name, char const* mode /* = nullptr */)
{
	constexpr std::size_t BUF_SIZE = 256;
	char buffer[BUF_SIZE];
	StringFormer err_fmt {buffer, BUF_SIZE};

	Close();

	if (name.empty())
	{
		throw std::runtime_error("File's name is empty.");
	}
	else if (name == "stdout")
	{
		m_file = stdout;
		return;
	}
	else if (name == "stderr")
	{
		m_file = stderr;
		return;
	}
	else if (name == "stdin")
	{
		m_file = stdin;
		return;
	}
	else if (not mode)
	{
		throw std::runtime_error(
			err_fmt("INTERNAL ERROR: select non standard file name [%s] but "
			        "the corresponding mode is empty.", name.data()).c_str());
	}

	m_file = std::fopen(name.data(), mode);
	if (not m_file)
	{
		throw std::runtime_error(
			err_fmt("Can not open file [%s]: %s",
			         name.data(), std::strerror(errno)).c_str());
	}
}



void
FileBase::FlushIfNeeded()
{
	if (IsFlushing())
	{
		std::fflush(m_file);
	}
}


void
FileBase::SetBufferSize(std::size_t size)
{
	int buf_mode = size == 0 ? _IONBF : _IOFBF;
	std::setvbuf(m_file, nullptr, buf_mode, size);
}

