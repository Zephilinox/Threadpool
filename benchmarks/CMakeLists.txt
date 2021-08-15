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

# Don't use the crosscompiling executor, if it's valgrind they will be very slow.
# We'll rely on static analysis for benchmark leaks. in theory, tests should cover it.
# Also, googlebenchmark has an intentional memory leak, so we'd need to supress it.
add_custom_target(threadpool_benchmarks_execute
    COMMAND "$<TARGET_FILE:threadpool_benchmarks>"
)