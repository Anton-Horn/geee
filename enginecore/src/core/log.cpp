#include "Log.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace ec {

	std::shared_ptr<spdlog::logger> Log::s_baseLogger;

	void Log::info(const std::string& m)
	{
		Log::s_baseLogger->trace(m);
	}

	void Log::warn(const std::string& m)
	{
		Log::s_baseLogger->warn(m);
	}

	void Log::error(const std::string& m)
	{
		Log::s_baseLogger->error(m);
	}

	void Log::create()
	{

		std::vector<spdlog::sink_ptr> sinks;
		sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		sinks[0]->set_pattern("%^[%T] %v%$");

		sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.log"));
		sinks[1]->set_pattern("%^[%T] %v%$");

		s_baseLogger = std::make_shared<spdlog::logger>("ec", sinks.begin(), sinks.end());
		spdlog::register_logger(s_baseLogger);
		s_baseLogger->set_level(spdlog::level::trace);
		s_baseLogger->flush_on(spdlog::level::trace);


	}

	void Log::terminate()
	{
		spdlog::shutdown();
	}
}