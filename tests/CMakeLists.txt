include(FetchDoctest)
include(FetchFunction2)
include(ConfigureTarget)
include(CompilerWarnings)

add_executable(threadpool_tests
    "main.cpp"
    "threadpool.cpp"
)

configure_target(threadpool_tests ${THREADPOOL_CODE_COVERAGE})
set_target_warnings(threadpool_tests ${THREADPOOL_WARNINGS_AS_ERRORS})

target_link_libraries(threadpool_tests
    PRIVATE
        zx::threadpool
        doctest::doctest
        function2::function2
)

include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")
#doctest_discover_tests(threadpool_tests JUNIT_OUTPUT_DIR ".")
doctest_discover_tests(threadpool_tests)