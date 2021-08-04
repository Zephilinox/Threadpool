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
    static void on_worker_thread_join(const Threadpool& pool, std::size_t thread_id) noexcept
    {
    }

    template <typename Threadpool>
    static void on_internal_error_new_work_in_queue_during_destruction(const Threadpool& pool) noexcept
    {
    }
};

} // namespace zx