#pragma once

//STD
#include <thread>
#include <functional>
#include <queue>
#include <future>
#include <utility>
#include <chrono>
#include <string>
#include <optional>

enum class ThreadpoolPolicyPendingWork
{
	// When the threadpool is destroyed, all pending work will be executed, and those futures will never have broken promises
	wait_for_work_to_finish,
	// When the threadpool is destroyed, any pending work will not be executed, and those futures will have broken promises
	leave_work_unfinished,
};

enum class ThreadpoolPolicyNewWork
{
	// When the threadpool is destroyed, no new pending work can be pushed
	// The user may toggle blocking new pending work before destruction
	configurable_and_forbidden_when_stopping,
	// New pending work can always be pushed, which may cause the threadpool destructor to block indefinitely
	allow,
};

namespace detail
{
	enum class WorkerType
	{
		// The worker type for all threads that the thread pool starts, to process pending work
		wait_until_shutdown,
		// The worker type that users can create to help process all pending work
		wait_until_shutdown_or_no_pending_work,
		// The worker type that users can create to help process a single unit of work
		do_once_if_any_pending,
	};
}

/* if a TraceFunctor is passed, TraceFunctor::trace() will be called with a single std::string parameter for the message
 * todo: have multiple methods and pass raw info to each one, letting the functor handle what it should do with it
 *   then a separate TraceFunctor can be built on top of it for ease-of-use
 */
template <
	ThreadpoolPolicyPendingWork pending_work_policy = ThreadpoolPolicyPendingWork::wait_for_work_to_finish,
	ThreadpoolPolicyNewWork new_work_policy = ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping,
	typename TraceFunctor = void>
class Threadpool final
{
public:
	explicit Threadpool(unsigned int thread_count = std::max(std::min(std::thread::hardware_concurrency(), 1U) - 1, 1U));

	Threadpool(const Threadpool& other) = delete;
	Threadpool(Threadpool&& other) = delete;
	Threadpool& operator=(const Threadpool& other) = delete;
	Threadpool& operator=(Threadpool&& other) = delete;

	~Threadpool() noexcept;

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
	template <typename = std::enable_if_t<new_work_policy == ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping>>
	void allow_new_work(bool value);
	
	// Set the threadpool to allow or refuse new work
	// If ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping
	//   If is_stopping_or_stopped() then this always returns false
	// If ThreadpoolPolicyNewWork::allow
	//   This always returns true
	[[nodiscard]] bool is_allowing_new_work() const;

	// Get whether the threadpool has started stopping (in destructor, deciding what to do with pending work)
	[[nodiscard]] bool is_stopping_or_stopped() const;

	// Get whether the threadpool has stopped (in destructor, waiting for current work to finish executing)
	[[nodiscard]] bool is_stopped() const;

	// Get the total number of work items executed by the specified thread index
	[[nodiscard]] unsigned int work_executed(unsigned int thread_index);

	// Get the total number of work items executed not by the threadpool, e.g. via process_all_pending()
	[[nodiscard]] unsigned int work_executed_by_others();

	// A job will be queued and executed, and you can specifically wait on the future for it to complete.
	//   warning: if the job pushes more work to the threadpool then it may deadlock if the thread pool does not hold sufficient threads
	//	 warning: if the job is not executed then the future will throw a broken promise exception
	//
	// If ThreadpoolPolicyNewWork::allow is used
	//   This returns std::future<...>.
	//   The job will always be executed (unless ThreadpoolPolicyPendingWork::leave_work_unfinished is used)
	//
	// If ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping is used
	//   This returns std::optional<std::future<...>> which will always hold a future when allowing_new_work()
	//   If a future is returned the job will always be executed (unless ThreadpoolPolicyPendingWork::leave_work_unfinished is used)
	//   If an empty optional is returned the job will never be executed
	template <typename F, typename... Args>
	auto push_job(F&& func, Args&&... args);

	// A task will be queued and executed, and you can't specifically wait for it complete. fire-and-forget.
	//   warning: if the task pushes more work to the threadpool then it may deadlock if the thread pool does not hold sufficient threads
	//
	// If ThreadpoolPolicyNewWork::allow is used
	//   This returns void
	//   The task will always be executed (unless ThreadpoolPolicyPendingWork::leave_work_unfinished is used)
	//
	// If ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping is used
	//   This returns bool, indicating whether the task was added to the pending work queue
	//   If it was added the task will always be executed (unless ThreadpoolPolicyPendingWork::leave_work_unfinished is used)
	//   If it was not added the task will never be executed
	template <typename F, typename... Args>
	auto push_task(F&& func, Args&&... args);
	
private:
	using WorkerType = detail::WorkerType;
	
