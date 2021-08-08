#pragma once

//STD
#include <thread>
#include <functional>
#include <queue>
#include <future>
#include <utility>
#include <chrono>
#include <optional>
#include <type_traits>

namespace zx
{

enum class threadpool_policy_pending_work
{
    // When the threadpool is destroyed, all pending work will be executed, and those futures will never have broken promises
    wait_for_work_to_finish,
    // When the threadpool is destroyed, any pending work will not be executed, and those futures will have broken promises
    leave_work_unfinished,
};

enum class threadpool_policy_new_work
{
    // When the threadpool is destroyed, no new pending work can be pushed
    // The user may toggle blocking new pending work before destruction
    configurable_and_forbidden_when_stopping,
    // New pending work can always be pushed, which may cause the threadpool destructor to block indefinitely
    always_allowed,
};

namespace detail
{
enum class worker_t
{
    // The worker type for all threads that the thread pool starts, to process pending work
    wait_until_shutdown,
    // The worker type that users can create to help process all pending work
    wait_until_shutdown_or_no_pending_work,
    // The worker type that users can create to help process a single unit of work
    do_once_if_any_pending,
};
}

#define THREADPOOL_INTERNAL_TRACE(name) \
    if constexpr (has_tracing_v)        \
        tracer_t::name(*this);

#define THREADPOOL_INTERNAL_TRACE_COMPLEX(name, ...) \
    if constexpr (has_tracing_v)                     \
        tracer_t::name(*this, __VA_ARGS__);

#define THREADPOOL_INTERNAL_TRACE_TEMPLATE(name, ...) \
    if constexpr (has_tracing_v)                      \
        tracer_t::template name(*this, __VA_ARGS__);

template <
    threadpool_policy_pending_work pending_work_policy = threadpool_policy_pending_work::wait_for_work_to_finish,
    threadpool_policy_new_work new_work_policy = threadpool_policy_new_work::configurable_and_forbidden_when_stopping,
    typename Tracer = void,
    typename Function = std::function<void()>>
class threadpool final
{
public:
    static constexpr auto policy_pending_work_v = pending_work_policy;
    static constexpr auto policy_new_work_v = new_work_policy;
    static constexpr bool has_tracing_v = !std::is_same_v<Tracer, void>;
    using tracer_t = Tracer;
    using worker_t = detail::worker_t;
    using threadpool_function_t = Function;

    explicit threadpool(unsigned int thread_count = std::max(std::min(std::thread::hardware_concurrency(), 1U) - 1U, 1U));

    threadpool(const threadpool& other) = delete;
    threadpool(threadpool&& other) = delete;
    threadpool& operator=(const threadpool& other) = delete;
    threadpool& operator=(threadpool&& other) = delete;

    ~threadpool() noexcept;

    // Help process all pending work by executing it from the calling thread, including any new work
    // Will block while there is pending work to process
    // Will not block waiting for work on other threads to finish
    // Will not block if there is no pending work
    void process_all_pending();

    // Help process a single unit of work by executing it from the calling thread
    // Will block if there is work to do
    // Will not block waiting for other threads to finish
    // Wlll not block if there is no pending work
    void process_once();

    // Wait for all pending work to be complete, excluding work currently being executed by other threads
    void wait_all_pending() const;

    // Wait for all work to be complete, including any work currently being executed by other threads
    void wait_all() const;

    // Set the threadpool to allow or refuse new work
    // Only exists when ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping is used
    //   if is_stopping_or_stopped() then new work will be refused regardless
    template <typename = std::enable_if_t<new_work_policy == threadpool_policy_new_work::configurable_and_forbidden_when_stopping>>
    void allow_new_work(bool value);

    // Set the threadpool to allow or refuse new work
    // If threadpool_policy_new_work::configurable_and_forbidden_when_stopping
    //   If is_stopping_or_stopped() then this always returns false
    // If threadpool_policy_new_work::always_allowed
    //   This always returns true
    [[nodiscard]] bool is_allowing_new_work() const;

    // Get whether the threadpool has started stopping (in destructor, deciding what to do with pending work)
    [[nodiscard]] bool is_stopping_or_stopped() const;

    // Get whether the threadpool has stopped (in destructor, waiting for current work to finish executing)
    [[nodiscard]] bool is_stopped() const;

    // Get the total number of work items executed by the specified thread index
    [[nodiscard]] unsigned int work_executed(unsigned int thread_index) const;

