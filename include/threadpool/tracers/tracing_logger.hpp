#pragma once

//SELF
#include "threadpool/threadpool.hpp"
#include "threadpool/tracers/tracing_nothing.hpp"

//LIBS

//STD
#include <string>

namespace zx
{

#define THREADPOOL_INTERNAL_TRACE_LOG(log_level, string) \
    if constexpr (has_logger_v)                          \
    Logger::log(log_level, (void*)(&pool), string)

#define THREADPOOL_INTERNAL_TRACE_FUNC(name, log_level, string) \
    template <typename Threadpool>                              \
    static void name(Threadpool& pool) noexcept                 \
    {                                                           \
        if constexpr (has_logger_v)                             \
            Logger::log(log_level, (void*)(&pool), string);     \
    }

#define THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(name, log_level, string)  \
    template <auto worker, typename Threadpool>                         \
    static void name(Threadpool& pool, unsigned int thread_id) noexcept \
    {                                                                   \
        if constexpr (has_logger_v)                                     \
            Logger::log(log_level, (void*)(&pool), string);             \
    }

template <typename Logger>
class threadpool_tracing_logger
{
public:
    static constexpr bool has_logger_v = !std::is_same_v<Logger, void>;

    template <typename Threadpool>
    static void on_construction_start(Threadpool& pool, unsigned int thread_count) noexcept
    {
        THREADPOOL_INTERNAL_TRACE_LOG(Logger::LogLevel::info,
                                      "construction started. spawning " + std::to_string(thread_count) + " worker threads");
    }

    THREADPOOL_INTERNAL_TRACE_FUNC(on_construction_end, Logger::LogLevel::info,
                                   "construction finished. worker threads spawned")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_destructor_start, Logger::LogLevel::info,
                                   "started destruction. is_stopping = " + (pool.m_is_stopping ? std::string("true") : std::string("false")))

