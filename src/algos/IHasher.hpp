#pragma once

#include <cstdint>

#include "HasherFactory.hpp"

namespace algo {

struct IHasher
{
	virtual int Init(InitHashStrategy const&)       = 0;
	virtual int Update(uint8_t const*, std::size_t) = 0;
	virtual int Finish(uint8_t*, std::size_t&)      = 0;
};

} // namespace algo