    // Get the total number of work items executed not by the threadpool, e.g. via process_all_pending()
    [[nodiscard]] unsigned int work_executed_by_others() const;

    // Get the total number of work items executed
    [[nodiscard]] unsigned int work_executed_total() const;

    // Get the number of pending units of work
    [[nodiscard]] unsigned int work_pending() const;

    // Get the number of executing units of work
    [[nodiscard]] unsigned int work_executing() const;

    // Get the total number of units of work
    [[nodiscard]] unsigned int work_total() const;

    // Get the number of worker threads
    [[nodiscard]] unsigned int thread_count() const;

    // A job will be queued and executed, and you can specifically wait on the future for it to complete.
    //   warning: if the job pushes more work to the threadpool then it may deadlock if the thread pool does not hold sufficient threads
    //	 warning: if the job is not executed then the future will throw a broken promise exception
    //
    // If threadpool_policy_new_work::always_allowed is used
    //   This returns std::future<...>.
    //   The job will always be executed (unless threadpool_policy_pending_work::leave_work_unfinished is used)
    //
    // If threadpool_policy_new_work::configurable_and_forbidden_when_stopping is used
    //   This returns std::optional<std::future<...>> which will always hold a future when allowing_new_work()
    //   If a future is returned the job will always be executed (unless threadpool_policy_pending_work::leave_work_unfinished is used)
    //   If an empty optional is returned the job will never be executed
    template <typename F, typename... Args>
    auto push_job(F&& func, Args&&... args);

    // A task will be queued and executed, and you can't specifically wait for it complete. fire-and-forget.
    //   warning: if the task pushes more work to the threadpool then it may deadlock if the thread pool does not hold sufficient threads
    //
    // If threadpool_policy_new_work::always_allowed is used
    //   This returns void
    //   The task will always be executed (unless threadpool_policy_pending_work::leave_work_unfinished is used)
    //
    // If threadpool_policy_new_work::configurable_and_forbidden_when_stopping is used
    //   This returns bool, indicating whether the task was added to the pending work queue
    //   If it was added the task will always be executed (unless threadpool_policy_pending_work::leave_work_unfinished is used)
    //   If it was not added the task will never be executed
    template <typename F, typename... Args>
    auto push_task(F&& func, Args&&... args);

private:
    template <worker_t type>
    auto make_worker(unsigned int thread_id);

    template <typename F, typename... Args>
    auto make_job(F&& func, Args&&... args) -> std::shared_ptr<std::packaged_task<std::invoke_result_t<F, Args...>()>>;

    template <typename F, typename... Args>
    auto make_task(F&& func, Args&&... args);

    template <typename F>
    void push_work(F&& func);

