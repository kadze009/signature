#pragma once

#include <limits>
#include <string>
#include <chrono>
#include <string_view>
#include <memory>

#include <cstdint>

#include "common/Singletone.hpp"
#include "common/Logger.hpp"
#include "algo/HasherFactory.hpp"



class Config : public Singletone<Config>
{
public:
	using start_clock_t = std::chrono::system_clock;
	using start_tp_t    = decltype(start_clock_t::now());
	using clock_t       = std::chrono::steady_clock;
	using clock_tp_t    = decltype(clock_t::now());
	using log_lvl_e     = Logger::log_level_e;
	using init_algo_t   = std::unique_ptr<algo::InitHashStrategy>;

	struct Default_s
	{
		//NOTE: AMAP = As Much As Possible
		static constexpr size_t      AMAP_THREAD_NUM    = std::numeric_limits<std::size_t>::max();
		static constexpr uintmax_t   BLOCK_SIZE_KB      = 1024;
		static constexpr size_t      LOG_MSG_BATCH_SIZE = 100;
		static constexpr char const* LOGFILE            = "stdout";
		static constexpr size_t      READ_BUF_SIZE      = 4096;
		static constexpr uint8_t     BLOCK_FILLER_BYTE  = 0;
		static constexpr size_t      THREAD_NUM_WHEN_HWCORE_IS_0 = 2;
	};

	struct BuildVersion_s
	{
		uint32_t major;
		uint32_t minor;
		uint32_t patch;
	};

public:
	static BuildVersion_s const& GetBuildVersion() noexcept { return m_buildVersion; }
	static void PrintUsage() noexcept;
	static void PrintHelp() noexcept;
	static void PrintVersion() noexcept;


	Config();

	bool ParseArgs(int, char**) noexcept;
	char const* toString() const noexcept;

	auto GetDurationSinceStart() const noexcept        { return clock_t::now() - m_startMoment; }
	start_tp_t const GetStartDateTime() const noexcept { return m_startDateTime; }

	init_algo_t const& GetInitAlgo() const noexcept    { return m_initAlgo; }
	log_lvl_e GetActualLogLevel() const noexcept       { return m_actLogLvl; }
	std::string const& GetLogfile() const noexcept     { return m_logfile; }
	std::string const& GetInputFile() const noexcept   { return m_inputFile; }
	std::string const& GetOutputFile() const noexcept  { return m_outputFile; }
	uintmax_t GetBlockSizeKB() const noexcept          { return m_blockSizeKB; }
	uintmax_t GetInputFileSize() const noexcept        { return m_inputFileSize; }
	size_t GetBatchSizeOfLogMessages() const noexcept  { return m_logMsgBatchSize; }
	size_t GetThreadsNum() const noexcept              { return m_numThreads; }
	size_t GetReadBufferSize() const noexcept          { return m_readBufSize; }
	uint8_t GetBlockFiller() const noexcept            { return m_blockFiller; }

	// NOTE: uint64_t for determinating byte size of block number which will
	//       be recorded
	void SetLastBlockNum(std::uint64_t v) noexcept     { m_lastBlockNum = v; }
	std::uint64_t GetLastBlockNum() const noexcept     { return m_lastBlockNum; }

	void SetBlocksShift(std::uint64_t v) noexcept      { m_blocksShift = v; }
	std::uint64_t GetBlocksShift() const noexcept      { return m_blocksShift; }

	void SetFileBytesShift(std::uintmax_t v) noexcept  { m_fileBytesShift = v; }
	std::uintmax_t GetFileBytesShift() const noexcept  { return m_fileBytesShift; }

private:
	void ParseVerbose(char const*);
	void ParseBlockSize(char const*);
	void ParseOption(char const*);

	void FinalCheck_InputOutputFiles();
	void FinalCheck_BlockSize();
	void FinalCheck_ThreadNums();
	void FinalCheck_Algo();
	void FinalCheck_LogSettings();

private:
	static BuildVersion_s const m_buildVersion;

private:
	start_tp_t const m_startDateTime;
	clock_tp_t const m_startMoment;

	uintmax_t      m_blockSizeKB     = Default_s::BLOCK_SIZE_KB;
	size_t         m_logMsgBatchSize = Default_s::LOG_MSG_BATCH_SIZE;
	size_t         m_numThreads      = Default_s::AMAP_THREAD_NUM; // AMAP = As Much As Possible
	std::string    m_logfile         { Default_s::LOGFILE };
	std::string    m_outputFile;
	std::string    m_inputFile;
	uintmax_t      m_inputFileSize   = 0;
	init_algo_t    m_initAlgo;
	uint64_t       m_blocksShift     = 0; // will be set by WorkerManager
	uint64_t       m_lastBlockNum    = 0; // will be set by WorkerManager
	uintmax_t      m_fileBytesShift  = 0; // will be set by WorkerManager
	size_t         m_readBufSize     = Default_s::READ_BUF_SIZE;     //TODO: add for configuring
	uint8_t        m_blockFiller     = Default_s::BLOCK_FILLER_BYTE; //TODO: add for configuring
	log_lvl_e      m_actLogLvl       = log_lvl_e::WARNING;
};
