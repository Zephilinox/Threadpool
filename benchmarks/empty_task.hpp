#pragma once

//SELF

//LIBS
#include <benchmark/benchmark.h>
#include <threadpool/threadpool.hpp>

//STD

static void empty_task()
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
    zx::threadpool<zx::threadpool_policy_pending_work::leave_work_unfinished> threadpool(0);
    zx::threadpool producers(state.range(0));
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
    zx::threadpool threadpool(state.range(0));
    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < empty_task_count; ++i)
            threadpool.push_task(&empty_task);

        threadpool.wait_all();
    }

    state.SetItemsProcessed(state.iterations() * empty_task_count);
}

static auto benchmark_threadpool_empty_job_push(benchmark::State& state) -> void
{
    zx::threadpool<zx::threadpool_policy_pending_work::leave_work_unfinished> threadpool(0);
    zx::threadpool producers(state.range(0));
    auto produce = [&threadpool]() {
        threadpool.push_task(&empty_task);
    };

    std::vector<std::optional<std::future<void>>> futures;
    futures.reserve(empty_task_count);

    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < empty_task_count; ++i)
            futures.emplace_back(producers.push_job(produce));

        for (int i = 0; i < empty_task_count; ++i)
            futures[i]->wait();
    }

    state.SetItemsProcessed(state.iterations() * empty_task_count * 2 /* once for the producers, and again for what they push*/);
}

static auto benchmark_threadpool_empty_job_execute(benchmark::State& state) -> void
{
    zx::threadpool threadpool(state.range(0));

    std::vector<std::optional<std::future<void>>> futures;
    futures.reserve(empty_task_count);

    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        for (int i = 0; i < empty_task_count; ++i)
            futures.emplace_back(threadpool.push_job(&empty_task));

        for (int i = 0; i < empty_task_count; ++i)
            futures[i]->wait();
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
BENCHMARK(benchmark_threadpool_empty_job_push)
    ->Args({ 1 })
    ->Args({ 2 })
    ->Args({ 4 })
    ->Args({ 8 })
    ->Args({ 16 })
    ->Args({ 32 })
    ->UseRealTime();
BENCHMARK(benchmark_threadpool_empty_job_execute)
    ->Args({ 1 })
    ->Args({ 2 })
    ->Args({ 4 })
    ->Args({ 8 })
    ->Args({ 16 })
    ->Args({ 32 })
    ->UseRealTime();