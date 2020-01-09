#include "Config.hpp"

#include <vector>
#include <exception>

#include <cstdio>

#include "BuildVersion.hpp"
#include "algos/HasherMd5.hpp"
#include "algos/HasherCrc32.hpp"



Config::Config()
	: m_startDateTime(start_clock_t::now())
	, m_startMoment(clock_t::now())
{
	m_init_algo.reset(new algo::InitMd5HashStrategy);
}


void
Config::PrintUsage() const
{
    fprintf(stderr,
"Usage:\n"
"    " APP_NAME " [KEYS]... INPUT_FILE OUTPUT_FILE\n");
}


void
Config::PrintHelp() const
{
    PrintUsage();
    fprintf(stderr,
"\nDESCRIPTION\n"
"    " APP_DESCRIPTION "\n\n"

R"(KEYS
    -h, --help
        this message

    --version
        Print version

    -v, --verbose LEVEL (default: WRN)
        Print information verbosly. Possible values: DBG, INF, WRN, ERR.

    -b, --block-size BLOCK_SIZE (default: 1M)
        the size of block on which input file is split. The value supports
        suffixes: K=KiloByte, M=MegaByte, G=GigaByte. A number without suffix
        is KiloBytes.

    -o, --option OPTION
        set special option:
        * sign_algo=[crc32,md5] (default: md5)
            signature algorithm
        * threads=NUM (default: as many threads as possible)
            number of created threads
        * log_file=<file path> (default: stdout)
            the log file path

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
    fprintf(stderr,
	        APP_NAME ": version %d.%d (patch %06d)\n",
	        BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH);
}


namespace {

enum class key_type_e
{
	UNKNOWN,
	HELP,
	VERSION,
	VERBOSE,
	BLOCK_SIZE,
	OPTION,
};


struct KeyArg
{
	static constexpr char NO_SHORT = '\0';

	std::string_view long_name;
	char             short_name = NO_SHORT;
	key_type_e       key_type   = key_type_e::UNKNOWN;
	bool             with_param = false;
};


std::vector<KeyArg> const g_OptArgs = {
	{}, // Unknown key
	{ "help",       'h',              key_type_e::HELP             },
	{ "version",    KeyArg::NO_SHORT, key_type_e::VERSION          },
	{ "verbose",    'v',              key_type_e::VERBOSE,    true },
	{ "block-size", 'b',              key_type_e::BLOCK_SIZE, true },
	{ "option",     'o',              key_type_e::OPTION,     true },
};


bool
IsKeyArg(std::string_view key)
{
	return key.size() > 1 && key[0] == '-';
}


KeyArg const*
FindKeyArg(std::string_view arg_str)
{
	if (not IsKeyArg(arg_str)) { return nullptr; }

	KeyArg const* res = &g_OptArgs[0];
	if (arg_str[1] == '-')
	{
		// detect long key, look up it
		auto act_name = arg_str.substr(2);
		for (auto const& preset_key : g_OptArgs)
		{
			if (preset_key.key_type == key_type_e::UNKNOWN)
			{
				continue;
			}
			else if (act_name == preset_key.long_name)
			{
				res = &preset_key;
				break;
			}
		}
	}
	else
	{
		// detect short key, look up it
		for (auto const& preset_key : g_OptArgs)
		{
			if (preset_key.key_type == key_type_e::UNKNOWN)
			{
				continue;
			}
			else if (   preset_key.short_name != KeyArg::NO_SHORT
			         && preset_key.short_name == arg_str[1]
					 && arg_str.size() == 2)
			{
				res = &preset_key;
				break;
			}
		}
	}
	return res;
}

} // namespace



bool
Config::ParseArgs(int argc, char** argv)
{
	constexpr std::size_t ERROR_BUFFER_SIZE = 512;
	std::vector<char> err_buf(ERROR_BUFFER_SIZE, '\0');
	StringFormer err_fmt {err_buf.data(), err_buf.size()};

	int i_arg = 1;
	try
	{
		for (; i_arg < argc; ++i_arg)
		{
			char const* cur_arg = argv[i_arg];
			KeyArg const* key_arg = FindKeyArg(cur_arg);

			// Process with non key like arguments
			if (not key_arg)
			{
				// detect not a key. Possible it is a name of input/output file
				if      (m_input_file.empty())  { m_input_file.assign(cur_arg); }
				else if (m_output_file.empty()) { m_output_file.assign(cur_arg); }
				else
				{
					throw std::invalid_argument(err_fmt(
						"unknown [%s]: input[%s] and output[%s] files were set.",
						 cur_arg, m_input_file.c_str(), m_output_file.c_str()).c_str());
				}
				continue;
			}

			// Detect key. Try get value of key if needed
			char const* key_value = nullptr;
			if (key_arg->with_param)
			{
				char const* error = nullptr;
				do
				{
					if (i_arg + 1 == argc)
					{
						error = "unexpected end of arguments` list";
						break;
					}
					key_value = argv[++i_arg];
					if (IsKeyArg(key_value))
					{
						--i_arg;
						error = "unxexpected detection of key argument";
						break;
					}
				}
				while (false);
				if (error)
				{
					throw std::invalid_argument(err_fmt(
						"%s: key[%s] requires value.\n",
						error, cur_arg).c_str());
				}
			}

			// Process with looked up key
			switch (key_arg->key_type)
			{
			case key_type_e::UNKNOWN:
				{
					throw std::invalid_argument(err_fmt("unknown key[%s]", cur_arg).c_str());
				}
				break;

			case key_type_e::HELP:
				{
					PrintHelp();
					return false;
				}
				break;

			case key_type_e::VERSION:
				{
					PrintVersion();
					return false;
				}
				break;

			case key_type_e::VERBOSE:    ParseVerbose(key_value, err_fmt); break;
			case key_type_e::BLOCK_SIZE: ParseBlockSize(key_value, err_fmt); break;
			case key_type_e::OPTION:     ParseOption(key_value, err_fmt); break;
			}
		}
	}
	catch (std::exception const& ex)
	{
		LOG_E("%s: caught exception on argument #%d: %s",
		      __FUNCTION__, i_arg, ex.what());
		PrintHelp();
		return false;
	}

	if     (m_input_file.empty() or m_output_file.empty())
	{
		err_fmt.append("%s: ", __FUNCTION__);
		if (m_input_file.empty())
		{
			err_fmt("INPUT file was not set", __FUNCTION__);
		}
		if (m_output_file.empty())
		{
			if (m_input_file.empty()) { err_fmt(" and "); }
			err_fmt("OUTPUT file was not set");
		}
	}
	else if (m_input_file == m_output_file)
	{
		err_fmt("%s: INPUT and OUTPUT files must have different names. Detect the same names '%s'",
		        __FUNCTION__, m_input_file.c_str());
	}

	if (not err_fmt.empty())
	{
		LOG_E("%s", err_fmt.c_str());
		PrintHelp();
		return false;
	}
	return true;
}


void
Config::ParseVerbose(std::string_view value, StringFormer& err_fmt)
{

}


void
Config::ParseBlockSize(std::string_view value, StringFormer& err_fmt)
{

}


void
Config::ParseOption(std::string_view value, StringFormer& err_fmt)
{

}


char const*
Config::toString() const
{
    return nullptr;
}

