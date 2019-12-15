#include "Config.hpp"

#include <cstdio>

#include "BuildVersion.hpp"



void
Config::PrintUsage() const
{
	printf(
"Usage:\n"
"    " APP_NAME "[KEYS]... INPUT_FILE OUTPUT_FILE\n");
}


void
Config::PrintHelp() const
{
	PrintUsage();
	printf(
"\nDESCRIPTION\n"
"    " APP_DESCRIPTION "\n\n"

R"(KEYS
    -h, --help
        this message

    --version
        Print version

    -v, --verbose
        Print verbose information

    -b, --block-size BLOCK_SIZE (default: 1M)
        the size of block on which input file is split. The value supports
        suffixes: K=KiloByte, M=MegaByte, G=GigaByte. A number without suffix
        is KiloBytes.

    -o OPTION
        set special option:
        * sign_algo=[crc32,md5] (default: crc32)
            signature algorithm
        * threads=NUM (default: as many threads as possible)
            number of created threads
)"
"EXAMPLES\n"
"    " APP_NAME " input.dat output.dat\n"
"    " APP_NAME " -b 32K input.dat output.dat -o threads=5\n"
"    " APP_NAME " --block-size 32K input.dat out.dat -o sign_algo=md5 -o threads=5\n"
	);
}


void
Config::PrintVersion() const
{
	printf(APP_NAME ": version %d.%d (patch %06d)\n",
	       BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH);
}


bool
Config::ParseArgs(int argc, char** argv)
{
	return false;
}


char const*
Config::toString() const
{
	return nullptr;
}

