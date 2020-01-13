#include "HasherCrc32.hpp"



namespace algo {

int
HasherCrc32::Init(InitHashStrategy const&)
{
	//TODO: implement
	return 0;
}


int
HasherCrc32::Update(uint8_t const*, std::size_t)
{
	//TODO: implement
	return 0;
}



int
HasherCrc32::Finish(uint8_t*, std::size_t&)
{
	//TODO: implement
	return 0;
}


std::size_t
HasherCrc32::ResultSize() const
{
	return 4; //32 bits
}

} // namespace algo

