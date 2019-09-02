//STD
#include <thread>
#include <functional>
#include <queue>
#include <future>
#include <iostream>
#include <utility>
#include <chrono>
#include <array>

using namespace std::chrono_literals;

class ThreadPool
{
public:
	ThreadPool()
	{
		std::cout << std::thread::hardware_concurrency() << " worker threads spawned\n";
		for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i)
		{
			threads.emplace_back([this, i]()
			{
				std::unique_lock l(this->mutex, std::defer_lock_t{});

				while (true)
				{
					l.lock();
					cv.wait(l, [this]()
					{
						return !this->running || !this->jobs.empty();
					});

					if (!this->running)
					{
						return;
					}

					auto job = std::move(this->jobs.front());
					this->jobs.pop();
					l.unlock();

					job();
					jobs_completed[i]++;
				}
			});
		}
	}

	~ThreadPool()
	{
		std::cout << "Stopping worker threads\n";
		while (!jobs.empty())
		{
			std::cout << "There are " << jobs.size() << " jobs queued up\n";
			std::this_thread::sleep_for(100ms);
		}
		std::cout << "No more jobs queued up\n";

		running = false;
		cv.notify_all();

		for (unsigned int i = 0; i < threads.size(); ++i)
		{
			std::cout << "Waiting for worker thread " << i << " to finish its current job\n";
			threads[i].join();
			std::cout << "Worker thread " << i << " has finished after completing " << jobs_completed[i] << " jobs.\n";
		}
	}

	template <typename F, typename... Args>
	auto addJob(F&& func, Args... args)
	{
		//for some reason I can't move a unique_ptr in to the lambda, so this will have to do
		auto task = new std::packaged_task<std::invoke_result_t<F, Args...>()>
			(std::bind(std::forward<F>(func), std::forward<Args>(args)...));

		auto future = task->get_future();

		{
			std::scoped_lock l(mutex);
			jobs.emplace([task]()
			{
				(*task)();
				delete task;
			});
		}

		cv.notify_one();
		//note: until .get() is called on the future
		//the internal shared state will stay alive and consume memory
		//even if packaged_task is deleted in the job
		return future;
	}

private:
	std::queue<std::function<void()>> jobs;
	std::mutex mutex;
	std::condition_variable cv;

	std::vector<std::thread> threads;
	std::atomic<bool> running = true;

	std::array<int, 256> jobs_completed;
};

int main()
{
	std::vector<std::future<int>> futures;
	futures.reserve(10000000);

	{
		ThreadPool pool;

		for (int i = 0; i < 10000000; ++i)
		{
			futures.emplace_back(pool.addJob([](int something) -> int
			{
				return something * 2;
			}, 5));
		}
	}

	int result = 0;
	for (auto& future : futures)
	{
		result += future.get();
	}

	std::cout << result << "\n";

	return 0;
}