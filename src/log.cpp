#include "log.h"
#include <Windows.h>
#include <cstdio>
#include <mutex>
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/dist_sink.h"

using proxy_dist_sink_mt = spdlog::sinks::dist_sink<std::mutex>;

std::mutex g_logger_initalized_mutex;
bool g_logger_initalized;
std::shared_ptr<proxy_dist_sink_mt> console_proxy = nullptr;
std::shared_ptr<proxy_dist_sink_mt> file_proxy = nullptr;

void Log::InitializeLogger() {
	std::lock_guard<std::mutex> guard(g_logger_initalized_mutex);
	if (g_logger_initalized == true)
		return;

	// Setup async logger.
	spdlog::init_thread_pool(8192, 1);
	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("dti_dumper.log");

	console_proxy = std::make_shared<proxy_dist_sink_mt>();
	console_proxy->add_sink(stdout_sink);

	file_proxy = std::make_shared<proxy_dist_sink_mt>();
	file_proxy->add_sink(file_sink);

	spdlog::sinks_init_list loaderLoggerSinkList = { console_proxy, file_proxy };
	auto loaderLogger = std::make_shared<spdlog::async_logger>("Dumper", loaderLoggerSinkList.begin(), loaderLoggerSinkList.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
	loaderLogger->flush_on(spdlog::level::debug);
	spdlog::register_logger(loaderLogger);

	g_logger_initalized = true;
}