    template <typename Threadpool>
    static void on_destructor_end(Threadpool& pool) noexcept
    {
        THREADPOOL_INTERNAL_TRACE_LOG(Logger::LogLevel::info,
                                      std::to_string(pool.m_total_work_executed[pool.m_threads.size()]) + " units of work were executed by others");
        THREADPOOL_INTERNAL_TRACE_LOG(Logger::LogLevel::info, "finished destruction");
    }

    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pushing_new_work_to_end, Logger::LogLevel::info,
                                   "waiting for " + std::to_string(pool.m_work_almost_pushed) + " threads to stop pushing work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pending_work_to_end, Logger::LogLevel::info,
                                   std::to_string(pool.m_pending_work_to_process) + " pending work left")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_leave_pending_work_unfinished, Logger::LogLevel::info,
                                   "leaving " + std::to_string(pool.m_pending_work_to_process) + " pending work unfinished")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_executing_work_to_finish, Logger::LogLevel::info,
                                   "waiting for " + std::to_string(pool.m_work_executing) + " work to finish executing...")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_has_stopped, Logger::LogLevel::info, "has stopped")

    template <typename Threadpool>
    static void on_worker_thread_joining(Threadpool& pool, std::size_t thread_id) noexcept
    {
        THREADPOOL_INTERNAL_TRACE_LOG(Logger::LogLevel::info,
                                      "joining worker thread " + std::to_string(thread_id + 1) + "/" + std::to_string(pool.m_threads.size()));
    }

    template <typename Threadpool>
    static void on_worker_thread_joined(Threadpool& pool, std::size_t thread_id) noexcept
    {
        THREADPOOL_INTERNAL_TRACE_LOG(Logger::LogLevel::info,
                                      "joined worker thread " + std::to_string(thread_id + 1) + "/" + std::to_string(pool.m_threads.size()) + " after executing " + std::to_string(pool.m_total_work_executed[thread_id]) + " units of work");
    }

    THREADPOOL_INTERNAL_TRACE_FUNC(on_internal_error_new_work_in_queue_during_destruction, Logger::LogLevel::error,
                                   "internal error, " + std::to_string(pool.m_pending_work_to_process) + " pending work was added to the queue during destruction, which was not executed")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_allow_new_work_changing, Logger::LogLevel::info,
                                   "allowing new work is changing. currently: " + (pool.m_allow_new_work ? std::string("true") : std::string("false")))
    THREADPOOL_INTERNAL_TRACE_FUNC(on_allow_new_work_changed, Logger::LogLevel::info,
                                   "allowing new work has changed. now: " + (pool.m_allow_new_work ? std::string("true") : std::string("false")))
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_work_almost_pushed_start, Logger::LogLevel::info, "waiting for all threads to finish pushing work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_work_almost_pushed_while, Logger::LogLevel::info,
                                   "waiting for " + std::to_string(pool.m_work_almost_pushed) + " threads to finish pushing work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_work_almost_pushed_done, Logger::LogLevel::info, "done waiting for all threads to finish pushing work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pending_work_to_end_start, Logger::LogLevel::info, "waiting for all pending work to process")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pending_work_to_end_while, Logger::LogLevel::info,
                                   "waiting for " + std::to_string(pool.m_pending_work_to_process) + " pending units of work to process")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pending_work_to_end_done, Logger::LogLevel::info, "done waiting for all pending work to process")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_leave_pending_work_unfinished_start, Logger::LogLevel::info,
                                   "about to clear " + std::to_string(pool.m_pending_work_to_process) + " pending units of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_leave_pending_work_unfinished_done, Logger::LogLevel::info, "cleared all pending work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_notify_workers_to_stop_start, Logger::LogLevel::info, "notifying all workers to stop")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_notify_workers_to_stop_done, Logger::LogLevel::info, "notified all workers to stop")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_executing_work_to_end_start, Logger::LogLevel::info,
                                   "waiting for all executing units of work to end")

    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_executing_work_to_end_while, Logger::LogLevel::info,
                                   "waiting for " + std::to_string(pool.m_work_executing) + " executing units of work to end")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_executing_work_to_end_done, Logger::LogLevel::info,
                                   "done waiting for all executing units of work to end")

    template <typename Threadpool>
    static void on_worker_thread_joining(Threadpool& pool, unsigned int thread_id) noexcept
    {
        THREADPOOL_INTERNAL_TRACE_LOG(Logger::LogLevel::info,
                                      "waiting for worker " + std::to_string(thread_id) + " to finish");
    }

    template <typename Threadpool>
    static void on_worker_thread_joined(Threadpool& pool, unsigned int thread_id) noexcept
    {
        THREADPOOL_INTERNAL_TRACE_LOG(Logger::LogLevel::info,
                                      "done waiting for worker " + std::to_string(thread_id) + " to finish");
    }

    THREADPOOL_INTERNAL_TRACE_FUNC(on_process_all_pending_start, Logger::LogLevel::info,
                                   "processing " + std::to_string(pool.m_pending_work_to_process) + " pending units of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_process_all_pending_done, Logger::LogLevel::info, "processed all pending work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_process_once_start, Logger::LogLevel::info, "processing up to 1 pending units of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_process_once_done, Logger::LogLevel::info, "processed up to 1 pending units of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_pending_start, Logger::LogLevel::info, "waiting for all pending work to process")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_pending_while, Logger::LogLevel::info,
                                   "waiting for " + std::to_string(pool.m_pending_work_to_process) + " pending units of work to process")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_pending_done, Logger::LogLevel::info, "done waiting for all pending work to process")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_start, Logger::LogLevel::info, "waiting for all work to process and execute")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_while, Logger::LogLevel::info,
                                   "waiting for " + std::to_string(pool.m_pending_work_to_process) + " pending units of work to process")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_done, Logger::LogLevel::info, "done waiting for all pending work to process")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_is_allowing_new_work, Logger::LogLevel::debug, "checking if allowing new work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_is_stopping_or_stopped, Logger::LogLevel::debug, "checking if is stopping or stopped")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_is_stopped, Logger::LogLevel::debug, "checking if is stopped")
    
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_work_executed, Logger::LogLevel::debug,
                                   "checking amount of work executed for worker thread " + std::to_string(thread_id))
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_executed_by_others, Logger::LogLevel::debug, "checking amount of work executed by others")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_executed_total, Logger::LogLevel::debug, "checking amount of work executed in total")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_pending, Logger::LogLevel::debug, "checking amount of work pending")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_executing, Logger::LogLevel::debug, "checking amount of work executing")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_total, Logger::LogLevel::debug, "checking total amount of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(thread_count, Logger::LogLevel::debug, "checking thread count")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_job_start, Logger::LogLevel::debug, "pushing new job")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_job_done, Logger::LogLevel::debug, "pushed new job")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_job_refused, Logger::LogLevel::debug, "refused to push new job")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_task_start, Logger::LogLevel::debug, "pushing new task")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_task_done, Logger::LogLevel::debug, "pushed new task")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_task_refused, Logger::LogLevel::debug, "refused to push new task")
    
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_make_worker_start, Logger::LogLevel::debug, "making new worker")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_make_worker_done, Logger::LogLevel::debug, "made new worker")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_wait_change_in_pending_work_start, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " is waiting for a change in pending work")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_wait_change_in_pending_work_while, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " might be done waiting for a change in pending work")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_wait_change_in_pending_work_done, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " is done waiting for a change in pending work")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_grab_work_start, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " is grabbing a unit of work to execute")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_grab_work_done, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " has grabbed a unit of work to execute")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_executing_start, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " is executing a unit of work")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_executing_done, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " has executed a unit of work")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_start, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " has started")
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_done, Logger::LogLevel::debug,
                                          "worker " + std::to_string(thread_id) + " has stopped")

    THREADPOOL_INTERNAL_TRACE_FUNC(on_make_job_start, Logger::LogLevel::debug, "making a job")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_make_job_done, Logger::LogLevel::debug, "made a job")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_make_task_start, Logger::LogLevel::debug, "making a task")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_make_task_done, Logger::LogLevel::debug, "made a task")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_work_start, Logger::LogLevel::debug, "pushing a new unit of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_work_done, Logger::LogLevel::debug, "pushed a new unit of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_queued_new_work_start, Logger::LogLevel::debug, "queuing up a new unit of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_queued_new_work_done, Logger::LogLevel::debug, "queued up a new unit of work")
    THREADPOOL_INTERNAL_TRACE_FUNC(on_notify_change_in_pending_work, Logger::LogLevel::debug, "notifying a worker of a change in pending work")
};

#undef THREADPOOL_INTERNAL_TRACE_LOG
#undef THREADPOOL_INTERNAL_TRACE_FUNC
#undef THREADPOOL_INTERNAL_TRACE_FUNC_WORKER

} // namespace zx