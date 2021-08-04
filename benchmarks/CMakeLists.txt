include(FetchGoogleBenchmark)
include(FetchASIO)
include(ConfigureTarget)

add_executable(threadpool_benchmarks "main.cpp"
    "empty_task.hpp"
    "heavy_task.hpp"
    "sleepy_task.hpp"
    "asio_bench.hpp"
)

configure_target(threadpool_benchmarks)

target_link_libraries(threadpool_benchmarks
    PRIVATE
        threadpool::threadpool
        benchmark::benchmark
        asio::asio
)

add_custom_target(threadpool_benchmarks_execute
    COMMAND "$<TARGET_FILE:threadpool_benchmarks>"
)