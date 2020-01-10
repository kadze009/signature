#include "HasherFactory.hpp"

#include "common/Logger.hpp"

#include "IHasher.hpp"
#include "HasherMd5.hpp"
#include "HasherCrc32.hpp"



char const*
toString(algo::hash_type_e v)
{
	switch (v)
	{
	case algo::hash_type_e::UNKNOWN: return "UNKNOWN";
	case algo::hash_type_e::MD5:     return "MD5";
	case algo::hash_type_e::CRC32:   return "CRC32";
	}
	return "?";
}


namespace algo {

// static
HasherFactory::hasher_t
HasherFactory::Create(InitHashStrategy& strategy) 
{
	hasher_t hasher;
	switch (strategy.GetType())
	{
	case hash_type_e::MD5:            hasher.reset(new HasherMd5); break;
	case hash_type_e::CRC32:          hasher.reset(new HasherCrc32); break;

	case hash_type_e::UNKNOWN:
		THROW_ERROR("%s", "Select UNKNOWN hasher");
	}
	hasher->Init(strategy);
	return hasher;
}

} // namespace algo

