//STD
#include <thread>
#include <functional>
#include <queue>
#include <future>
#include <iostream>
#include <utility>
#include <chrono>
#include <array>
#include <string>
#include <cassert>

using namespace std::chrono_literals;

enum class ThreadPoolDestructorPolicy
{
	wait_for_jobs_to_finish,
	leave_jobs_unfinished
};

template <ThreadPoolDestructorPolicy destructor_policy = ThreadPoolDestructorPolicy::wait_for_jobs_to_finish,
          typename TraceFunctor = void>
class ThreadPool final
{
private:
	enum class worker_type
	{
		wait_for_new_job,
		wait_until_jobs_finished,
		one_job,
	};

	template <worker_type type>
	auto make_worker()
	{
		auto worker = [this]() -> void
		{
			std::unique_lock<std::mutex> l(mutex, std::defer_lock_t{});

			while (true)
			{
				l.lock();
				if constexpr (type == worker_type::wait_for_new_job)
				{
					cv.wait(l, [this]()
						{
							return !running || !jobs.empty();
						});

					if (!running)
						return;
				}
				else if constexpr (type == worker_type::wait_until_jobs_finished
					|| type == worker_type::one_job)
				{
					if (!running || jobs.empty())
						return;
				}

				auto job = std::move(jobs.front());
				jobs.pop();
				jobs_to_process--;

				l.unlock();

				job();

				if constexpr (type == worker_type::one_job)
					return;
			}
		};

		return worker;
	}

public:
	explicit ThreadPool(unsigned int thread_count = std::max(std::thread::hardware_concurrency() - 1, 1U))
	{
		if constexpr (std::is_same_v<TraceFunctor, void> == false)
			TraceFunctor::trace("spawning " + std::to_string(thread_count) + " worker threads");

		for (unsigned int i = 0; i < thread_count; ++i)
			threads.emplace_back(make_worker<worker_type::wait_for_new_job>());
	}

	ThreadPool(const ThreadPool& other) = delete;
	ThreadPool(ThreadPool&& other) = delete;
	ThreadPool& operator=(const ThreadPool& other) = delete;
	ThreadPool& operator=(ThreadPool&& other) = delete;

	~ThreadPool() noexcept
	{
		if constexpr (destructor_policy == ThreadPoolDestructorPolicy::wait_for_jobs_to_finish)
		{
			while (jobs_to_process)
			{
				if constexpr (std::is_same_v<TraceFunctor, void> == false)
					TraceFunctor::trace("\t" + std::to_string(jobs_to_process) + " jobs left");
				std::this_thread::sleep_for(100ms);
			}
		}
		else
		{
			if constexpr (std::is_same_v<TraceFunctor, void> == false)
				TraceFunctor::trace("leaving " + std::to_string(jobs_to_process) + " jobs unfinished");

			std::scoped_lock<std::mutex> l(mutex);
			while (jobs.size())
				jobs.pop();
			jobs_to_process = 0;
		}

		running = false;
		cv.notify_all();

		for (auto& thread : threads)
		{
			if constexpr (std::is_same_v<TraceFunctor, void> == false)
				TraceFunctor::trace("waiting for worker to finish last job");
			thread.join();
		}
	}

	void help()
	{
		if (!jobs_to_process)
			return;

		make_worker<worker_type::wait_until_jobs_finished>()();
	}

	void help_once()
	{
		if (!jobs_to_process)
			return;

		make_worker<worker_type::one_job>()();
	}

	template <typename F, typename... Args>
	auto addJob(F&& func, Args&&... args)
	{
		auto task = [func = std::forward<F>(func), tuple_args = std::make_tuple(std::forward<Args>(args)...)]() mutable
		{
			return std::apply([func = std::forward<F>(func)](auto&&... tuple_args)
			{
				return func(tuple_args...);
			}, std::move(tuple_args));
		};

		auto job = std::make_shared<std::packaged_task<std::invoke_result_t<F, Args...>()>>(std::move(task));
		auto future = job->get_future();

		auto perform_job = [job = std::move(job)]()
		{
			(*job)();
		};

		{
			std::scoped_lock<std::mutex> l(mutex);
			jobs.emplace(std::move(perform_job));
		}

		jobs_to_process++;
		cv.notify_one();
		return future;
	}

private:
	std::queue<std::function<void()>> jobs;
	std::mutex mutex;
	std::condition_variable cv;

	std::vector<std::thread> threads;
	std::atomic<bool> running = true;
	std::atomic<unsigned int> jobs_to_process = 0;
};

class ThreadPoolConsoleTracer
{
public:
    static void trace(std::string str)
    {
		std::cout << str << "\n";
    }
};

template <ThreadPoolDestructorPolicy destructor_policy>
using ThreadPoolConsoleTracing = ThreadPool<ThreadPoolDestructorPolicy::wait_for_jobs_to_finish, ThreadPoolConsoleTracer>;

int main()
{
	std::vector<std::future<int>> futures;
	std::vector<std::future<float>> futures2;
	futures.reserve(1000);

	static constexpr auto lambda = [](int something) -> int
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
		return 1;
	};

	int result = 0;

	const auto start_time = std::chrono::high_resolution_clock::now();

	{
		ThreadPool<ThreadPoolDestructorPolicy::leave_jobs_unfinished, ThreadPoolConsoleTracer> pool(10);

		const auto lambda2 = [&](int something, int something2) -> float
		{
			auto future = pool.addJob(lambda, something2);
			pool.help();
			future.wait();
			return 1;
		};

		for (int k = 0; k < 10; ++k)
		{
			for (int i = 0; i < 100; ++i)
				futures.emplace_back(pool.addJob(lambda, 1));

			for (int i = 0; i < 100; ++i)
				futures2.emplace_back(pool.addJob(lambda2, 1, 2));

			pool.help();

			for (auto& future : futures)
				result += future.get();

			for (auto& future : futures2)
				result += static_cast<int>(future.get());

			futures.clear();
			futures2.clear();
		}

		for (int i = 0; i < 1000; ++i)
			futures2.emplace_back(pool.addJob(lambda2, 1, 2));
	}

	const auto end_time = std::chrono::high_resolution_clock::now();
	std::cout << "took " << static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count()) / 1000.0f << " seconds\n";
	return 0;
}