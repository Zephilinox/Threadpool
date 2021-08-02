if (WIN32)
    message(WARNING "SanitizeLeaks will not work on windows")
    return()
endif()

add_compile_options(
    "-fsanitize=leak"
    "-fno-omit-frame-pointer"
    "-g"
    "-O1")
add_link_options("-fsanitize=leak")