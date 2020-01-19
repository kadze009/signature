#include "Config.hpp"

#include <array>
#include <exception>
#include <thread>
#include <charconv>
#include <system_error>
#include <filesystem>

#include <cstdio>
#include <cstdarg>

#include "common/StringFormer.hpp"
#include "BuildVersion.hpp"
#include "algo/HasherMd5.hpp"
#include "algo/HasherCrc32.hpp"



Config::BuildVersion_s const Config::m_buildVersion
{
	  .major = BUILD_VERSION_MAJOR
	, .minor = BUILD_VERSION_MINOR
	, .patch = BUILD_VERSION_PATCH
};


Config::Config()
	: m_startDateTime(start_clock_t::now())
	, m_startMoment(clock_t::now())
{
	m_initAlgo.reset(new algo::InitMd5HashStrategy);
}


void
Config::PrintUsage() const
{
	fprintf(stderr,
"Usage:\n"
"    " APP_NAME " [KEYS]... <INPUT_FILE> <OUTPUT_FILE>\n");
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
        * threads=NUM (default: as many threads as available)
            integer number of threads for processing
            (at least 2: one -- manager, others -- workers)
        * log_file=<file path> (default: stdout)
            the log file path
        * log_batch_size=<number> (default: 100)
            the integer number of log messages for writing in async mode during
            multithread execution

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
	        GetBuildVersion().major,
	        GetBuildVersion().minor,
	        GetBuildVersion().patch
	       );
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


std::array<KeyArg, 6> const g_OptArgs =
{{
	// NOTE: double braces need for successful compilation with GCC.
	// https://stackoverflow.com/questions/8192185/using-stdarray-with-initialization-lists

	{}, // Unknown key
	{ "help",       'h',              key_type_e::HELP             },
	{ "version",    KeyArg::NO_SHORT, key_type_e::VERSION          },
	{ "verbose",    'v',              key_type_e::VERBOSE,    true },
	{ "block-size", 'b',              key_type_e::BLOCK_SIZE, true },
	{ "option",     'o',              key_type_e::OPTION,     true },
}};


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


void
THROW_INVALID_ARGUMENT(char const* fmt, ...)
{
	constexpr std::size_t ERROR_BUFFER_SIZE = 512;
	std::array<char, ERROR_BUFFER_SIZE> err_buf;
	err_buf.fill('\0');
	StringFormer err_fmt {err_buf.data(), err_buf.size()};

	va_list args;
	va_start(args, fmt);
	err_fmt.append(fmt, args);
	va_end(args);
	throw std::invalid_argument(err_fmt.c_str());
}

} // namespace



bool
Config::ParseArgs(int argc, char** argv)
{
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
				if      (m_inputFile.empty())
				{
					using std::filesystem::file_size;
					m_inputFile.assign(cur_arg);

					std::error_code ec;
					m_inputFileSize = file_size(m_inputFile, ec);
					if (ec)
					{
						THROW_INVALID_ARGUMENT("can not get size of input file "
							"[%s]: %s",
							m_inputFile.c_str(), ec.message().c_str());
					}
				}
				else if (m_outputFile.empty())
				{
					m_outputFile.assign(cur_arg);
				}
				else
				{
					THROW_INVALID_ARGUMENT(
						"unknown [%s]: input[%s] and output[%s] files were set.",
						cur_arg, m_inputFile.c_str(), m_outputFile.c_str());
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
					THROW_INVALID_ARGUMENT(
						"%s: key[%s] requires value.\n",
						error, cur_arg);
				}
			}

			// Process with looked up key
			switch (key_arg->key_type)
			{
			case key_type_e::UNKNOWN:
				{
					THROW_INVALID_ARGUMENT("unknown key[%s]", cur_arg);
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

			case key_type_e::VERBOSE:    ParseVerbose(key_value); break;
			case key_type_e::BLOCK_SIZE: ParseBlockSize(key_value); break;
			case key_type_e::OPTION:     ParseOption(key_value); break;
			}
		}

		FinalCheck_InputOutputFiles();
		FinalCheck_BlockSize();
		FinalCheck_ThreadNums();
		FinalCheck_Algo();
		FinalCheck_LogSettings();
	}
	catch (std::invalid_argument const& ex)
	{
		LOG_E("%s: invalid argument #%d: %s",
		      __FUNCTION__, i_arg, ex.what());
		PrintHelp();
		return false;
	}
	catch (std::exception const& ex)
	{
		PrintHelp();
		return false;
	}

	return true;
}