	static constexpr bool has_trace_functor()
	{
		return !std::is_same_v<TraceFunctor, void>;
	}

	template <WorkerType type>
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

	std::queue<std::function<void()>> m_pending_work;
	//also ensures m_pending_work_to_process is always in-sync with the queue
	std::mutex m_pending_work_mutex;
	std::condition_variable m_change_in_pending_work;

	std::vector<std::thread> m_threads;
	std::vector<std::atomic<unsigned int>> m_total_work_executed;
	std::atomic<bool> m_shutting_down = false;
	std::atomic<bool> m_allow_new_work = true;
	std::atomic<bool> m_is_stopping = false;
	std::atomic<bool> m_is_stopped = false;
	std::atomic<unsigned int> m_pending_work_to_process = 0;
	std::atomic<unsigned int> m_work_executing = 0;
	std::atomic<unsigned int> m_work_almost_pushed = 0;
};

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename TraceFunctor>
Threadpool<A, B, TraceFunctor>::Threadpool(unsigned int thread_count)
	: m_total_work_executed(thread_count + 1)
{
	if constexpr (has_trace_functor())
		TraceFunctor::trace("spawning " + std::to_string(thread_count) + " worker threads");

	for (unsigned int i = 0; i < thread_count; ++i)
	{
		m_threads.emplace_back(make_worker<WorkerType::wait_until_shutdown>(i));
		m_total_work_executed[i] = 0;
	}

	// Used by threads executing work that are not owned by this object, e.g. via process_all_pending()
	m_total_work_executed[thread_count] = 0;
}

template <
	ThreadpoolPolicyPendingWork pending_work_policy,
	ThreadpoolPolicyNewWork new_work_policy,
	typename TraceFunctor>
