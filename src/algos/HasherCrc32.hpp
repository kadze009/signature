#pragma once


#include "IHasher.hpp"



namespace algo {

class InitCrc32HashStrategy : public InitHashStrategy
{
public:
	InitCrc32HashStrategy()
		: InitHashStrategy(hash_type_e::CRC32)
	{}

private:
};



class HasherCrc32 : public IHasher
{
public:
	HasherCrc32(HasherCrc32&&)                 = delete;
	HasherCrc32(HasherCrc32 const&)            = delete;
	HasherCrc32& operator=(HasherCrc32&&)      = delete;
	HasherCrc32& operator=(HasherCrc32 const&) = delete;

	HasherCrc32() {}
	~HasherCrc32() = default;

	// IHasher
	int Init(InitHashStrategy&) override;
	int Update(uint8_t const*, std::size_t) override;
	int Finish(uint8_t*, std::size_t&) override;

private:
	bool m_was_finished = false;
};

} // namespace algo