void
Config::ParseVerbose(std::string_view value)
{
	if      (value == "DBG" or value == "DEBUG")
	{
		m_actLogLvl = log_lvl_e::DEBUG;
	}
	else if (value == "INF" or value == "INFO")
	{
		m_actLogLvl = log_lvl_e::INFO;
	}
	else if (value == "WRN" or value == "WARNING")
	{
		m_actLogLvl = log_lvl_e::WARNING;
	}
	else if (value == "ERR" or value == "ERROR")
	{
		m_actLogLvl = log_lvl_e::ERROR;
	}
	else
	{
		THROW_INVALID_ARGUMENT("unknown verbose level [%s]", value.data());
	}
}


void
Config::ParseBlockSize(std::string_view value)
{
	char const* begin = value.data();
	char const* end   = value.data() + value.size();
	auto res = std::from_chars(begin, end, m_blockSizeKB);
	if (res.ec != std::errc())
	{
		THROW_INVALID_ARGUMENT("can not parse block size [%s]: %s",
			value.data(), std::make_error_code(res.ec).message().c_str());
	}

	if (res.ptr != end)
	{
		if (res.ptr + 1 != end)
		{
			THROW_INVALID_ARGUMENT(
				"too long suffix [%s] of block size's number [%zu]. "
				"Available only K, M and G.",
				res.ptr, m_blockSizeKB);
		}

		switch (res.ptr[0])
		{
		case 'k':
		case 'K': break;
		case 'm':
		case 'M': m_blockSizeKB *= 1024; break;
		case 'g':
		case 'G': m_blockSizeKB *= 1024*1024; break;
		default:
			THROW_INVALID_ARGUMENT(
				"unknown number modificator [%c] for found block size [%zu]. "
				"Available only K, M and G.",
				res.ptr[0], m_blockSizeKB);
		}
	}
}


void
Config::ParseOption(std::string_view value)
{
	constexpr char delimeter = '=';
	auto delim_pos = value.find_first_of(delimeter);
	if (delim_pos == std::string_view::npos)
	{
		THROW_INVALID_ARGUMENT("detect invalid format for option [%s]", value.data());
	}
	auto opt_k = value.substr(0, delim_pos);
	auto opt_v = value.substr(delim_pos+1, value.size() - delim_pos - 1);
	if (opt_v.empty())
	{
		THROW_INVALID_ARGUMENT("empty value for option [%.*s]",
			(int)opt_k.size(), opt_k.data());
	}

	if      (opt_k == "sign_algo")
	{
		if      (opt_v == "md5")
		{
			m_initAlgo.reset(new algo::InitMd5HashStrategy);
		}
		else if (opt_v == "crc32")
		{
			m_initAlgo.reset(new algo::InitCrc32HashStrategy);
		}
		else
		{
			THROW_INVALID_ARGUMENT("unknown signature algorithm [%.*s]",
				(int)opt_v.size(), opt_v.data());
		}
	}

	else if (opt_k == "threads")
	{
		auto res = std::from_chars(opt_v.data(), opt_v.data() + opt_v.size(),
		                           m_numThreads);
		if (res.ec != std::errc())
		{
			THROW_INVALID_ARGUMENT("can not parse threads number [%.*s]: %s",
				(int)opt_v.size(), opt_v.data(),
				std::make_error_code(res.ec).message().c_str());
		}
	}

	else if (opt_k == "log_file")
	{
		// WorkerManager changes the mode and the logfile of LoggerManager
		// on start.
		m_logfile.assign(opt_v);
	}

	else if (opt_k == "log_batch_size")
	{
		auto res = std::from_chars(opt_v.data(), opt_v.data() + opt_v.size(),
		                           m_logMsgBatchSize);
		if (res.ec != std::errc())
		{
			THROW_INVALID_ARGUMENT("can not parse log messages batch size [%.*s]: %s",
				(int)opt_v.size(), opt_v.data(),
				std::make_error_code(res.ec).message().c_str());
		}
	}

	else
	{
		THROW_INVALID_ARGUMENT("unknown option [%.*s]",
			(int)opt_k.size(), opt_k.data());
	}
}


