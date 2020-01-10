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

	static constexpr std::size_t DEFAULT_BLOCK_SIZE    = 1024; // KBytes
	static constexpr std::size_t DEFAULT_THREAD_NUM    = std::numeric_limits<std::size_t>::max();
	static constexpr std::string_view DEFAULT_LOGFILE  = "stdout";

	Config();

	log_lvl_e GetActualLogLevel() const    { return m_actLogLvl; }
	void PrintUsage() const;
	void PrintHelp() const;
	void PrintVersion() const;

	bool ParseArgs(int, char**);
	char const* toString() const;

	auto GetDurationSinceStart() const     { return clock_t::now() - m_startMoment; }
	auto const GetStartDateTime() const    { return m_startDateTime; }

private:
	void ParseVerbose(std::string_view);
	void ParseBlockSize(std::string_view);
	void ParseOption(std::string_view);

	void FinalCheck_InputOutputFiles();
	void FinalCheck_BlockSize();
	void FinalCheck_ThreadNums();
	void FinalCheck_Algo();


	decltype(start_clock_t::now()) const m_startDateTime;
	decltype(clock_t::now()) const       m_startMoment;

	//log_lvl_e   m_actLogLvl    = log_lvl_e::WARNING;  //TODO: uncomment
	log_lvl_e   m_actLogLvl    = log_lvl_e::DEBUG;
	std::size_t m_block_size   = DEFAULT_BLOCK_SIZE; // KBytes
	std::size_t m_num_threads  = DEFAULT_THREAD_NUM; // As many as possible
	std::string m_logfile      { DEFAULT_LOGFILE };
	std::string m_input_file;
	std::string m_output_file;

	using init_algo_t = std::unique_ptr<algo::InitHashStrategy>;
	init_algo_t m_init_algo;
};

