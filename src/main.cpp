#include "common/Config.hpp"



int main(int argc, char** argv)
{
	if (not Config::RefInstance().ParseArgs(argc, argv)) { return 1; }
	
	return 0;
}

