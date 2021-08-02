if (WIN32)
    message(WARNING "SanitizeThreads will not work on windows")
    return()
endif()

add_compile_options(
    "-fsanitize=thread"
    "-g"
    "-O1")
add_link_options("-fsanitize=thread")