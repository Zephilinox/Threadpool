//SELF

//LIBS
#include <doctest/doctest.h>
#include <threadpool/threadpool.hpp>
#include <threadpool/tracers/tracing_console_logger.hpp>
#include <function2/function2.hpp>

//STD
#include <memory>
#include <iostream>

struct ExpensiveType
{
    ExpensiveType() = default;
    ExpensiveType(const ExpensiveType&)
    {
        CHECK(false);
        copy_count++;
    }
    ExpensiveType& operator=(const ExpensiveType&)
    {
        CHECK(false);
        copy_count++;
        return *this;
    }

    int copy_count = 0;
};

struct NormalFunctor
{
    NormalFunctor() = default;

    int operator()()
    {
        CHECK_EQ(five, 5);
        return five;
    }

    int five = 5;
};

template <typename MoveOnlyParam>
struct NormalFunctorMoveOnlyParam
{
    NormalFunctorMoveOnlyParam() = default;

    int operator()(MoveOnlyParam five)
    {
        CHECK_EQ(*five, 5);
        return *five;
    }
};

struct ConstOnlyFunctor
{
    ConstOnlyFunctor() = default;

    const int& operator()() const
    {
        CHECK_EQ(five, 5);
        return five;
    }

    int five = 5;
};

struct MoveOnlyFunctor
{
    MoveOnlyFunctor() = default;

    std::unique_ptr<int> operator()() &&
    {
        CHECK_EQ(*five, 5);
        return std::move(five);
    }

    std::unique_ptr<int> five = std::make_unique<int>(5);
};

struct CounterState
{
    bool operator==(const CounterState& rhs) const
    {
        return constructor == rhs.constructor
            && copy_constructor == rhs.copy_constructor
            && move_constructor == rhs.move_constructor
            && copy_assign == rhs.copy_assign
            && move_assign == rhs.move_assign
            && destructor == rhs.destructor
            && rvalue_call == rhs.rvalue_call
            && lvalue_call == rhs.lvalue_call
            && const_lvalue_call == rhs.const_lvalue_call
            && const_rvalue_call == rhs.const_rvalue_call;
    }
    unsigned int constructor = 0;
    unsigned int copy_constructor = 0;
    unsigned int move_constructor = 0;
    unsigned int copy_assign = 0;
    unsigned int move_assign = 0;
    unsigned int destructor = 0;
    unsigned int rvalue_call = 0;
    unsigned int lvalue_call = 0;
    unsigned int const_lvalue_call = 0;
    unsigned int const_rvalue_call = 0;
};

struct CounterFunctor
{
    CounterFunctor(std::string n, CounterState* state)
        : name(n + " ")
        , state(state)
    {
        //std::cout << name << "construct CounterFunctor\n";
        state->constructor++;
    }

    ~CounterFunctor()
    {
        //std::cout << name << "destroy CounterFunctor\n";
        state->destructor++;
    }

    CounterFunctor(const CounterFunctor& rhs)
    {
        name = rhs.name;
        state = rhs.state;
        state->copy_constructor++;
        //std::cout << name << "copy construct CounterFunctor\n";
    }

    CounterFunctor(CounterFunctor&& rhs) noexcept
    {
        name = rhs.name;
        state = rhs.state;
        state->move_constructor++;
        //std::cout << name << "move construct CounterFunctor\n";
    }

    CounterFunctor& operator=(const CounterFunctor& rhs)
    {
        name = rhs.name;
        state = rhs.state;
        state->copy_assign++;
        //std::cout << name << "copy assign CounterFunctor\n";
        return *this;
    }

    CounterFunctor& operator==(CounterFunctor&& rhs) noexcept
    {
        name = rhs.name;
        state = rhs.state;
        state->move_assign++;
        //std::cout << name << "move assign CounterFunctor\n";
        return *this;
    }

    template <typename T2>
    void operator()(T2&& t2) &
    {
        //std::cout << name << "called CounterFunctor&\n";
        state->lvalue_call++;
    }

    template <typename T2>
    void operator()(T2&& t2) &&
    {
        //std::cout << name << "called CounterFunctor&& \n";
        state->rvalue_call++;
    }

    template <typename T2>
    void operator()(T2&& t2) const&
    {
        //std::cout << name << "called const CounterFunctor&\n";
        state->const_lvalue_call++;
    }

    template <typename T2>
    void operator()(T2&& t2) const&&
    {
        //std::cout << name << "called const CounterFunctor&\n";
        state->const_rvalue_call++;
    }

    std::string name;
    CounterState* state;
};

template <
    zx::threadpool_policy_pending_work A = zx::threadpool_policy_pending_work::wait_for_work_to_finish,
    zx::threadpool_policy_new_work B = zx::threadpool_policy_new_work::configurable_and_forbidden_when_stopping,
    typename C = void>
using threadpool_function2 = zx::threadpool<A, B, void, fu2::unique_function<void()>>;

