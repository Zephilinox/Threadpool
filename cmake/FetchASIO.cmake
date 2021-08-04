CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-19-2
    DOWNLOAD_ONLY
)

file(GLOB_RECURSE asio_SOURCES CONFIGURE_DEPENDS ${asio_SOURCE_DIR}/*.hpp ${asio_SOURCE_DIR}/*.h ${asio_SOURCE_DIR}/*.c ${asio_SOURCE_DIR}/*.cpp)
add_library(asio INTERFACE ${asio_SOURCES})
add_library(asio::asio ALIAS asio)
target_include_directories(asio INTERFACE
    "${asio_SOURCE_DIR}/asio/include"
)
target_compile_features(asio INTERFACE cxx_std_17)
if (UNIX AND NOT APPLE)
    target_link_libraries(asio INTERFACE "m" "atomic" "pthread")
endif () 

source_group(TREE ${asio_SOURCE_DIR} FILES ${asio_SOURCES})
set_target_properties(asio PROPERTIES FOLDER dependencies)