#pragma once

//STD
#include <cstddef>

namespace zx
{

#define THREADPOOL_INTERNAL_TRACE_FUNC(name)    \
    template <typename Threadpool>              \
    static void name(Threadpool& pool) noexcept \
    {                                           \
    }

#define THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(name)                     \
    template <auto worker, typename Threadpool>                         \
    static void name(Threadpool& pool, unsigned int thread_id) noexcept \
    {                                                                   \
    }

class threadpool_tracing_nothing
{
public:
    template <typename Threadpool>
    static void on_construction_start(const Threadpool& pool, unsigned int thread_count) noexcept
    {
    }

    THREADPOOL_INTERNAL_TRACE_FUNC(on_construction_end)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_destructor_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_destructor_end)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pushing_new_work_to_end)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pending_work_to_end)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_leave_pending_work_unfinished)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_executing_work_to_finish)

    THREADPOOL_INTERNAL_TRACE_FUNC(on_has_stopped)

    template <typename Threadpool>
    static void on_worker_thread_joining(const Threadpool& pool, std::size_t thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_worker_thread_joined(const Threadpool& pool, std::size_t thread_id) noexcept
    {
    }

    THREADPOOL_INTERNAL_TRACE_FUNC(on_internal_error_new_work_in_queue_during_destruction)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_allow_new_work_changing)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_allow_new_work_changed)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_work_almost_pushed_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_work_almost_pushed_while)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_work_almost_pushed_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pending_work_to_end_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pending_work_to_end_while)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_pending_work_to_end_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_leave_pending_work_unfinished_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_leave_pending_work_unfinished_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_notify_workers_to_stop_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_notify_workers_to_stop_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_executing_work_to_end_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_executing_work_to_end_while)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_for_executing_work_to_end_done)

    template <typename Threadpool>
    static void on_worker_thread_joining(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_worker_thread_joined(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    THREADPOOL_INTERNAL_TRACE_FUNC(on_process_all_pending_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_process_all_pending_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_process_once_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_process_once_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_pending_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_pending_while)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_pending_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_while)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_wait_all_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_is_allowing_new_work)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_is_stopped)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_executed)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_executed_by_others)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_executed_total)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_executing)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_work_total)
    THREADPOOL_INTERNAL_TRACE_FUNC(thread_count)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_job_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_job_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_job_refused)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_task_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_task_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_task_refused)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_make_worker_start)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_make_worker_done)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_wait_change_in_pending_work_start)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_wait_change_in_pending_work_while)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_wait_change_in_pending_work_done)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_grab_work_start)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_grab_work_done)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_executing_start)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_executing_done)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_start)
    THREADPOOL_INTERNAL_TRACE_FUNC_WORKER(on_worker_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_make_job_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_make_job_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_make_task_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_make_task_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_work_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_push_work_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_queued_new_work_start)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_queued_new_work_done)
    THREADPOOL_INTERNAL_TRACE_FUNC(on_notify_change_in_pending_work)
};

#undef THREADPOOL_INTERNAL_TRACE_FUNC
#undef THREADPOOL_INTERNAL_TRACE_FUNC_WORKER

} // namespace zx