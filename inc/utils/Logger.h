#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>

enum class LogLevel
{
	INFO,	 // Random Infos
	WARNING, // Problem with execution
	ERROR	 // Fatal, program must stop
};

class Logger
{
public:
	static void Log(LogLevel level, const std::string &message);

	static void Log(const std::string &message);
	static void LogWarn(const std::string &message);
	static void LogErr(const std::string &message);
};
