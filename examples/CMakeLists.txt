include(ConfigureTarget)
include(CompilerWarnings)

add_executable(threadpool_example_testapp
    "testapp.cpp"
)

configure_target(threadpool_example_testapp ${THREADPOOL_CODE_COVERAGE})
set_target_warnings(threadpool_example_testapp ${THREADPOOL_WARNINGS_AS_ERRORS})

target_link_libraries(threadpool_example_testapp
    PRIVATE
        zx::threadpool
)