Threadpool<pending_work_policy, new_work_policy, TraceFunctor>::~Threadpool() noexcept
{
	m_is_stopping = true;
	
	if constexpr (has_trace_functor())
		TraceFunctor::trace("stopping threadpool");

	if constexpr (new_work_policy == ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping)
	{
		m_allow_new_work = false;

		// A thread may have been in the middle of adding a new job when we blocked new work, so wait for it to finish
		while (m_work_almost_pushed)
		{
			if constexpr (has_trace_functor())
				TraceFunctor::trace("\twaiting for " + std::to_string(m_work_almost_pushed) + " threads to stop pushing work");

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	if constexpr (pending_work_policy == ThreadpoolPolicyPendingWork::wait_for_work_to_finish)
	{
		while (m_pending_work_to_process)
		{
			if constexpr (has_trace_functor())
				TraceFunctor::trace("\t" + std::to_string(m_pending_work_to_process) + " pending work left");

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	else if constexpr (pending_work_policy == ThreadpoolPolicyPendingWork::leave_work_unfinished)
	{
		if constexpr (has_trace_functor())
			TraceFunctor::trace("\tleaving " + std::to_string(m_pending_work_to_process) + " pending work unfinished");

		std::scoped_lock lock(m_pending_work_mutex);
		//clear the m_pending_work
		m_pending_work = {};
		m_pending_work_to_process = 0;
	}
	else
	{
		static_assert(pending_work_policy == ThreadpoolPolicyPendingWork::wait_for_work_to_finish, "Internal error: unknown pending work policy");
	}

	m_shutting_down = true;
	m_change_in_pending_work.notify_all();

	while (m_work_executing)
	{
		if constexpr (has_trace_functor())
			TraceFunctor::trace("\twaiting for " + std::to_string(m_work_executing) + " work to finish executing...");

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	m_is_stopped = true;
	
	if constexpr (has_trace_functor())
		TraceFunctor::trace("\tstopped threadpool");

	for (std::size_t thread_id = 0; thread_id < m_threads.size(); ++thread_id)
	{
		auto& thread = m_threads[thread_id];

		if constexpr (has_trace_functor())
		{
			TraceFunctor::trace("\tjoining worker thread " + std::to_string(thread_id + 1) + "/" + std::to_string(m_threads.size()) + " after executing " + std::to_string(m_total_work_executed[thread_id]) + " units of work");
		}

		thread.join();
	}
	
	if constexpr (has_trace_functor())
	{
		TraceFunctor::trace("\t" + std::to_string(m_total_work_executed[m_threads.size()]) + " units of work were executed by others");

		if (m_pending_work_to_process)
		{
			TraceFunctor::trace("\tinternal error: " + std::to_string(m_pending_work_to_process) + " pending work was added to the queue during destruction, and did not execute");
		}
	}

	if constexpr (has_trace_functor())
		TraceFunctor::trace("\tdestroyed threadpool");
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
void Threadpool<A, B, C>::process_all_pending()
{
	if (!m_pending_work_to_process)
		return;

	make_worker<WorkerType::wait_until_shutdown_or_no_pending_work>(m_threads.size())();
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
void Threadpool<A, B, C>::process_once()
{
	if (!m_pending_work_to_process)
		return;

	make_worker<WorkerType::do_once_if_any_pending>(m_threads.size())();
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
void Threadpool<A, B, C>::wait_all_pending() const
{
	while (m_pending_work_to_process)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
void Threadpool<A, B, C>::wait_all() const
{
	// it's possible that m_pending_work_to_process increases after being read and before we check m_work_executing
	//   however, we only really need to wait for the existing work at the time of calling the function
	//   so any new work that may come in afterwards and become pending isn't a problem
	// We could lock a mutex here and during task/job push, to ensure no new work is added, but it shouldn't be necessary
	while (m_pending_work_to_process || m_work_executing)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
template <typename>
void Threadpool<A, B, C>::allow_new_work(bool value)
{
	if (m_is_stopping)
		return;

	m_allow_new_work = value;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
bool Threadpool<A, B, C>::is_allowing_new_work() const
{
	return m_allow_new_work && !m_is_stopping;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
bool Threadpool<A, B, C>::is_stopping_or_stopped() const
{
	return m_is_stopping;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
bool Threadpool<A, B, C>::is_stopped() const
{
	return m_is_stopped;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
unsigned int Threadpool<A, B, C>::work_executed(unsigned int thread_index)
{
	return m_total_work_executed[thread_index];
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
unsigned int Threadpool<A, B, C>::work_executed_by_others()
{
	return m_total_work_executed[m_threads.size()];
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork new_work_policy, typename C>
template <typename F, typename... Args>
auto Threadpool<A, new_work_policy, C>::push_job(F&& func, Args&&... args)
{
	if constexpr (new_work_policy == ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping)
	{
		return push_job_new_work_forbid(std::forward<F>(func), std::forward<Args>(args)...);
	}
	else if (new_work_policy == ThreadpoolPolicyNewWork::allow)
	{
		return push_job_new_work_allow(std::forward<F>(func), std::forward<Args>(args)...);
	}
	else
	{
		static_assert(new_work_policy == ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping, "internal error: unknown new work policy");
	}
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
template <typename F, typename... Args>
auto Threadpool<A, B, C>::push_job_new_work_allow(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
{
	auto job = make_job(std::forward<F>(func), std::forward<Args>(args)...);
	auto future = job->get_future();
	push_work(std::move(job));
	return future;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
template <typename F, typename... Args>
auto Threadpool<A, B, C>::push_job_new_work_forbid(F&& func, Args&&... args) -> std::optional<std::future<std::invoke_result_t<F, Args...>>>
{
	if (!m_allow_new_work)
		return std::nullopt;

	++m_work_almost_pushed;
	auto job = make_job(std::forward<F>(func), std::forward<Args>(args)...);
	auto future = job->get_future();
	push_work(std::move(job));
	--m_work_almost_pushed;
	return future;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork new_work_policy, typename C>
template <typename F, typename... Args>
auto Threadpool<A, new_work_policy, C>::push_task(F&& func, Args&&... args)
{
	if constexpr (new_work_policy == ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping)
	{
		return push_task_new_work_forbid(std::forward<F>(func), std::forward<Args>(args)...);
	}
	else if constexpr (new_work_policy == ThreadpoolPolicyNewWork::allow)
	{
		return push_task_new_work_allow(std::forward<F>(func), std::forward<Args>(args)...);
	}
	else
	{
		static_assert(new_work_policy == ThreadpoolPolicyNewWork::configurable_and_forbidden_when_stopping, "internal error: unknown new work policy");
	}
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork new_work_policy, typename C>
template <typename F, typename... Args>
auto Threadpool<A, new_work_policy, C>::push_task_new_work_allow(F&& func, Args&&... args) -> void
{
	auto task = make_task(std::forward<F>(func), std::forward<Args>(args)...);
	push_work(std::move(task));
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork new_work_policy, typename C>
template <typename F, typename... Args>
auto Threadpool<A, new_work_policy, C>::push_task_new_work_forbid(F&& func, Args&&... args) -> bool
{
	if (!m_allow_new_work)
		return false;
	
	++m_work_almost_pushed;
	auto task = make_task(std::forward<F>(func), std::forward<Args>(args)...);
	push_work(std::move(task));
	--m_work_almost_pushed;
	return true;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
template <detail::WorkerType type>
auto Threadpool<A, B, C>::make_worker(unsigned int thread_id)
{
	auto worker = [this, thread_id]() -> void
	{
		std::unique_lock work_lock(m_pending_work_mutex, std::defer_lock_t{});

		while (true)
		{
			if constexpr (type == WorkerType::wait_until_shutdown)
			{
				work_lock.lock();
				m_change_in_pending_work.wait(work_lock, [this]()
				{
					// If we're shutting down or there's work to process, then stop waiting
					return m_shutting_down || m_pending_work_to_process;
				});

				if (m_shutting_down)
					return;
			}
			else if constexpr (type == WorkerType::wait_until_shutdown_or_no_pending_work
				|| type == WorkerType::do_once_if_any_pending)
			{
				if (m_shutting_down || !m_pending_work_to_process)
					return;
				
				// The condition variable will have locked it for the other worker, but these need to lock it explicitly
				work_lock.lock();
			}
			else
			{
				static_assert(type == WorkerType::wait_until_shutdown, "internal error: unknown worker type");
			}
			
			auto job = std::move(m_pending_work.front());
			m_pending_work.pop();
			--m_pending_work_to_process;
			work_lock.unlock();

			++m_work_executing;
			std::move(job)();
			++m_total_work_executed[thread_id];
			--m_work_executing;

			if constexpr (type == WorkerType::do_once_if_any_pending)
				return;
		}
	};

	return worker;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
template <typename F, typename... Args>
auto Threadpool<A, B, C>::make_job(F&& func, Args&&... args) -> std::shared_ptr<std::packaged_task<std::invoke_result_t<F, Args...>()>>
{
	auto task = make_task(std::forward<F>(func), std::forward<Args>(args)...);
	// todo: we need to wrap the packaged task in a shared_ptr to make it copyable for storing in a std::function
	//   we can use a unique_function type, like fu2::unique_function, to avoid this. would reducing memory allocations when pushing
	auto job = std::make_shared<std::packaged_task<std::invoke_result_t<F, Args...>()>>(std::move(task));
	return job;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
template <typename F, typename... Args>
auto Threadpool<A, B, C>::make_task(F&& func, Args&&... args)
{
	auto task = [func = std::forward<F>(func), tuple_args = std::make_tuple(std::forward<Args>(args)...)]() mutable
	{
		return std::apply([func = std::forward<F>(func)](auto&&... tuple_args) mutable
		{
			return std::forward<F>(func)(tuple_args...);
		}, std::move(tuple_args));
	};

	return task;
}

template <ThreadpoolPolicyPendingWork A, ThreadpoolPolicyNewWork B, typename C>
template <typename F>
void Threadpool<A, B, C>::push_work(F&& func)
{
	auto perform_job = [work = std::forward<F>(func)]() mutable -> void
	{
		if constexpr (std::is_invocable<F>())
		{
			std::invoke(std::move(work)); // Tasks
		}
		if constexpr (std::is_invocable<decltype(*work)>())
		{
			std::invoke(*work); // Jobs
		}
		else
		{
			static_assert(std::is_invocable<F>(), "internal error: work is not invocable");
		}
	};

	{
		std::scoped_lock lock(m_pending_work_mutex);
		m_pending_work.emplace(std::move(perform_job));
		++m_pending_work_to_process;
	}

	m_change_in_pending_work.notify_one();
}