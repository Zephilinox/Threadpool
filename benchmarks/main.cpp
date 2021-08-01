//SELF
#include "empty_task.hpp"
#include "asio_bench.hpp"
#include "heavy_task.hpp"
#include "sleepy_task.hpp"

//LIBS
#include <benchmark/benchmark.h>

auto main(int argc, char** argv) -> int
{
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();

    return 0;
}