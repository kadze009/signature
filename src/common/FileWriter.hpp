#pragma once

#include <string_view>
#include <string>
#include <iosfwd>
#include <memory>

#include <cstdint>



class FileWriter
{
public:
	static constexpr std::string_view DEFAULT_NAME {"stderr"};
	enum class file_type_e : uint8_t { TEXT, BINARY };

	explicit FileWriter(std::string_view name, file_type_e type = file_type_e::BINARY);
	~FileWriter();
	FileWriter(FileWriter const&)             = delete;
	FileWriter& operator= (FileWriter const&) = delete;
	FileWriter(FileWriter&&)                  = default;
	FileWriter& operator= (FileWriter&&)      = default;

	void Reset(std::string_view name, file_type_e type = file_type_e::BINARY);

	void Write(char ch);
	void Write(std::string_view);
	void Write(uint8_t const* data, size_t size);
	void Write(char const* data, size_t size);
	void Write(void const* data, size_t elem_size, size_t elem_count);

	char const* GetName() const noexcept { return m_name.c_str(); }
	void SetFlushing(bool is_need)       { m_need_flush = is_need; }
	bool IsFlushing() const noexcept     { return m_need_flush; }
	void FlushIfNeeded();
	void SetBufferSize(char* buff, size_t buff_size);

private:
	void DetachIfStandardStreams() noexcept;

private:
	using up_ostream_t = std::unique_ptr<std::ostream>;
	std::string  m_name;
	up_ostream_t m_out;
	bool         m_need_flush = false;
};