void
Config::FinalCheck_InputOutputFiles()
{
	if (m_inputFile.empty() and m_outputFile.empty())
	{
		THROW_ERROR("%s: INPUT and OUTPUT files are unknown.", __FUNCTION__);
	}
	if (m_inputFile.empty())
	{
		THROW_ERROR("%s: unknown INPUT file.", __FUNCTION__);
	}
	if (m_outputFile.empty())
	{
		THROW_ERROR("%s: unknown OUTPUT file.", __FUNCTION__);
	}

	if (m_inputFile == m_outputFile)
	{
		THROW_ERROR(
			"%s: INPUT and OUTPUT files must have different names. "
			"Detect the same names '%s'",
			__FUNCTION__, m_inputFile.c_str());
	}
}


void
Config::FinalCheck_BlockSize()
{
	if (m_blockSizeKB == 0)
	{
		THROW_ERROR("%s: the block size MUST BE more then 0", __FUNCTION__);
	}
}


void
Config::FinalCheck_ThreadNums()
{
	if (m_numThreads == 0)
	{
		THROW_ERROR("%s: the number of threads MUST BE more then 0", __FUNCTION__);
	}

	std::size_t hw_cores = std::thread::hardware_concurrency();

	if (m_numThreads == DEFAULT_THREAD_NUM)
	{
		m_numThreads = hw_cores;
		//if (m_numThreads > 7) { --m_numThreads; } // stay one core for OS's needes
	}
	else if (m_numThreads > 2*hw_cores)
	{
		THROW_ERROR(
			"%s: the invalid number of expected threads [%zu]: "
			"available maximum 2*%zu threads.",
			__FUNCTION__, m_numThreads, hw_cores);
	}

	if (m_numThreads < 2)
	{
		THROW_ERROR(
			"%s: the invalid number of expected threads [%zu]: "
			"expected MUST BE at least 2 threads "
			"(one -- manager, others -- workers)",
			__FUNCTION__, m_numThreads);
	}
}


void
Config::FinalCheck_Algo()
{
	if (not m_initAlgo) { THROW_ERROR("%s: algorithm was not select.", __FUNCTION__); }
}


void
Config::FinalCheck_LogSettings()
{
	if (m_logMsgBatchSize == 0)
	{
		THROW_ERROR(
			"%s: the log messages batch size MUST be more then 0",
			__FUNCTION__);
	}
}


char const*
Config::toString() const
{
	constexpr std::size_t MAX_SIZE = 368;
	static std::array<char, MAX_SIZE> buffer;

	buffer.fill('\0');
	StringFormer str(buffer.data(), buffer.size());
	str(R"({
	LOG FILE        = %s
	LOG LEVEL       = %s
	LOG BATCH SIZE  = %zu
	ALGORITHM       = %s
	NUMBER THREADS  = %zu
	INPUT FILE      = %s
	INPUT FILE SIZE = %zu
	OUTPUT FILE     = %s
	BLOCK SIZE (KB) = %zu
	LAST BLOCK NUM  = %zu
})",
		  m_logfile.c_str()
		, ::toString(m_actLogLvl)
		, m_logMsgBatchSize
		, ::toString(m_initAlgo->GetType())
		, m_numThreads
		, m_inputFile.c_str()
		, m_inputFileSize
		, m_outputFile.c_str()
		, m_blockSizeKB
		, m_lastBlockNum
		);

    return str.c_str();
}

