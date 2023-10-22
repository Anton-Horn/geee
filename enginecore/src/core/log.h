#pragma once
#include <spdlog/spdlog.h>

#include "core/core.h"

//Log Defines

#ifdef DEBUG 
#define EC_LOG(...) Log::s_baseLogger->trace(__VA_ARGS__)
#define EC_INFO(...) Log::s_baseLogger->info(__VA_ARGS__)
#define EC_WARN(...) Log::s_baseLogger->warn(__VA_ARGS__)
#define EC_ERROR(...) Log::s_baseLogger->error(__VA_ARGS__)
#else
#define EC_LOG(...) 
#define EC_WARN(...) 
#define EC_INFO(...)
#define EC_ERROR(...)
#endif

namespace spdlog {
	class logger;
}

class Log {

public:

	static std::shared_ptr<spdlog::logger> s_baseLogger;

	static void create();
	static void terminate();


};

