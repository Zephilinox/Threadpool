include(ConfigureTarget)

add_executable(threadpool_example_testapp
    "testapp.cpp"
)

configure_target(threadpool_example_testapp ${THREADPOOL_CODE_COVERAGE})

target_link_libraries(threadpool_example_testapp
    PRIVATE
        zx::threadpool
)