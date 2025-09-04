find_program(VALGRIND valgrind)
if (NOT VALGRIND)
    message(SEND_ERROR "Valgrind could not be found")
endif()

if (CMAKE_CROSSCOMPILING_EMULATOR)
    message(SEND_ERROR "Valgrind can not be used with a cross-compiling emulator, as we will set it as the emulator")
endif()

if (NOT CMAKE_CROSSCOMPILING)
    set(CMAKE_CROSSCOMPILING ON)
endif()

set(CMAKE_CROSSCOMPILING_EMULATOR "${VALGRIND}"
    "--leak-check=full"
    "--show-leak-kinds=all"
    "--track-origins=yes"
    "--error-exitcode=1"
)

set(MEMORYCHECK_COMMAND "${VALGRIND}")
set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full"
    "--show-leak-kinds=all"
    "--track-origins=yes"
    "--error-exitcode=1")

set(CTEST_MEMORYCHECK_COMMAND "${MEMORYCHECK_COMMAND}")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS}")
