include(FetchDoctest)
include(ConfigureTarget)

add_executable(threadpool_tests
    "main.cpp"
)

configure_target(threadpool_tests)

target_link_libraries(threadpool_tests
    PRIVATE
        threadpool
        doctest::doctest
)

include("${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake")
doctest_discover_tests(threadpool_tests JUNIT_OUTPUT_DIR ".")