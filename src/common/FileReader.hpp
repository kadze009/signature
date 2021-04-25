#pragma once

#include <string_view>
#include <iosfwd>
#include <memory>

#include <cstdint>



class FileReader
{
public:
	static constexpr std::string_view DEFAULT_NAME {"stdin"};

	FileReader(FileReader const&)             = delete;
	FileReader& operator= (FileReader const&) = delete;
	FileReader(FileReader&&)                  = default;
	FileReader& operator= (FileReader&&)      = default;

	FileReader(char const* name);
	~FileReader();

	std::size_t Read(char* buf_data, std::size_t buf_size);
	std::size_t Read(uint8_t* buf_data, std::size_t buf_size);
	void SkipNextBytes(std::uintmax_t offset);

	char const* GetName() const noexcept { return m_name; }

private:
	void DetachStandardStreams() noexcept;

private:
	char const*                      m_name;
	std::unique_ptr<std::istream>    m_in;
};

