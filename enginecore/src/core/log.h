#pragma once

#include "core/core.h"

//Log Defines

#ifdef DEBUG 
#define logInfo(x) Log::info(x)
#define logWarn(x) Log::warn(x);
#define logError(x) Log::Error(x);

#else
#define logInfo(x) 
#define logWarn(x) 
#define logError(x) 
#endif

namespace spdlog {
	class logger;
}
namespace ec {

class Log {

public:

	static void info(const std::string& m);
	static void warn(const std::string& m);
	static void error(const std::string& m);

	static void create();
	static void terminate();

private:

	static std::shared_ptr<spdlog::logger> s_baseLogger;

};

}