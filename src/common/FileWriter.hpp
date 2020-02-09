#pragma once

#include <string_view>
#include <iosfwd>
#include <memory>

#include <cstdint>



class FileWriter
{
public:
	static constexpr std::string_view DEFAULT_NAME {"stderr"};
	enum class file_type_e : uint8_t { TEXT, BINARY };

	FileWriter(FileWriter const&)             = delete;
	FileWriter& operator= (FileWriter const&) = delete;

	FileWriter(char const* name, file_type_e type = file_type_e::BINARY);
	FileWriter(FileWriter&&)             = default;
	FileWriter& operator= (FileWriter&&) = default;
	~FileWriter();

	void Reset(char const* name, file_type_e type = file_type_e::BINARY);

	void Write(char ch);
	void Write(std::string_view);
	void Write(uint8_t const* data, std::size_t size);
	void Write(char const* data, std::size_t size);
	void Write(void const* data, std::size_t elem_size, std::size_t elem_count);

	char const* GetName() const noexcept { return m_name; }
	void SetFlushing(bool is_need)       { m_need_flush = is_need; }
	bool IsFlushing() const              { return m_need_flush; }
	void FlushIfNeeded();
	void SetBufferSize(char* buff, std::size_t buff_size);

private:
	void DetachStandardStreams() noexcept;

	char const*                   m_name       = nullptr;
	std::unique_ptr<std::ostream> m_out;
	bool                          m_need_flush = false;
};

