cmake_minimum_required(VERSION 3.14)

project(threadpool)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckIsRoot)
check_project_is_root(THREADPOOL_IS_ROOT)

option(THREADPOOL_EXAMPLES "Build the threadpool examples" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_TESTS "Build the threadpool tests" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_BENCHMARKS "Build the threadpool benchmarks" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_INSTALL "Enable cmake install for the threadpool" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_DOWNLOAD_CPM "Download CPM. Disable if you're providing your own version of CPM" ON)
option(THREADPOOL_CLANG_FORMAT_CHECK "Adds the threadpool_clang_format_check target" ${THREADPOOL_IS_ROOT})
option(THREADPOOL_CLANG_FORMAT_FIX "Adds the threadpool_clang_format_fix target" ${THREADPOOL_IS_ROOT})
option(CMAKE_EXPORT_COMPILE_COMMANDS "Exports compile commands (if available)" ${THREADPOOL_IS_ROOT})

if (THREADPOOL_IS_ROOT)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if (THREADPOOL_DOWNLOAD_CPM)
    include(DownloadCPM)
endif()

add_library(threadpool INTERFACE "include/thread_pool/Threadpool.hpp")
target_include_directories(threadpool INTERFACE "include"
    $<BUILD_INTERFACE:${threadpool_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(threadpool INTERFACE cxx_std_17)
if (UNIX AND NOT APPLE)
    target_link_libraries(threadpool INTERFACE "m" "atomic" "pthread")
endif () 

include(ConfigureTarget)
configure_target(threadpool)

if (THREADPOOL_INSTALL)
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
        message(ERROR "Failed to find clang-format")
    endif()

    find_package(Python3)
    if (NOT Python3_FOUND)
        message(ERROR "Failed to find python3")
    endif()
endif()

set(THREADPOOL_CLANG_FORMAT_COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-clang-format.py)
set(THREADPOOL_CLANG_FORMAT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/examples"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks"
)

if (THREADPOOL_CLANG_FORMAT_CHECK)
    add_custom_target(threadpool_clang_format_check
            COMMAND ${THREADPOOL_CLANG_FORMAT_COMMAND} -r ${THREADPOOL_CLANG_FORMAT_SOURCES})
endif()

if (THREADPOOL_CLANG_FORMAT_FIX)
    add_custom_target(threadpool_clang_format_fix
        COMMAND ${THREADPOOL_CLANG_FORMAT_COMMAND} -r -i ${THREADPOOL_CLANG_FORMAT_SOURCES})
endif()