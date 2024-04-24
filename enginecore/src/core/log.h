#pragma once
#include <spdlog/spdlog.h>

#include "core/core.h"

//Log Defines



#ifdef DEBUG 
#define EC_LOG(...) Log::logger->info(__VA_ARGS__)
#define EC_INFO(...) Log::logger->info(__VA_ARGS__)
#define EC_WARN(...) Log::logger->warn(__VA_ARGS__)
#define EC_ERROR(...) Log::logger->error(__VA_ARGS__)
#define EC_WARN_CON(con, ...) if ((con)) Log::logger->warn(__VA_ARGS__)
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

	static std::shared_ptr<spdlog::logger> logger;

	static void create();
	static void terminate();


};

