include(FetchGoogleBenchmark)
include(ConfigureTarget)

add_executable(threadpool_benchmarks "main.cpp"
    "empty_task.hpp"
    "heavy_task.hpp"
    "sleepy_task.hpp"
)

configure_target(threadpool_benchmarks)

target_link_libraries(threadpool_benchmarks
    PRIVATE
        threadpool
        benchmark::benchmark
)

add_custom_target(threadpool_benchmarks_execute
    COMMAND "$<TARGET_FILE:threadpool_benchmarks>"
)