    template <typename F, typename... Args>
    auto push_job_new_work_allow(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    template <typename F, typename... Args>
    auto push_job_new_work_forbid(F&& func, Args&&... args) -> std::optional<std::future<std::invoke_result_t<F, Args...>>>;

    template <typename F, typename... Args>
    auto push_task_new_work_allow(F&& func, Args&&... args) -> void;

    template <typename F, typename... Args>
    auto push_task_new_work_forbid(F&& func, Args&&... args) -> bool;

    std::queue<Function> m_pending_work;
    //also ensures m_pending_work_to_process is always in-sync with the queue
    std::mutex m_pending_work_mutex;
    std::condition_variable m_change_in_pending_work;

    std::vector<std::thread> m_threads;
    std::vector<std::atomic<unsigned int>> m_total_work_executed;
    std::atomic<bool> m_allow_new_work = true;
    //todo: should probably be an enum
    std::atomic<bool> m_shutting_down = false;
    std::atomic<bool> m_is_stopping = false;
    std::atomic<bool> m_is_stopped = false;
    std::atomic<unsigned int> m_pending_work_to_process = 0;
    std::atomic<unsigned int> m_work_executing = 0;
    std::atomic<unsigned int> m_work_almost_pushed = 0;

    friend Tracer;
};

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
threadpool<A, B, Tracer, D>::threadpool(unsigned int thread_count)
    : m_total_work_executed(thread_count + 1U)
{
    THREADPOOL_INTERNAL_TRACE_COMPLEX(on_construction_start, thread_count);
    for (unsigned int i = 0; i < thread_count; ++i)
    {
        m_threads.emplace_back(make_worker<worker_t::wait_until_shutdown>(i));
    }
    THREADPOOL_INTERNAL_TRACE(on_construction_end);
}

template <
    threadpool_policy_pending_work pending_work_policy,
    threadpool_policy_new_work new_work_policy,
    typename Tracer,
    typename D>
threadpool<pending_work_policy, new_work_policy, Tracer, D>::~threadpool() noexcept
{
    THREADPOOL_INTERNAL_TRACE(on_destructor_start);
    m_is_stopping = true;

    if constexpr (new_work_policy == threadpool_policy_new_work::configurable_and_forbidden_when_stopping)
    {
        THREADPOOL_INTERNAL_TRACE(on_allow_new_work_changing);
        m_allow_new_work = false;
        THREADPOOL_INTERNAL_TRACE(on_allow_new_work_changed);

        THREADPOOL_INTERNAL_TRACE(on_wait_for_work_almost_pushed_start);
        // A thread may have been in the middle of adding a new job when we blocked new work, so wait for it to finish
        while (m_work_almost_pushed)
        {
            THREADPOOL_INTERNAL_TRACE(on_wait_for_work_almost_pushed_while);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        THREADPOOL_INTERNAL_TRACE(on_wait_for_work_almost_pushed_done);
    }

    if constexpr (pending_work_policy == threadpool_policy_pending_work::wait_for_work_to_finish)
    {
        THREADPOOL_INTERNAL_TRACE(on_wait_for_pending_work_to_end_start);

        while (m_pending_work_to_process)
        {
            THREADPOOL_INTERNAL_TRACE(on_wait_for_pending_work_to_end_while);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        THREADPOOL_INTERNAL_TRACE(on_wait_for_pending_work_to_end_done);
    }
    else if constexpr (pending_work_policy == threadpool_policy_pending_work::leave_work_unfinished)
    {
        THREADPOOL_INTERNAL_TRACE(on_leave_pending_work_unfinished_start);
        std::scoped_lock lock(m_pending_work_mutex);

        //clear the m_pending_work
        m_pending_work = {};
        m_pending_work_to_process = 0;
        THREADPOOL_INTERNAL_TRACE(on_leave_pending_work_unfinished_done);
    }
    else
    {
        static_assert(pending_work_policy == threadpool_policy_pending_work::wait_for_work_to_finish, "Internal error: unknown pending work policy");
    }

    THREADPOOL_INTERNAL_TRACE(on_notify_workers_to_stop_start);
    m_shutting_down = true;
    m_change_in_pending_work.notify_all();
    THREADPOOL_INTERNAL_TRACE(on_notify_workers_to_stop_done);

    THREADPOOL_INTERNAL_TRACE(on_wait_for_executing_work_to_end_start);
    while (m_work_executing)
    {
        THREADPOOL_INTERNAL_TRACE(on_wait_for_executing_work_to_end_while);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    THREADPOOL_INTERNAL_TRACE(on_wait_for_executing_work_to_end_done);

    m_is_stopped = true;
    THREADPOOL_INTERNAL_TRACE(on_has_stopped);

    for (std::size_t thread_id = 0; thread_id < m_threads.size(); ++thread_id)
    {
        auto& thread = m_threads[thread_id];

        THREADPOOL_INTERNAL_TRACE_COMPLEX(on_worker_thread_joining, static_cast<unsigned int>(thread_id));
        thread.join();
        THREADPOOL_INTERNAL_TRACE_COMPLEX(on_worker_thread_joined, static_cast<unsigned int>(thread_id));
    }

    if constexpr (has_tracing_v)
    {
        if constexpr (new_work_policy == threadpool_policy_new_work::configurable_and_forbidden_when_stopping)
            if (m_pending_work_to_process)
                Tracer::on_internal_error_new_work_in_queue_during_destruction(*this);

        Tracer::on_destructor_end(*this);
    }
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
void threadpool<A, B, Tracer, D>::process_all_pending()
{
    if (!m_pending_work_to_process)
        return;

    THREADPOOL_INTERNAL_TRACE(on_process_all_pending_start);
    make_worker<worker_t::wait_until_shutdown_or_no_pending_work>(m_threads.size())();
    THREADPOOL_INTERNAL_TRACE(on_process_all_pending_done);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
void threadpool<A, B, Tracer, D>::process_once()
{
    if (!m_pending_work_to_process)
        return;

    THREADPOOL_INTERNAL_TRACE(on_process_once_start);
    make_worker<worker_t::do_once_if_any_pending>(m_threads.size())();
    THREADPOOL_INTERNAL_TRACE(on_process_once_done);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
void threadpool<A, B, Tracer, D>::wait_all_pending() const
{
    THREADPOOL_INTERNAL_TRACE(on_wait_all_pending_start);

    while (m_pending_work_to_process)
    {
        THREADPOOL_INTERNAL_TRACE(on_wait_all_pending_while);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    THREADPOOL_INTERNAL_TRACE(on_wait_all_pending_done);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
void threadpool<A, B, Tracer, D>::wait_all() const
{
    THREADPOOL_INTERNAL_TRACE(on_wait_all_start);
    // it's possible that m_pending_work_to_process increases after being read and before we check m_work_executing
    //   however, we only really need to wait for the existing work at the time of calling the function
    //   so any new work that may come in afterwards and become pending isn't a problem
    // We could lock a mutex here and during task/job push, to ensure no new work is added, but it shouldn't be necessary
    while (m_pending_work_to_process || m_work_executing)
    {
        THREADPOOL_INTERNAL_TRACE(on_wait_all_while);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    THREADPOOL_INTERNAL_TRACE(on_wait_all_done);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
template <typename>
void threadpool<A, B, Tracer, D>::allow_new_work(bool value)
{
    THREADPOOL_INTERNAL_TRACE(on_allow_new_work_changing);
    // ensure that we check m_is_stopping last to prevent a race that could allow new work to be pushed
    m_allow_new_work = value && m_is_stopping;
    THREADPOOL_INTERNAL_TRACE(on_allow_new_work_changed);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
bool threadpool<A, B, Tracer, D>::is_allowing_new_work() const
{
    THREADPOOL_INTERNAL_TRACE(on_is_allowing_new_work);
    return m_allow_new_work && !m_is_stopping;
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
bool threadpool<A, B, Tracer, D>::is_stopping_or_stopped() const
{
    THREADPOOL_INTERNAL_TRACE(on_is_stopping_or_stopped);
    return m_is_stopping;
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
bool threadpool<A, B, Tracer, D>::is_stopped() const
{
    THREADPOOL_INTERNAL_TRACE(on_is_stopped);
    return m_is_stopped;
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
unsigned int threadpool<A, B, Tracer, D>::work_executed(unsigned int thread_index) const
{
    THREADPOOL_INTERNAL_TRACE(on_work_executed);
    return m_total_work_executed[thread_index];
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
unsigned int threadpool<A, B, Tracer, D>::work_executed_by_others() const
{
    THREADPOOL_INTERNAL_TRACE(on_work_executed_by_others);
    return m_total_work_executed[m_threads.size()];
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
unsigned int threadpool<A, B, Tracer, D>::work_executed_total() const
{
    THREADPOOL_INTERNAL_TRACE(on_work_executed_total);
    // represents at minimum the total work executed at the time of function call
    // unlikely to represent total work executed at time of function call ending
    unsigned int total = 0;
    for (auto& thread_total : m_total_work_executed)
        total += thread_total;

    return total;
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
unsigned int threadpool<A, B, Tracer, D>::work_pending() const
{
    THREADPOOL_INTERNAL_TRACE(on_work_pending);
    return m_pending_work_to_process;
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
unsigned int threadpool<A, B, Tracer, D>::work_executing() const
{
    THREADPOOL_INTERNAL_TRACE(on_work_executing);
    return m_work_executing;
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
unsigned int threadpool<A, B, Tracer, D>::work_total() const
{
    THREADPOOL_INTERNAL_TRACE(on_work_total);
    // If pending work moves to executing between atomic loads the work total may appear higher than it really is, but that's preferable to lower
    return m_pending_work_to_process + m_work_executing;
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
unsigned int threadpool<A, B, Tracer, D>::thread_count() const
{
    THREADPOOL_INTERNAL_TRACE(on_thread_count);
    return m_threads.size();
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work new_work_policy, typename Tracer, typename D>
template <typename F, typename... Args>
auto threadpool<A, new_work_policy, Tracer, D>::push_job(F&& func, Args&&... args)
{
    THREADPOOL_INTERNAL_TRACE(on_push_job_start);

    if constexpr (new_work_policy == threadpool_policy_new_work::configurable_and_forbidden_when_stopping)
    {
        return push_job_new_work_forbid(std::forward<F>(func), std::forward<Args>(args)...);
    }
    else if (new_work_policy == threadpool_policy_new_work::always_allowed)
    {
        return push_job_new_work_allow(std::forward<F>(func), std::forward<Args>(args)...);
    }
    else
    {
        static_assert(new_work_policy == threadpool_policy_new_work::configurable_and_forbidden_when_stopping, "internal error: unknown new work policy");
    }
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
template <typename F, typename... Args>
auto threadpool<A, B, Tracer, D>::push_job_new_work_allow(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
{
    auto job = make_job(std::forward<F>(func), std::forward<Args>(args)...);
    auto future = job->get_future();
    push_work(std::move(job));

    THREADPOOL_INTERNAL_TRACE(on_push_job_done);
    return std::move(future);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
template <typename F, typename... Args>
auto threadpool<A, B, Tracer, D>::push_job_new_work_forbid(F&& func, Args&&... args) -> std::optional<std::future<std::invoke_result_t<F, Args...>>>
{
    ++m_work_almost_pushed;

    if (!m_allow_new_work)
    {
        --m_work_almost_pushed;
        THREADPOOL_INTERNAL_TRACE(on_push_job_refused);
        return std::nullopt;
    }

    auto job = make_job(std::forward<F>(func), std::forward<Args>(args)...);
    auto future = job->get_future();
    push_work(std::move(job));
    --m_work_almost_pushed;

    THREADPOOL_INTERNAL_TRACE(on_push_job_done);
    return std::move(future);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work new_work_policy, typename Tracer, typename D>
template <typename F, typename... Args>
auto threadpool<A, new_work_policy, Tracer, D>::push_task(F&& func, Args&&... args)
{
    THREADPOOL_INTERNAL_TRACE(on_push_task_start);

    if constexpr (new_work_policy == threadpool_policy_new_work::configurable_and_forbidden_when_stopping)
    {
        return push_task_new_work_forbid(std::forward<F>(func), std::forward<Args>(args)...);
    }
    else if constexpr (new_work_policy == threadpool_policy_new_work::always_allowed)
    {
        return push_task_new_work_allow(std::forward<F>(func), std::forward<Args>(args)...);
    }
    else
    {
        static_assert(new_work_policy == threadpool_policy_new_work::configurable_and_forbidden_when_stopping, "internal error: unknown new work policy");
    }
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work new_work_policy, typename Tracer, typename D>
template <typename F, typename... Args>
auto threadpool<A, new_work_policy, Tracer, D>::push_task_new_work_allow(F&& func, Args&&... args) -> void
{
    auto task = make_task(std::forward<F>(func), std::forward<Args>(args)...);
    push_work(std::move(task));

    THREADPOOL_INTERNAL_TRACE(on_push_task_done);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work new_work_policy, typename Tracer, typename D>
template <typename F, typename... Args>
auto threadpool<A, new_work_policy, Tracer, D>::push_task_new_work_forbid(F&& func, Args&&... args) -> bool
{
    if (!m_allow_new_work)
    {
        THREADPOOL_INTERNAL_TRACE(on_push_task_refused);
        return false;
    }

    ++m_work_almost_pushed;
    auto task = make_task(std::forward<F>(func), std::forward<Args>(args)...);
    push_work(std::move(task));
    --m_work_almost_pushed;

    THREADPOOL_INTERNAL_TRACE(on_push_task_done);
    return true;
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
template <detail::worker_t type>
auto threadpool<A, B, Tracer, D>::make_worker(unsigned int thread_id)
{
    THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_make_worker_start<type>, thread_id);
    auto worker = [this, thread_id]() -> void {
        std::unique_lock work_lock(m_pending_work_mutex, std::defer_lock_t{});
        THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_start<type>, thread_id);

        while (true)
        {
            work_lock.lock();

            if constexpr (type == worker_t::wait_until_shutdown)
            {
                THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_wait_change_in_pending_work_start<type>, thread_id);
                // If we're shutting down or there's work to process, then stop waiting
                // The predicate is checked before we wait, so if we have shut down between now and executing the last job
                //   we'll exit okay rather than deadlock
                m_change_in_pending_work.wait(work_lock, [this, thread_id]() {
                    THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_wait_change_in_pending_work_while<type>, thread_id);
                    return m_shutting_down || !m_pending_work.empty();
                });
                THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_wait_change_in_pending_work_done<type>, thread_id);
            }
            else if constexpr (type == worker_t::wait_until_shutdown_or_no_pending_work
                               || type == worker_t::do_once_if_any_pending)
            {
                // It's important to grab the lock before this
                //   so that new work doesn't come in after checking but before we return
                if (m_pending_work.empty())
                    break;
            }
            else
            {
                static_assert(type == worker_t::wait_until_shutdown, "internal error: unknown worker type");
            }

            if (m_shutting_down)
                break;

            THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_grab_work_start<type>, thread_id);
            auto job = std::move(m_pending_work.front());
            THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_grab_work_done<type>, thread_id);
            m_pending_work.pop();
            --m_pending_work_to_process;
            // lock after modifying m_pending_work_to_process to keep it in sync with the real queue
            work_lock.unlock();

            ++m_work_executing;
            THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_executing_start<type>, thread_id);
            std::move(job)();
            THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_executing_done<type>, thread_id);
            ++m_total_work_executed[thread_id];
            --m_work_executing;

            if constexpr (type == worker_t::do_once_if_any_pending)
                break;
        }

        THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_worker_done<type>, thread_id);
    };

    THREADPOOL_INTERNAL_TRACE_TEMPLATE(on_make_worker_done<type>, thread_id);
    return std::move(worker);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
template <typename F, typename... Args>
auto threadpool<A, B, Tracer, D>::make_job(F&& func, Args&&... args) -> std::shared_ptr<std::packaged_task<std::invoke_result_t<F, Args...>()>>
{
    THREADPOOL_INTERNAL_TRACE(on_make_job_start);

    auto task = make_task(std::forward<F>(func), std::forward<Args>(args)...);
    // todo: we need to wrap the packaged task in a shared_ptr to make it copyable for storing in a std::function
    //   we can use a unique_function type, like fu2::unique_function, to avoid this. would reducing memory allocations when pushing
    auto job = std::make_shared<std::packaged_task<std::invoke_result_t<F, Args...>()>>(std::move(task));

    THREADPOOL_INTERNAL_TRACE(on_make_job_done);
    return std::move(job);
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
template <typename F, typename... Args>
auto threadpool<A, B, Tracer, D>::make_task(F&& func, Args&&... args)
{
    THREADPOOL_INTERNAL_TRACE(on_make_task_start);

    //if there are no arguments then we can just use the function directly, we'll wrap it later
    if constexpr (sizeof...(args) == 0)
    {
        THREADPOOL_INTERNAL_TRACE(on_make_task_done);
        return std::forward<F>(func);
    }
    else
    {
        auto func_and_args_as_tuple = std::make_tuple(std::forward<F>(func), std::forward<Args>(args)...);

        //todo: C++20 allows for parameter pack captures: `...a = std::move(a)`
        auto task = [func_and_args_as_tuple = std::move(func_and_args_as_tuple)]() mutable -> decltype(auto) {
            return std::apply([](auto&& func, auto&&... args) mutable -> decltype(auto) {
                //forward/move the function so it has rvalue qualifiers for better optimisations
                return std::forward<decltype(func)>(func)(std::forward<decltype(args)>(args)...);
            },
                              std::move(func_and_args_as_tuple));
        };

        THREADPOOL_INTERNAL_TRACE(on_make_task_done);
        return std::move(task);
    }
}

template <threadpool_policy_pending_work A, threadpool_policy_new_work B, typename Tracer, typename D>
template <typename F>
void threadpool<A, B, Tracer, D>::push_work(F&& func)
{
    THREADPOOL_INTERNAL_TRACE(on_push_work_start);

    //make sure the work conforms to the interface of the queue functions (void return, no params)
    auto perform_job = [work = std::forward<F>(func)]() mutable -> void {
        if constexpr (std::is_invocable<F>())
        {
            std::move(work)(); // Tasks
        }
        else if constexpr (std::is_invocable<decltype(*work)>())
        {
            std::move (*work)(); // Jobs
        }
        else
        {
            static_assert(std::is_invocable<F>(), "internal error: work is not invocable");
        }
    };

    {
        std::scoped_lock lock(m_pending_work_mutex);

        THREADPOOL_INTERNAL_TRACE(on_queued_new_work_start);
        m_pending_work.emplace(std::move(perform_job));
        THREADPOOL_INTERNAL_TRACE(on_queued_new_work_done);

        ++m_pending_work_to_process;
    }

    THREADPOOL_INTERNAL_TRACE(on_notify_change_in_pending_work);
    m_change_in_pending_work.notify_one();

    THREADPOOL_INTERNAL_TRACE(on_push_work_done);
}

#undef THREADPOOL_INTERNAL_TRACE_TEMPLATE
#undef THREADPOOL_INTERNAL_TRACE_COMPLEX
#undef THREADPOOL_INTERNAL_TRACE

} // namespace zx