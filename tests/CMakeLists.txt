include(FetchDoctest)
include(FetchFunction2)
include(ConfigureTarget)

add_executable(threadpool_tests
    "main.cpp"
    "threadpool.cpp"
)

configure_target(threadpool_tests)

target_link_libraries(threadpool_tests
    PRIVATE
        threadpool::threadpool
        doctest::doctest
        function2::function2
)

include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")
doctest_discover_tests(threadpool_tests JUNIT_OUTPUT_DIR ".")