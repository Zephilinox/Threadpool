//SELF

//LIBS
#include <benchmark/benchmark.h>
#include <thread_pool/Threadpool.hpp>

//STD

void empty_task()
{
}

constexpr auto empty_task_count = 10000;

static auto benchmark_empty_task_execute(benchmark::State& state) -> void
{
    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < empty_task_count; ++i)
            empty_task();
    }

    state.SetItemsProcessed(state.iterations() * empty_task_count);
}

static auto benchmark_empty_task_std_function_execute(benchmark::State& state) -> void
{
    const std::function<void()> empty_task_func = empty_task;
    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < empty_task_count; ++i)
            empty_task_func();
    }

    state.SetItemsProcessed(state.iterations() * empty_task_count);
}

static auto benchmark_threadpool_empty_task_push(benchmark::State& state) -> void
{
    Threadpool<ThreadpoolPolicyPendingWork::leave_work_unfinished> threadpool(0);
    Threadpool producers(state.range(0));
    auto produce = [&threadpool]() {
        threadpool.push_task(&empty_task);
    };

    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < empty_task_count; ++i)
            producers.push_task(produce);

        producers.wait_all();
    }

    state.SetItemsProcessed(state.iterations() * empty_task_count * 2 /* once for the producers, and again for what they push*/);
}

static auto benchmark_threadpool_empty_task_execute(benchmark::State& state) -> void
{
    Threadpool threadpool(state.range(0));
    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < empty_task_count; ++i)
            threadpool.push_task(&empty_task);

        threadpool.wait_all();
    }

    state.SetItemsProcessed(state.iterations() * empty_task_count);
}

BENCHMARK(benchmark_empty_task_execute)
    ->UseRealTime();
BENCHMARK(benchmark_empty_task_std_function_execute)
    ->UseRealTime();
BENCHMARK(benchmark_threadpool_empty_task_push)
    ->Args({ 1 })
    ->Args({ 2 })
    ->Args({ 4 })
    ->Args({ 8 })
    ->Args({ 16 })
    ->Args({ 32 })
    ->UseRealTime();
BENCHMARK(benchmark_threadpool_empty_task_execute)
    ->Args({ 1 })
    ->Args({ 2 })
    ->Args({ 4 })
    ->Args({ 8 })
    ->Args({ 16 })
    ->Args({ 32 })
    ->UseRealTime();