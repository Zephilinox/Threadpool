//SELF

//LIBS
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

//STD

auto main(int argc, char** argv) -> int
{
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    context.setOption("no-breaks", true);
    context.setOption("--version", true);
    context.setOption("--count", true);
    context.setOption("--list-test-cases", true);
    context.setOption("--list-test-suites", true);
    context.setOption("--success", false);
    context.setOption("--exit", true);

    const int result = context.run();
    if (context.shouldExit())
        return result;

    return result;
}