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

template <typename T>
struct LoggerFunctor
{
    LoggerFunctor(std::string n)
        : name(n + " ")
    {
        std::cout << name << "construct LoggerFunctor\n";
    }

    ~LoggerFunctor()
    {
        std::cout << name << "destroy LoggerFunctor\n";
    }

    LoggerFunctor(const LoggerFunctor& rhs)
    {
        name = rhs.name;
        std::cout << name << "copy construct LoggerFunctor\n";
    }

    LoggerFunctor(LoggerFunctor&& rhs) noexcept
    {
        name = rhs.name;
        std::cout << name << "move construct LoggerFunctor\n";
    }

    LoggerFunctor& operator=(const LoggerFunctor& rhs)
    {
        name = rhs.name;
        std::cout << name << "copy assign LoggerFunctor\n";
        return *this;
    }

    LoggerFunctor& operator==(LoggerFunctor&& rhs) noexcept
    {
        name = rhs.name;
        std::cout << name << "move assign LoggerFunctor\n";
        return *this;
    }

    template <typename T2>
    void operator()(T2&& t2) &
    {
        std::cout << name << "called LoggerFunctor&\n";
    }

    template <typename T2>
    void operator()(T2&& t2) &&
    {
        std::cout << name << "called LoggerFunctor&& \n";
    }

    template <typename T2>
    void operator()(T2&& t2) const&
    {
        std::cout << name << "called const LoggerFunctor&\n";
    }

    template <typename T2>
    void operator()(T2&& t2) const&&
    {
        std::cout << name << "called const LoggerFunctor&\n";
    }

    T t;
    std::string name;
};

template <
    zx::threadpool_policy_pending_work A = zx::threadpool_policy_pending_work::wait_for_work_to_finish,
    zx::threadpool_policy_new_work B = zx::threadpool_policy_new_work::configurable_and_forbidden_when_stopping,
    typename C = void>
using threadpool_function2 = zx::threadpool<A, B, void, fu2::unique_function<void()>>;

TEST_SUITE("Tasks")
{
    /*TEST_CASE("LoggerFunctor")
    {
        ThreadpoolConsoleTracing<> pool(1);
        auto logger1 = LoggerFunctor<std::shared_ptr<int>>("a");
        logger1.t = std::make_shared<int>(1);
        auto logger2 = LoggerFunctor<std::shared_ptr<int>>("b");
        logger2.t = std::make_shared<int>(5);
        pool.push_task(std::move(logger2), std::move(logger1));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 1);
    }*/

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