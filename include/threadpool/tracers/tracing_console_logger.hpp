#pragma once

//SELF

//LIBS
#include <threadpool/threadpool.hpp>
#include <threadpool/tracers/tracing_logger.hpp>

//STD
#include <iostream>
#include <mutex>
#include <string>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace zx
{

template <unsigned int MaxLogLevel = 3>
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
            return "";
        case LogLevel::critical:
            return "CRT";
        case LogLevel::error:
            return "ERR";
        case LogLevel::info:
            return "INF";
        case LogLevel::debug:
            return "DBG";
        default:
            return "???";
        }
    }

    static std::size_t get_thread_id() noexcept
    {
        static std::atomic<std::size_t> thread_id{ 0 };
        const thread_local std::size_t id = thread_id;
        ++thread_id;
        return id;
    }

    static void log(LogLevel level, void* pool, const std::string& str)
    {
        if (static_cast<unsigned int>(level) > MaxLogLevel)
            return;

        static std::mutex mutex;
        std::scoped_lock lock(mutex);

        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) - std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());

        std::tm timeinfo;
#ifdef _MSC_VER
        localtime_s(&timeinfo, &time);
#else
        localtime_r(&time, &timeinfo);
#endif

        std::cout
            << " " << std::put_time(&timeinfo, "%H:%M:%S.") << std::setfill('0') << std::setw(6) << ms.count() << " "
            << " " << std::setfill('0') << std::setw(4) << get_thread_id() << " "
            << " " << pool << " "
            << log_level_to_string(level)
            << " " << str << std::endl;
    }
};

template <
    zx::threadpool_policy_pending_work pending_work_policy = zx::threadpool_policy_pending_work::wait_for_work_to_finish,
    zx::threadpool_policy_new_work new_work_policy = zx::threadpool_policy_new_work::configurable_and_forbidden_when_stopping,
    template <typename Logger> typename Tracer = zx::threadpool_tracing_logger>
using threadpool_console_logging = zx::threadpool<pending_work_policy, new_work_policy, Tracer<zx::threadpool_console_logger<>>>;

} // namespace zx