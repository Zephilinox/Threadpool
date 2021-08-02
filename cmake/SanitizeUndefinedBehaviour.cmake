if (WIN32)
    message(WARNING "SanitizeUndefinedBehaviour will not work on windows")
    return()
endif()

add_compile_options("-fsanitize=undefined")
add_link_options("-fsanitize=undefined")