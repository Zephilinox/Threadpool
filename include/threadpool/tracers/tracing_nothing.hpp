#pragma once

//STD
#include <cstddef>

namespace zx
{

class threadpool_tracing_nothing
{
public:
    template <typename Threadpool>
    static void on_construction_start(const Threadpool& pool, unsigned int thread_count) noexcept
    {
    }

    template <typename Threadpool>
    static void on_construction_end(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_destructor_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_destructor_end(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_pushing_new_work_to_end(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_leave_pending_work_unfinished(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_finish(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_has_stopped(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_worker_thread_joining(const Threadpool& pool, std::size_t thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_worker_thread_joined(const Threadpool& pool, std::size_t thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_internal_error_new_work_in_queue_during_destruction(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_allow_new_work_changing(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_allow_new_work_changed(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_work_almost_pushed_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_work_almost_pushed_while(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_work_almost_pushed_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end_while(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_pending_work_to_end_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_leave_pending_work_unfinished_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_leave_pending_work_unfinished_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_notify_workers_to_stop_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_notify_workers_to_stop_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_end_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_end_while(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_for_executing_work_to_end_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_worker_thread_joining(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_worker_thread_joined(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_process_all_pending_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_process_all_pending_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_process_once_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_process_once_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_all_pending_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_all_pending_while(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_all_pending_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_all_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_all_while(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_wait_all_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_is_allowing_new_work(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_is_stopping_or_stopped(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_is_stopped(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_work_executed(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_work_executed_by_others(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_work_executed_total(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_work_pending(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_work_executing(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_work_total(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void thread_count(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_push_job_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_push_job_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_push_job_refused(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_push_task_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_push_task_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_push_task_refused(const Threadpool& pool) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_make_worker_start(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_make_worker_done(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_wait_change_in_pending_work_start(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_wait_change_in_pending_work_while(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_wait_change_in_pending_work_done(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_grab_work_start(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_grab_work_done(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_executing_start(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_executing_done(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_start(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <auto worker, typename Threadpool>
    static void on_worker_done(const Threadpool& pool, unsigned int thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_make_job_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_make_job_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_make_task_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_make_task_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_push_work_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_push_work_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_queued_new_work_start(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_queued_new_work_done(const Threadpool& pool) noexcept
    {
    }

    template <typename Threadpool>
    static void on_notify_change_in_pending_work(const Threadpool& pool) noexcept
    {
    }
};

} // namespace zx