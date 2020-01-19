#pragma once

#include <array>

#include <cstdint>

#include "HasherFactory.hpp"

namespace algo {

struct IHasher
{
	virtual int Init(InitHashStrategy const&)       = 0;
	virtual int Update(uint8_t const*, std::size_t) = 0;
	virtual int Finish(uint8_t*)                    = 0;
	virtual std::size_t ResultSize() const          = 0;

	int Update(uint8_t ch, std::size_t num_repeats = 1)
	{
		std::array<uint8_t, 256> filler;
		filler.fill(ch);

		std::size_t remains = num_repeats;
		while (remains > filler.size())
		{
			Update(filler.data(), filler.size());
			remains -= filler.size();
		}
		if (remains > 0)
		{
			Update(filler.data(), remains);
		}
		return 0;
	}
};

} // namespace algo

