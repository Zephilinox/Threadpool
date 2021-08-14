include(FetchGoogleBenchmark)
include(FetchASIO)
include(ConfigureTarget)
include(CompilerWarnings)

add_executable(threadpool_benchmarks "main.cpp"
    "empty_task.hpp"
    "heavy_task.hpp"
    "sleepy_task.hpp"
    "asio_bench.hpp"
)

configure_target(threadpool_benchmarks ${THREADPOOL_CODE_COVERAGE})
set_target_warnings(threadpool_benchmarks ${THREADPOOL_WARNINGS_AS_ERRORS})

target_link_libraries(threadpool_benchmarks
    PRIVATE
        zx::threadpool
        benchmark::benchmark
        asio::asio
)

add_custom_target(threadpool_benchmarks_execute
    COMMAND "$<TARGET_FILE:threadpool_benchmarks>"
)