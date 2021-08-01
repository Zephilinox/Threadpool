CPMAddPackage(
    NAME benchmark
    GITHUB_REPOSITORY google/benchmark
    VERSION 1.5.2
    OPTIONS "BENCHMARK_ENABLE_TESTING OFF"
    EXCLUDE_FROM_ALL YES
)

set_property(TARGET benchmark PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET benchmark PROPERTY CXX_STANDARD 17)
set_property(TARGET benchmark PROPERTY CXX_EXTENSIONS OFF)

set_target_properties(benchmark PROPERTIES FOLDER dependencies)