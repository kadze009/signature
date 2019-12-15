#pragma once

#include "Singletone.hpp"


class Config : public Singletone<Config>
{
public:
	void PrintUsage() const;
	void PrintHelp() const;
	void PrintVersion() const;

	bool ParseArgs(int, char**);
	char* toString() const;

private:

};