TEST_SUITE("Tasks")
{
    TEST_CASE("Ensure functors and arguments don't copy unexpectedly")
    {
        CounterState argument;
        CounterState function;
        zx::threadpool pool(1);
        auto logger1 = CounterFunctor("argument", &argument);
        auto logger2 = CounterFunctor("function", &function);
        pool.push_task(std::move(logger2), std::move(logger1));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 1);
        CHECK_EQ(function.rvalue_call, 1);

        CounterState function_without_call = function;
        --function_without_call.rvalue_call;
        CHECK_EQ(argument, function_without_call);
        CHECK_EQ(argument.constructor, 1);
        CHECK_EQ(argument.move_constructor, 6); //note: reductions are good, but wanna know when they happen
        CHECK_EQ(argument.destructor, 6); //note: reductions are good, but wanna know when they happen
        CHECK_EQ(argument.copy_constructor, 0);
        CHECK_EQ(argument.copy_assign, 0);
        CHECK_EQ(argument.move_assign, 0);
        CHECK_EQ(argument.lvalue_call, 0);
        CHECK_EQ(argument.const_lvalue_call, 0);
        CHECK_EQ(argument.const_rvalue_call, 0);
    }

    TEST_CASE("Lambdas with no captures, returns, or parameters")
    {
        zx::threadpool pool(1);
        pool.push_task([]() mutable {});
        pool.push_task([]() mutable noexcept {});
        pool.push_task([]() constexpr {});
        pool.push_task([]() constexpr noexcept {});
        pool.push_task([]() {});

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 5);
    }

    TEST_CASE("Lambdas that return with no captures")
    {
        zx::threadpool pool(1);
        pool.push_task([]() mutable { return 5; });
        pool.push_task([]() mutable noexcept { return 5; });
        pool.push_task([]() constexpr { return 5; });
        pool.push_task([]() constexpr noexcept { return 5; });
        pool.push_task([]() { return 5; });

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 5);
    }

    TEST_CASE("Lambdas that return copyable value capture")
    {
        zx::threadpool pool(1);
        int five = 5;
        pool.push_task([five]() mutable { CHECK_EQ(five, 5); return five; });
        pool.push_task([five]() mutable noexcept { CHECK_EQ(five, 5); return five; });
        pool.push_task([five]() constexpr { return five; });
        pool.push_task([five]() constexpr noexcept { return five; });
        pool.push_task([five]() { CHECK_EQ(five, 5); return five; });

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 5);
    }

    TEST_CASE("Lambdas that return copyable reference capture")
    {
        zx::threadpool pool(1);
        int five = 5;
        pool.push_task([&five]() mutable { CHECK_EQ(five, 5); return five; });
        pool.push_task([&five]() mutable noexcept { CHECK_EQ(five, 5); return five; });
        pool.push_task([&five]() constexpr { return five; });
        pool.push_task([&five]() constexpr noexcept { return five; });
        pool.push_task([&five]() { CHECK_EQ(five, 5); return five; });

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 5);
    }

    TEST_CASE("Lambdas can have unique_ptr arguments")
    {
        threadpool_function2<> pool(1);

        pool.push_task([](std::unique_ptr<int> five) mutable { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](std::unique_ptr<int> five) mutable noexcept { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](std::unique_ptr<int> five) { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 3);
    }

    TEST_CASE("Lambdas can have unique_ptr&& arguments")
    {
        threadpool_function2<> pool(1);

        pool.push_task([](std::unique_ptr<int>&& five) mutable { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](std::unique_ptr<int>&& five) mutable noexcept { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](std::unique_ptr<int>&& five) { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 3);
    }

    TEST_CASE("Lambdas can have auto unique_ptr arguments")
    {
        threadpool_function2<> pool(1);

        pool.push_task([](auto five) mutable { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](auto five) mutable noexcept { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](auto five) { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 3);
    }

    TEST_CASE("Lambdas can have auto&& unique_ptr arguments")
    {
        threadpool_function2<> pool(1);

        pool.push_task([](auto&& five) mutable { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](auto&& five) mutable noexcept { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](auto&& five) { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 3);
    }

    TEST_CASE("Function2 Lambdas can have unique_ptr captures")
    {
        threadpool_function2<> pool(1);

        auto five = std::make_unique<int>(5);
        pool.push_task([five = std::move(five)]() mutable { CHECK_EQ(*five, 5); return *five; });
        five = std::make_unique<int>(5);
        pool.push_task([five = std::move(five)]() mutable noexcept { CHECK_EQ(*five, 5); return *five; });
        five = std::make_unique<int>(5);
        pool.push_task([five = std::move(five)]() { CHECK_EQ(*five, 5); return *five; });

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 3);
    }

    TEST_CASE("Function2 Lambdas can move unique_ptr captures")
    {
        threadpool_function2<> pool(1);

        auto five = std::make_unique<int>(5);
        pool.push_task([five = std::move(five)]() mutable { CHECK_EQ(*five, 5); return std::move(five); });
        five = std::make_unique<int>(5);
        pool.push_task([five = std::move(five)]() mutable noexcept { CHECK_EQ(*five, 5); return std::move(five); });

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 2);
    }

    TEST_CASE("Function2 MoveOnlyFunctor is valid")
    {
        threadpool_function2<> pool(1);

        MoveOnlyFunctor mof;
        pool.push_task(std::move(mof));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 1);
    }

    TEST_CASE("ConstOnlyFunctor is valid")
    {
        zx::threadpool pool(1);

        ConstOnlyFunctor cof;
        pool.push_task(cof);

        const ConstOnlyFunctor cof2;
        pool.push_task(cof2);

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 2);
    }

    TEST_CASE("NormalFunctor is valid")
    {
        zx::threadpool pool(1);

        NormalFunctor nf;
        pool.push_task(nf);

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 1);
    }

    TEST_CASE("NormalFunctorMoveOnlyParam for std::unique_ptr is valid")
    {
        threadpool_function2<> pool(1);

        NormalFunctorMoveOnlyParam<std::unique_ptr<int>> nfmop;
        pool.push_task(nfmop, std::make_unique<int>(5));

        NormalFunctorMoveOnlyParam<std::unique_ptr<int>&&> nfmop3;
        pool.push_task(nfmop3, std::make_unique<int>(5));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 2);
    }
}