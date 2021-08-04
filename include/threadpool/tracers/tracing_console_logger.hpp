#pragma once

//SELF

//LIBS
#include <threadpool/threadpool.hpp>
#include <threadpool/tracers/tracing_logger.hpp>

//STD
#include <iostream>
#include <mutex>
#include <string>

namespace zx
{

class threadpool_console_logger
{
public:
    enum class LogLevel
    {
        none = 0,
        critical = 1,
        error = 2,
        info = 3,
        debug = 4,
    };

    static std::string log_level_to_string(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::none:
            return "[NONE]";
        case LogLevel::critical:
            return "[CRITICAL]";
        case LogLevel::error:
            return "[ERROR]";
        case LogLevel::info:
            return "[INFO]";
        case LogLevel::debug:
            return "[DEBUG]";
        default:
            return "[UNKNOWN]";
        }
    }

    static void log(LogLevel level, const std::string& str)
    {
        static std::mutex mutex;
        std::scoped_lock lock(mutex);
        std::cout << log_level_to_string(level) << " " << str << "\n";
    }
};

template <
    zx::threadpool_policy_pending_work pending_work_policy = zx::threadpool_policy_pending_work::wait_for_work_to_finish,
    zx::threadpool_policy_new_work new_work_policy = zx::threadpool_policy_new_work::configurable_and_forbidden_when_stopping,
    template <typename Logger> typename Tracer = zx::threadpool_tracing_logger>
using threadpool_console_logging = zx::threadpool<pending_work_policy, new_work_policy, Tracer<zx::threadpool_console_logger>>;

} // namespace zx