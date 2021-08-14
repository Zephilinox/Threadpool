include(SetSystemIncludes)

CPMAddPackage(
    NAME function2
    GITHUB_REPOSITORY Naios/function2
    GIT_TAG 4.2.0
)

file(GLOB_RECURSE function2_SOURCES CONFIGURE_DEPENDS ${function2_SOURCE_DIR}/*.hpp ${function2_SOURCE_DIR}/*.h ${function2_SOURCE_DIR}/*.c ${function2_SOURCE_DIR}/*.cpp)

source_group(TREE ${function2_SOURCE_DIR} FILES ${function2_SOURCES})
set_target_properties(function2 PROPERTIES FOLDER dependencies)
set_target_includes_as_system(function2)