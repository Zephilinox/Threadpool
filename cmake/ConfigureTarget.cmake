function(configure_target target)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS OFF)

    get_target_property(target_SOURCES ${target} SOURCES)
    get_target_property(target_SOURCE_DIR ${target} SOURCE_DIR)
    source_group(TREE ${target_SOURCE_DIR} FILES ${target_SOURCES})
endfunction()