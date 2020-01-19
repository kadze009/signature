#pragma once


#include "IHasher.hpp"


namespace algo {

class InitMd5HashStrategy : public InitHashStrategy
{
public:
	InitMd5HashStrategy()
		: InitHashStrategy(hash_type_e::MD5)
	{}

private:
};




// Was taken from https://tools.ietf.org/html/rfc1321
class HasherMd5 : public IHasher
{
public:
	HasherMd5(HasherMd5&&)                  = delete;
	HasherMd5(HasherMd5 const&)             = delete;
	HasherMd5& operator= (HasherMd5&&)      = delete;
	HasherMd5& operator= (HasherMd5 const&) = delete;

	HasherMd5()  { Init(); }
	~HasherMd5() = default;

	// IHasher
	int Init(InitHashStrategy const&) override;
	int Update(uint8_t const*, std::size_t) override;
	int Finish(uint8_t*) override;
	std::size_t ResultSize() const override;

private:
	void Init();

	/* MD5 context. See realisation in RFC1321*/
	struct Context
	{
		uint32_t state[4];      // state (ABCD)
		uint32_t count[2];      // number of bits, modulo 2^64 (lsb first)
		uint8_t  buffer[64];    // input buffer
	};

	Context m_ctx;
	bool    m_was_finished = false;
};

} // namespace algo

