#include "log.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>

std::shared_ptr<spdlog::logger> Log::logger;

void Log::create()
{
	logger = spdlog::stdout_color_mt("Engine Core");

}

void Log::terminate()
{
	spdlog::drop_all();
	spdlog::shutdown();
}
