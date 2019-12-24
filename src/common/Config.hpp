#pragma once

#include "Singletone.hpp"
#include "Logger.hpp"



class Config : public Singletone<Config>
{
public:
	using log_lvl_e = Logger::log_level_e;

	log_lvl_e GetActualLogLevel() const    { return m_actLogLvl; }
	void PrintUsage() const;
	void PrintHelp() const;
	void PrintVersion() const;

	bool ParseArgs(int, char**);
	char const* toString() const;

private:
	log_lvl_e m_actLogLvl = log_lvl_e::DEBUG;
};

