include(CodeCoverage)

function(configure_target target use_coverage)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 23)
    set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS OFF)

    if (use_coverage)
        target_code_coverage(${target})
    endif()

    get_target_property(target_SOURCES ${target} SOURCES)
    get_target_property(target_SOURCE_DIR ${target} SOURCE_DIR)
    get_target_property(target_CROSSCOMPILING_EMULATOR ${target} CROSSCOMPILING_EMULATOR)
    if (target_CROSSCOMPILING_EMULATOR)
        message(STATUS "Configured target '${target}' has crosscompiling emulator: ${target_CROSSCOMPILING_EMULATOR}")
    endif()
    
    if ((NOT target_CROSSCOMPILING_EMULATOR) AND (CMAKE_CROSSCOMPILING_EMULATOR MATCHES "valgrind"))
        message(WARNING "Configured target '${target}' is not using valgrind: ${target_CROSSCOMPILING_EMULATOR}")
    endif()

    source_group(TREE ${target_SOURCE_DIR} FILES ${target_SOURCES})
endfunction()
