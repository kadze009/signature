#include <iostream>

#include "common/Config.hpp"



int
main(int argc, char** argv)
{
	std::ios::sync_with_stdio(false);
	if (not Config::RefInstance().ParseArgs(argc, argv)) { return 1; }
	return 0;
}

