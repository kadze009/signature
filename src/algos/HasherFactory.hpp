#pragma once

#include <memory>

#include <cstdint>



namespace algo {

struct IHasher;



enum class hash_type_e
{
	UNKNOWN,
	MD5,
	CRC32,
};



class InitHashStrategy
{
public:
	InitHashStrategy(hash_type_e type = hash_type_e::UNKNOWN)
		: m_type(type)
	{}

	hash_type_e GetType() const { return  m_type; }

private:
	hash_type_e m_type;
};



struct HasherFactory
{
	using hasher_t = std::unique_ptr<IHasher>;
	static hasher_t Create(InitHashStrategy const&);
};

} // namespace algo

