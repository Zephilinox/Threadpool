cmake_minimum_required(VERSION 3.14)

project(threadpool)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckIsRoot)
check_project_is_root(THREADPOOL_IS_ROOT)

option(THREADPOOL_CI "We are running on CI. May change certain defaults." OFF)
if (THREADPOOL_CI)
    message(STATUS "Threadpool running on CI")
endif()

option(THREADPOOL_EXAMPLES "Build the threadpool examples" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_TESTS "Build the threadpool tests" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_BENCHMARKS "Build the threadpool benchmarks" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_INSTALL "Enable cmake install for the threadpool" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_DOWNLOAD_CPM "Download CPM. Disable if you're providing your own version of CPM" ON)
option(THREADPOOL_CLANG_FORMAT_CHECK "Adds the threadpool_clang_format_check target" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_CLANG_FORMAT_FIX "Adds the threadpool_clang_format_fix target" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_CPPCHECK "Adds the threadpool_cppcheck target" OFF)
option(THREADPOOL_CLANG_TIDY_CHECK "Adds the threadpool_clang_tidy_check target" OFF)
option(THREADPOOL_ASAN "Build with ASAN" OFF)
option(THREADPOOL_LSAN "Build with LSAN" OFF)
option(THREADPOOL_TSAN "Build with TSAN" OFF)
option(THREADPOOL_UBSAN "Build with UBSAN" OFF)
option(THREADPOOL_CODE_COVERAGE "Enable code coverage" OFF)
option(THREADPOOL_WARNINGS_AS_ERRORS "Treat warnings as errors" ${THREADPOOL_CI})
option(THREADPOOL_VALGRIND "Use valgrind to run all executables" OFF)

if (THREADPOOL_IS_ROOT)
    message(STATUS "Threadpool is root")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if (THREADPOOL_DOWNLOAD_CPM)
    message(STATUS "Threadpool CPM Download Enabled")
    include(DownloadCPM)
endif()

if (THREADPOOL_ASAN)
    message(STATUS "Threadpool ASAN Enabled")
    include(SanitizeAddress)
endif()

if (THREADPOOL_LSAN)
    message(STATUS "Threadpool LSAN Enabled")
    include(SanitizeLeaks)
endif()

if (THREADPOOL_TSAN)
    message(STATUS "Threadpool TSAN Enabled")
    include(SanitizeThreads)
endif()

if (THREADPOOL_UBSAN)
    message(STATUS "Threadpool UBSAN Enabled")
    include(SanitizeUndefinedBehaviour)
endif()

if (THREADPOOL_VALGRND)
    message(STATUS "Threadpool Valgrind Enabled")
    include(Valgrind)
endif()

add_library(threadpool INTERFACE
    "include/threadpool/threadpool.hpp"
    "include/threadpool/tracers/tracing_nothing.hpp"
    "include/threadpool/tracers/tracing_logger.hpp"
    "include/threadpool/tracers/tracing_console_logger.hpp"
)
add_library(zx::threadpool ALIAS threadpool)
target_include_directories(threadpool INTERFACE
    $<BUILD_INTERFACE:${threadpool_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(threadpool INTERFACE cxx_std_17)
if (UNIX AND NOT APPLE)
    target_link_libraries(threadpool INTERFACE "m" "atomic" "pthread")
endif () 

include(ConfigureTarget)
configure_target(threadpool OFF)

if (THREADPOOL_INSTALL)
    message(STATUS "Threadpool Install Enabled")
    install(
        DIRECTORY "${threadpool_SOURCE_DIR}/include"
        DESTINATION "."
    )
endif()


if (THREADPOOL_EXAMPLES)
    message(STATUS "Threadpool Examples Enabled")
    add_subdirectory(examples)
endif()

if (THREADPOOL_TESTS)
    message(STATUS "Threadpool Tests Enabled")
    enable_testing()
    add_subdirectory(tests)
endif()

if (THREADPOOL_BENCHMARKS)
    message(STATUS "Threadpool Benchmarks Enabled")
    add_subdirectory(benchmarks)
endif()

if (THREADPOOL_CLANG_FORMAT_CHECK OR THREADPOOL_CLANG_FORMAT_FIX)
    find_program(CLANG_FORMAT clang-format)
    if (NOT CLANG_FORMAT)
        message(SEND_ERROR "Failed to find clang-format")
    endif()

    find_package(Python3)
    if (NOT Python3_FOUND)
        message(SEND_ERROR "Failed to find python3")
    endif()
endif()

set(THREADPOOL_CLANG_FORMAT_COMMAND
    "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-clang-format.py"
)
set(THREADPOOL_LINT_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/examples"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks"
)

if (THREADPOOL_CLANG_FORMAT_CHECK)
    message(STATUS "Threadpool clang-format check Enabled")
    add_custom_target(threadpool_clang_format_check
        COMMAND "${THREADPOOL_CLANG_FORMAT_COMMAND}" "-r" "${THREADPOOL_LINT_PATHS}")
endif()

if (THREADPOOL_CLANG_FORMAT_FIX)
    message(STATUS "Threadpool clang-format fix Enabled")
    add_custom_target(threadpool_clang_format_fix
        COMMAND "${THREADPOOL_CLANG_FORMAT_COMMAND}" "-r" "-i" "${THREADPOOL_LINT_PATHS}")
endif()

if (THREADPOOL_CPPCHECK)
    message(STATUS "Threadpool cppcheck Enabled")
    include(CPPCheck)
    cppcheck_sources(threadpool_cppcheck
        "${THREADPOOL_LINT_PATHS}"
    )
endif()

if (THREADPOOL_CLANG_TIDY_CHECK)
    message(STATUS "Threadpool clang-tidy check Enabled")
    include(ClangTidy)
    create_clang_tidy_check(threadpool_clang_tidy_check)
endif()