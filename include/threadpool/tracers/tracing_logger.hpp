#pragma once

//SELF
#include "threadpool/threadpool.hpp"
#include "threadpool/tracers/tracing_nothing.hpp"

//LIBS

//STD
#include <string>

namespace zx
{

template <typename Logger>
class threadpool_tracing_logger
{
public:
    static constexpr bool has_logger_v = !std::is_same_v<Logger, void>;

    template <typename Threadpool>
    static void on_construction_start(Threadpool& pool, unsigned int thread_count) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, (void*)(&pool), "construction started. spawning " + std::to_string(thread_count) + " worker threads");
    }

    template <typename Threadpool>
    static void on_construction_end(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, (void*)(&pool), "construction finished. worker threads spawned");
    }

    template <typename Threadpool>
    static void on_destructor_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, (void*)(&pool), "started destruction. is_stopping = " + (pool.m_is_stopping ? std::string("true") : std::string("false")));
    }

    template <typename Threadpool>
    static void on_destructor_end(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool), std::to_string(pool.m_total_work_executed[pool.m_threads.size()]) + " units of work were executed by others");
            Logger::log(Logger::LogLevel::info, (void*)(&pool), "finished destruction");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_pushing_new_work_to_end(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, (void*)(&pool), "waiting for " + std::to_string(pool.m_work_almost_pushed) + " threads to stop pushing work");
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, (void*)(&pool), std::to_string(pool.m_pending_work_to_process) + " pending work left");
    }

    template <typename Threadpool>
    static void on_leave_pending_work_unfinished(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, (void*)(&pool), "leaving " + std::to_string(pool.m_pending_work_to_process) + " pending work unfinished");
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_finish(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, (void*)(&pool), "waiting for " + std::to_string(pool.m_work_executing) + " work to finish executing...");
    }

    template <typename Threadpool>
    static void on_has_stopped(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
            Logger::log(Logger::LogLevel::info, (void*)(&pool), "has stopped");
    }

    template <typename Threadpool>
    static void on_worker_thread_joining(Threadpool& pool, std::size_t thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "joining worker thread " + std::to_string(thread_id + 1) + "/" + std::to_string(pool.m_threads.size()));
        }
    }

    template <typename Threadpool>
    static void on_worker_thread_joined(Threadpool& pool, std::size_t thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "joined worker thread " + std::to_string(thread_id + 1) + "/" + std::to_string(pool.m_threads.size()) + " after executing " + std::to_string(pool.m_total_work_executed[thread_id]) + " units of work");
        }
    }

    template <typename Threadpool>
    static void on_internal_error_new_work_in_queue_during_destruction(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::error, (void*)(&pool),
                        "internal error, " + std::to_string(pool.m_pending_work_to_process) + " pending work was added to the queue during destruction, which was not executed");
        }
    }

    template <typename Threadpool>
    static void on_allow_new_work_changing(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "allowing new work is changing. currently: " + (pool.m_allow_new_work ? std::string("true") : std::string("false")));
        }
    }

    template <typename Threadpool>
    static void on_allow_new_work_changed(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "allowing new work has changed. now: " + (pool.m_allow_new_work ? std::string("true") : std::string("false")));
        }
    }

    template <typename Threadpool>
    static void on_wait_for_work_almost_pushed_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for all threads to finish pushing work");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_work_almost_pushed_while(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for " + std::to_string(pool.m_work_almost_pushed) + " threads to finish pushing work");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_work_almost_pushed_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "done waiting for all threads to finish pushing work");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for all pending work to process");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end_while(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for " + std::to_string(pool.m_pending_work_to_process) + " pending units of work to process");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "done waiting for all pending work to process");
        }
    }

    template <typename Threadpool>
    static void on_leave_pending_work_unfinished_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "about to clear " + std::to_string(pool.m_pending_work_to_process) + " pending units of work");
        }
    }

    template <typename Threadpool>
    static void on_leave_pending_work_unfinished_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "cleared all pending work");
        }
    }

    template <typename Threadpool>
    static void on_notify_workers_to_stop_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "notifying all workers to stop");
        }
    }

    template <typename Threadpool>
    static void on_notify_workers_to_stop_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "notified all workers to stop");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_end_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for all executing units of work to end");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_end_while(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for " + std::to_string(pool.m_work_executing) + " executing units of work to end");
        }
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_end_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "done waiting for all executing units of work to end");
        }
    }

    template <typename Threadpool>
    static void on_workerhread_joining(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for worker " + std::to_string(thread_id) + " to finish");
        }
    }

    template <typename Threadpool>
    static void on_workerhread_joined(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "done waiting for worker " + std::to_string(thread_id) + " to finish");
        }
    }

    template <typename Threadpool>
    static void on_process_all_pending_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "processing " + std::to_string(pool.m_pending_work_to_process) + " pending units of work");
        }
    }

    template <typename Threadpool>
    static void on_process_all_pending_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "processed all pending work");
        }
    }

    template <typename Threadpool>
    static void on_process_once_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "processing up to 1 pending units of work");
        }
    }

    template <typename Threadpool>
    static void on_process_once_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "processed up to 1 pending units of work");
        }
    }

    template <typename Threadpool>
    static void on_wait_all_pending_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for all pending work to process");
        }
    }

    template <typename Threadpool>
    static void on_wait_all_pending_while(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for " + std::to_string(pool.m_pending_work_to_process) + " pending units of work to process");
        }
    }

    template <typename Threadpool>
    static void on_wait_all_pending_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "done waiting for all pending work to process");
        }
    }

    template <typename Threadpool>
    static void on_wait_all_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for all work to process and execute");
        }
    }

    template <typename Threadpool>
    static void on_wait_all_while(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "waiting for " + std::to_string(pool.m_pending_work_to_process) + " pending units of work to process");
        }
    }

    template <typename Threadpool>
    static void on_wait_all_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::info, (void*)(&pool),
                        "done waiting for all pending work to process");
        }
    }

    template <typename Threadpool>
    static void on_is_allowing_new_work(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking if allowing new work");
        }
    }

    template <typename Threadpool>
    static void on_is_stopping_or_stopped(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking if is stopping or stopped");
        }
    }

    template <typename Threadpool>
    static void on_is_stopped(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking if is stopped");
        }
    }

    template <typename Threadpool>
    static void on_work_executed(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking amount of work executed for worker thread " + std::to_string(thread_id));
        }
    }

    template <typename Threadpool>
    static void on_work_executed_by_others(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking amount of work executed by others");
        }
    }

    template <typename Threadpool>
    static void on_work_executed_total(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking amount of work executed in total");
        }
    }

    template <typename Threadpool>
    static void on_work_pending(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking amount of work pending");
        }
    }

    template <typename Threadpool>
    static void on_work_executing(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking amount of work executing");
        }
    }

    template <typename Threadpool>
    static void on_work_total(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking total amount of work");
        }
    }

    template <typename Threadpool>
    static void thread_count(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "checking thread count");
        }
    }

    template <typename Threadpool>
    static void on_push_job_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "pushing new job");
        }
    }

    template <typename Threadpool>
    static void on_push_job_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "pushed new job");
        }
    }

    template <typename Threadpool>
    static void on_push_job_refused(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "refused to push new job");
        }
    }

    template <typename Threadpool>
    static void on_push_task_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "pushing new task");
        }
    }

    template <typename Threadpool>
    static void on_push_task_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "pushed new task");
        }
    }

    template <typename Threadpool>
    static void on_push_task_refused(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "refused to push new task");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_make_worker_start(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "making new worker");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_make_worker_done(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "made new worker");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_wait_change_in_pending_work_start(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " is waiting for a change in pending work");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_wait_change_in_pending_work_while(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " might be done waiting for a change in pending work");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_wait_change_in_pending_work_done(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " is done waiting for a change in pending work");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_grab_work_start(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " is grabbing a unit of work to execute");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_grab_work_done(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " has grabbed a unit of work to execute");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_executing_start(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " is executing a unit of work");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_executing_done(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " has executed a unit of work");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_start(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " has started");
        }
    }

    template <auto worker, typename Threadpool>
    static void on_worker_done(Threadpool& pool, unsigned int thread_id) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "worker " + std::to_string(thread_id) + " has stopped");
        }
    }

    template <typename Threadpool>
    static void on_make_job_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "making a job");
        }
    }

    template <typename Threadpool>
    static void on_make_job_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "made a job");
        }
    }

    template <typename Threadpool>
    static void on_make_task_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "making a task");
        }
    }

    template <typename Threadpool>
    static void on_make_task_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "made a task");
        }
    }

    template <typename Threadpool>
    static void on_push_work_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "pushing a new unit of work");
        }
    }

    template <typename Threadpool>
    static void on_push_work_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "pushed a new unit of work");
        }
    }

    template <typename Threadpool>
    static void on_queued_new_work_start(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "queuing up a new unit of work");
        }
    }

    template <typename Threadpool>
    static void on_queued_new_work_done(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "queued up a new unit of work");
        }
    }

    template <typename Threadpool>
    static void on_notify_change_in_pending_work(Threadpool& pool) noexcept
    {
        if constexpr (has_logger_v)
        {
            Logger::log(Logger::LogLevel::debug, (void*)(&pool),
                        "notifying a worker of a change in pending work");
        }
    }
};

} // namespace zx