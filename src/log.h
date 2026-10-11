#pragma once
#include <string>
#include "spdlog/spdlog.h" 

namespace Log {
	void InitializeLogger();
}

#define LOG_DEBUG(fmt, ...) \
    do {spdlog::get("Dumper")->debug(fmt, __VA_ARGS__);} while (0)
#define LOG_INFO(fmt, ...) \
    do {spdlog::get("Dumper")->info(fmt, __VA_ARGS__);} while (0)
#define LOG_WARN(fmt, ...) \
    do {spdlog::get("Dumper")->warn(fmt, __VA_ARGS__);} while (0)
#define LOG_ERROR(fmt, ...) \
    do {spdlog::get("Dumper")->error(fmt, __VA_ARGS__);} while (0)
#define LOG_CRITICAL(fmt, ...) \
    do {spdlog::get("Dumper")->critical(fmt, __VA_ARGS__);} while (0)