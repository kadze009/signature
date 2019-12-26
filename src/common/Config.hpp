#pragma once

#include <limits>
#include <string>
#include <string_view>

#include <cstdint>

#include "Singletone.hpp"
#include "Logger.hpp"
#include "StringFormer.hpp"



class Config : public Singletone<Config>
{
public:
	using log_lvl_e = Logger::log_level_e;

	static constexpr std::size_t DEFAULT_BLOCK_SIZE    = 1024; // KBytes
	static constexpr std::size_t DEFAULT_THREAD_NUM    = std::numeric_limits<std::size_t>::max();
	static constexpr std::string_view DEFAULT_LOGFILE  = "stdout";

	log_lvl_e GetActualLogLevel() const    { return m_actLogLvl; }
	void PrintUsage() const;
	void PrintHelp() const;
	void PrintVersion() const;

	bool ParseArgs(int, char**);
	char const* toString() const;

private:
	void ParseVerbose(std::string_view, StringFormer&);
	void ParseBlockSize(std::string_view, StringFormer&);
	void ParseOption(std::string_view, StringFormer&);


	//log_lvl_e   m_actLogLvl    = log_lvl_e::WARNING;  //TODO: uncomment
	log_lvl_e   m_actLogLvl    = log_lvl_e::DEBUG;
	std::size_t m_block_size   = DEFAULT_BLOCK_SIZE; // KBytes
	std::size_t m_num_threads  = DEFAULT_THREAD_NUM; // As many as possible
	std::string m_logfile      { DEFAULT_LOGFILE };
	std::string m_input_file;
	std::string m_output_file;
	// algo_e m_algo = algo_e::CRC32;
};

