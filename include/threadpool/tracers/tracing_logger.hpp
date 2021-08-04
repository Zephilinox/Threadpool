#pragma once

//SELF

//LIBS
#include <threadpool/threadpool.hpp>

//STD

namespace zx
{

template <typename Logger>
class threadpool_tracing_logger
{
public:
    static constexpr bool has_logger_v = !std::is_same_v<Logger, void>;

    template <typename Threadpool>
    static void on_construction_start(const Threadpool& pool, unsigned int thread_count) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, "threadpool: construction started. spawning " + std::to_string(thread_count) + " worker threads");
    }

    template <typename Threadpool>
    static void on_construction_end(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, "threadpool: construction finished. worker threads spawned");
    }

    template <typename Threadpool>
    static void on_destructor_start(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, "threadpool: started destruction. is_stopping = " + (pool.m_is_stopping ? std::string("true") : std::string("false")));
    }

    template <typename Threadpool>
    static void on_destructor_end(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, "threadpool: " + std::to_string(pool.m_total_work_executed[pool.m_threads.size()]) + " units of work were executed by others");
            Logger::log(Logger::LogLevel::info, "threadpool: finished destruction");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_pushing_new_work_to_end(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, "threadpool: waiting for " + std::to_string(pool.m_work_almost_pushed) + " threads to stop pushing work");
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, "threadpool: " + std::to_string(pool.m_pending_work_to_process) + " pending work left");
    }

    template <typename Threadpool>
    static void on_leave_pending_work_unfinished(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, "threadpool: leaving " + std::to_string(pool.m_pending_work_to_process) + " pending work unfinished");
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_finish(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, "threadpool: waiting for " + std::to_string(pool.m_work_executing) + " work to finish executing...");
    }

    template <typename Threadpool>
    static void on_has_stopped(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, "threadpool: has stopped");
    }

    template <typename Threadpool>
    static void on_worker_thread_join(const Threadpool& pool, std::size_t thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info,
                        "threadpool: joining worker thread " + std::to_string(thread_id + 1) + "/" + std::to_string(pool.m_threads.size()) + " after executing " + std::to_string(pool.m_total_work_executed[thread_id]) + " units of work");
        }
    }

    template <typename Threadpool>
    static void on_internal_error_new_work_in_queue_during_destruction(const Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::error,
                        "threadpool: internal error, " + std::to_string(pool.m_pending_work_to_process) + " pending work was added to the queue during destruction, which was not executed");
        }
    }
};

} // namespace zx