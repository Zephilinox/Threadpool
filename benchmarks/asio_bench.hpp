#pragma once

//SELF
#include "empty_task.hpp"

//LIBS
#include <benchmark/benchmark.h>
#include <threadpool/threadpool.hpp>
#include <asio.hpp>

//STD
#include <iostream>

using work_guard_type = asio::executor_work_guard<asio::io_context::executor_type>;

static auto benchmark_asio_empty_task_push(benchmark::State& state) -> void
{
    asio::io_context consumer_context;
    asio::io_context producer_context;

    std::vector<std::thread> producers;
    std::atomic<bool> stop = false;
    ;
    for (int i = 0; i < state.range(0); ++i)
    {
        producers.emplace_back([&]() {
            while (!stop) //kinda hacky, but close enough
                producer_context.run();
        });
    }

    auto consumer = std::thread([&]() {
        consumer_context.run();
    });

    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        work_guard_type work_guard_producer(producer_context.get_executor());

        for (int i = 0; i < empty_task_count; ++i)
        {
            asio::post(producer_context, [&]() {
                asio::post(consumer_context, &empty_task);
            });
        }

        //make sure we block until it has posted all the work to the consumer
        work_guard_producer.reset();
    }

    stop = true;

    producer_context.stop();
    //let consumer exit without executing jobs
    consumer_context.stop();

    for (auto& thread : producers)
        thread.join();

    consumer.join();

    state.SetItemsProcessed(state.iterations() * empty_task_count * 2);
}

static auto benchmark_asio_empty_task_execute(benchmark::State& state) -> void
{
    asio::io_context context;

    std::vector<std::thread> threads;
    std::atomic<bool> stop = false;
    ;
    for (int i = 0; i < state.range(0); ++i)
    {
        threads.emplace_back([&]() {
            while (!stop) //kinda hacky, but close enough
                context.run();
        });
    }

    for (auto _ : state) // NOLINT(clang-analyzer-deadcode.DeadStores)
    {
        work_guard_type work_guard(context.get_executor());

        for (int i = 0; i < empty_task_count; ++i)
            asio::post(context, &empty_task);

        //finish all work
        work_guard.reset();
    }

    stop = true;

    for (auto& thread : threads)
        thread.join();

    state.SetItemsProcessed(state.iterations() * empty_task_count);
}

BENCHMARK(benchmark_asio_empty_task_push)
    ->Args({ 1 })
    ->Args({ 2 })
    ->Args({ 4 })
    ->Args({ 8 })
    ->Args({ 16 })
    ->Args({ 32 })
    ->UseRealTime();
BENCHMARK(benchmark_asio_empty_task_execute)
    ->Args({ 1 })
    ->Args({ 2 })
    ->Args({ 4 })
    ->Args({ 8 })
    ->Args({ 16 })
    ->Args({ 32 })
    ->UseRealTime();