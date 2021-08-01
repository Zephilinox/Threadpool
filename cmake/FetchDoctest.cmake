CPMAddPackage(
    NAME doctest
    GITHUB_REPOSITORY onqtam/doctest
    GIT_TAG 2.4.5
    EXCLUDE_FROM_ALL "YES"
    OPTIONS
        DOCTEST_WITH_MAIN_IN_STATIC_LIB "OFF"
)

set_target_properties(doctest PROPERTIES FOLDER dependencies)