//LIBS
#include <threadpool/threadpool.hpp>

//STD
#include <iostream>

class ThreadpoolConsoleLogger
{
public:
    enum class LogLevel
    {
        none = 0,
        critical = 1,
        error = 2,
        info = 3,
        debug = 4,
    };

    static std::string log_level_to_string(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::none:
            return "[NONE]";
        case LogLevel::critical:
            return "[CRITICAL]";
        case LogLevel::error:
            return "[ERROR]";
        case LogLevel::info:
            return "[INFO]";
        case LogLevel::debug:
            return "[DEBUG]";
        default:
            return "[UNKNOWN]";
        }
    }

    static void log(LogLevel level, const std::string& str)
    {
        static std::mutex mutex;
        std::scoped_lock lock(mutex);
        std::cout << log_level_to_string(level) << " " << str << "\n";
    }
};

template <
    zx::threadpool_policy_pending_work pending_work_policy = zx::threadpool_policy_pending_work::wait_for_work_to_finish,
    zx::threadpool_policy_new_work new_work_policy = zx::threadpool_policy_new_work::configurable_and_forbidden_when_stopping>
using ThreadpoolConsoleTracing = zx::threadpool<pending_work_policy, new_work_policy, zx::threadpool_tracing_logger<ThreadpoolConsoleLogger>>;

int main()
{
    std::vector<std::optional<std::future<int>>> futures;
    std::vector<std::optional<std::future<int>>> futures2;

    float task_baseline_nanoseconds = 0;

    constexpr auto task_iterations = 1;
    constexpr auto thread_count = 1;
    constexpr auto task_count = 100000;
    futures.reserve(task_count);
    futures2.reserve(task_count);

    constexpr auto sleep_ms = 0;

    auto lambda = [&](int something) -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds{ sleep_ms });
        //++result;
        volatile int result = 0;
        for (int i = 0; i < task_iterations; ++i)
        {
            ++result;
        }
        return something;
    };

    {
        volatile int result = 0;
        auto start_time = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < task_count; ++i)
        {
            result += lambda(1);
        }
        const auto end_time = std::chrono::high_resolution_clock::now();
        task_baseline_nanoseconds = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count());
    }

    float baseline_nanoseconds = 0;

    {
        auto start_time = std::chrono::high_resolution_clock::now();

        {
            zx::threadpool<zx::threadpool_policy_pending_work::wait_for_work_to_finish> consumer(thread_count);
        }

        const auto end_time = std::chrono::high_resolution_clock::now();
        baseline_nanoseconds = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count());
    }

    std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();

    {
        ThreadpoolConsoleTracing<zx::threadpool_policy_pending_work::wait_for_work_to_finish> producer(1);
        ThreadpoolConsoleTracing<zx::threadpool_policy_pending_work::wait_for_work_to_finish> consumer(1);

        for (int i = 0; i < task_count; ++i)
        {
            futures.emplace_back(producer.push_job([&]() {
                auto maybe_future = consumer.push_job(lambda, 1);
                if (maybe_future)
                    return maybe_future->get();

                return 0;
            }));
        }

        consumer.wait_all();
        //producer.wait_all();
    }

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto nanoseconds = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count());
    const auto nanoseconds_diff = nanoseconds - baseline_nanoseconds;
    const auto milliseconds = nanoseconds / 1000000.0f;
    const auto milliseconds_diff = nanoseconds_diff / 1000000.0f;
    const auto baseline_milliseconds = baseline_nanoseconds / 1000000.0f;
    std::cout << "baseline is " << baseline_milliseconds << "ms\n";
    std::cout << "took " << milliseconds_diff << " ms (" << milliseconds << "ms - " << baseline_milliseconds << "ms)\n";
    const auto task_baseline_total_ms = task_baseline_nanoseconds / 1000000.0f;
    std::cout << "took " << milliseconds_diff - task_baseline_total_ms << "ms excluding task baseline of " << task_baseline_total_ms << "ms\n";
    const auto nanoseconds_per_task = nanoseconds_diff / static_cast<float>(task_count);
    const auto nanoseconds_per_task_diff = (nanoseconds_diff - task_baseline_nanoseconds) / static_cast<float>(task_count);
    std::cout << "baseline per-task processing time is " << std::fixed << task_baseline_nanoseconds / 1000000.0f / static_cast<float>(task_count) << "ms\n";
    std::cout << "which is " << std::fixed << nanoseconds_per_task / 1000000.0f << "ms per task inc. baseline\n";
    std::cout << "which is " << std::fixed << nanoseconds_per_task_diff / 1000000.0f << "ms per task\n";
    std::cout << "which is " << std::fixed << (nanoseconds_per_task_diff / 1000000.0f) * static_cast<float>(thread_count) << "ms per task * thread_count\n";

    int result = 0;
    for (auto& future : futures)
    {
        if (future)
            result += future.value().get();
    }
    return result;
}