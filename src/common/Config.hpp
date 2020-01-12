#pragma once

#include <limits>
#include <string>
#include <chrono>
#include <string_view>
#include <memory>

#include <cstdint>

#include "Singletone.hpp"
#include "Logger.hpp"
#include "algos/HasherFactory.hpp"



class Config : public Singletone<Config>
{
public:
	using start_clock_t = std::chrono::system_clock;
	using clock_t       = std::chrono::steady_clock;
	using log_lvl_e     = Logger::log_level_e;
	using init_algo_t   = std::unique_ptr<algo::InitHashStrategy>;

	static constexpr std::size_t DEFAULT_THREAD_NUM =
			std::numeric_limits<std::size_t>::max();
	static constexpr std::uintmax_t DEFAULT_BLOCK_SIZE_KB = 1024;
	static constexpr std::size_t DEFAULT_LOG_MSG_BATCH_SIZE = 100;
	static constexpr std::string_view DEFAULT_LOGFILE  = "stdout";

	Config();

	struct BuildVersion_s
	{
		std::uint32_t major;
		std::uint32_t minor;
		std::uint32_t patch;
	};

	static
	BuildVersion_s const& GetBuildVersion() { return m_buildVersion; }

	void PrintUsage() const;
	void PrintHelp() const;
	void PrintVersion() const;

	bool ParseArgs(int, char**);
	char const* toString() const;

	auto GetDurationSinceStart() const      { return clock_t::now() - m_startMoment; }
	auto const GetStartDateTime() const     { return m_startDateTime; }

	init_algo_t const& GetInitAlgo() const  { return m_initAlgo; }
	log_lvl_e GetActualLogLevel() const     { return m_actLogLvl; }
	std::string_view GetLogfile() const     { return m_logfile; }
	std::string_view GetInputFile() const   { return m_inputFile; }
	std::string_view GetOutputFile() const  { return m_outputFile; }
	std::uintmax_t GetBlockSizeKB() const   { return m_blockSizeKB; }
	std::size_t GetBatchSizeOfLogMessages() const { return m_logMsgBatchSize; }
	bool NeedUseMainThread() const          { return m_withMainThread; }

	std::size_t GetThreadsNum() const       { return m_numThreads; }
	void SetThreadsNum(std::size_t v)       { m_numThreads = v; }

	std::uintmax_t GetBytesShift() const    { return m_bytesShift; }
	void SetBytesShift(std::uintmax_t v)    { m_bytesShift = v; }

private:
	void ParseVerbose(std::string_view);
	void ParseBlockSize(std::string_view);
	void ParseOption(std::string_view);

	void FinalCheck_InputOutputFiles();
	void FinalCheck_BlockSize();
	void FinalCheck_ThreadNums();
	void FinalCheck_Algo();
	void FinalCheck_LogSettings();


	decltype(start_clock_t::now()) const m_startDateTime;
	decltype(clock_t::now()) const       m_startMoment;

	//log_lvl_e    m_actLogLvl       = log_lvl_e::WARNING;  //TODO: uncomment
	log_lvl_e      m_actLogLvl       = log_lvl_e::DEBUG;
	std::uintmax_t m_blockSizeKB     = DEFAULT_BLOCK_SIZE_KB;
	std::uintmax_t m_bytesShift      = 0;
	std::size_t    m_logMsgBatchSize = DEFAULT_LOG_MSG_BATCH_SIZE;
	std::size_t    m_numThreads      = DEFAULT_THREAD_NUM; // As many as possible
	std::string    m_logfile         { DEFAULT_LOGFILE };
	std::string    m_outputFile;
	std::string    m_inputFile;
	std::uintmax_t m_inputFileSize   = 0;
	init_algo_t    m_initAlgo;
	bool           m_withMainThread  = false;

	static BuildVersion_s const m_buildVersion;
};

