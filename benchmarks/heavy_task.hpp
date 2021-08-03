#pragma once

//SELF

//LIBS
#include <benchmark/benchmark.h>
#include <threadpool/threadpool.hpp>

//STD
#include <numeric>

constexpr auto heavy_task_count = 100;

static void heavy_task()
{
    std::vector<float> vec(32);
    for (int i = 0; i < 32; ++i)
        vec[i] = (static_cast<float>(std::rand() % 100000) / 100000.0f);

    volatile auto heavy_task_result = std::accumulate(vec.begin(), vec.end(), 0.0f);
}

static auto benchmark_heavy_task_execute(benchmark::State& state) -> void
{
    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < heavy_task_count; ++i)
            heavy_task();
    }

    state.SetItemsProcessed(state.iterations() * heavy_task_count);
}

static auto benchmark_heavy_task_std_function_execute(benchmark::State& state) -> void
{
    const std::function<void()> heavy_task_func = heavy_task;
    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < heavy_task_count; ++i)
            heavy_task_func();
    }

    state.SetItemsProcessed(state.iterations() * heavy_task_count);
}

static auto benchmark_threadpool_heavy_task_push(benchmark::State& state) -> void
{
    zx::threadpool<zx::threadpool_policy_pending_work::leave_work_unfinished> threadpool(0);
    zx::threadpool producers(state.range(0));
    auto produce = [&threadpool]() {
        threadpool.push_task(&heavy_task);
    };

    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < heavy_task_count; ++i)
            producers.push_task(produce);

        producers.wait_all();
    }

    state.SetItemsProcessed(state.iterations() * heavy_task_count * 2 /* once for the producers, and again for what they push*/);
}

static auto benchmark_threadpool_heavy_task_execute(benchmark::State& state) -> void
{
    zx::threadpool threadpool(state.range(0));
    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < heavy_task_count; ++i)
            threadpool.push_task(&heavy_task);

        threadpool.wait_all();
    }

    state.SetItemsProcessed(state.iterations() * heavy_task_count);
}

BENCHMARK(benchmark_heavy_task_execute)
    ->UseRealTime();
BENCHMARK(benchmark_heavy_task_std_function_execute)
    ->UseRealTime();
BENCHMARK(benchmark_threadpool_heavy_task_push)
    ->Args({ 1 })
    ->Args({ 2 })
    ->Args({ 4 })
    ->Args({ 8 })
    ->Args({ 16 })
    ->Args({ 32 })
    ->UseRealTime();
BENCHMARK(benchmark_threadpool_heavy_task_execute)
    ->Args({ 1 })
    ->Args({ 2 })
    ->Args({ 4 })
    ->Args({ 8 })
    ->Args({ 16 })
    ->Args({ 32 })
    ->UseRealTime();