include(ConfigureTarget)

add_executable(threadpool_example_testapp
    "testapp.cpp"
)

configure_target(threadpool_example_testapp)

target_link_libraries(threadpool_example_testapp
    PRIVATE
        threadpool
)