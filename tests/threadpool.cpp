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

    ExpensiveType(ExpensiveType&&) = default;
    ExpensiveType& operator=(ExpensiveType&&) = default;
    ~ExpensiveType() = default;

    int copy_count = 0;
};

struct NormalFunctor
{
    NormalFunctor() = default;

    int operator()() const
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
    std::atomic<unsigned int> constructor = 0;
    std::atomic<unsigned int> copy_constructor = 0;
    std::atomic<unsigned int> move_constructor = 0;
    std::atomic<unsigned int> copy_assign = 0;
    std::atomic<unsigned int> move_assign = 0;
    std::atomic<unsigned int> destructor = 0;
    std::atomic<unsigned int> rvalue_call = 0;
    std::atomic<unsigned int> lvalue_call = 0;
    std::atomic<unsigned int> const_lvalue_call = 0;
    std::atomic<unsigned int> const_rvalue_call = 0;
};

struct CounterFunctor
{
    CounterFunctor(const std::string& n, CounterState* state)
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
        : name(rhs.name)
        , state(rhs.state)
    {
        state->copy_constructor++;
        //std::cout << name << "copy construct CounterFunctor\n";
    }

    CounterFunctor(CounterFunctor&& rhs) noexcept
        : name(std::move(rhs.name))
        , state(rhs.state)
    {
        state->move_constructor++;
        //std::cout << name << "move construct CounterFunctor\n";
    }

    CounterFunctor& operator=(const CounterFunctor& rhs)
    {
        if (this == &rhs)
            return *this;

        name = rhs.name;
        state = rhs.state;
        state->copy_assign++;
        //std::cout << name << "copy assign CounterFunctor\n";
        return *this;
    }

    CounterFunctor& operator=(CounterFunctor&& rhs) noexcept
    {
        name = std::move(rhs.name);
        state = rhs.state;
        state->move_assign++;
        //std::cout << name << "move assign CounterFunctor\n";
        return *this;
    }

    template <typename T2>
    void operator()(T2&&) &
    {
        //std::cout << name << "called CounterFunctor&\n";
        state->lvalue_call++;
    }

    template <typename T2>
    void operator()(T2&&) &&
    {
        //std::cout << name << "called CounterFunctor&& \n";
        state->rvalue_call++;
    }

    template <typename T2>
    void operator()(T2&&) const&
    {
        //std::cout << name << "called const CounterFunctor&\n";
        state->const_lvalue_call++;
    }

    template <typename T2>
    void operator()(T2&&) const&&
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

template <typename Threadpool, typename Work, typename... WorkArgs>
void push_job_or_task_and_wait(bool is_task, Threadpool& pool, Work&& work, WorkArgs&&... work_args)
{
    if (is_task)
    {
        pool.push_task(std::forward<Work>(work), std::forward<WorkArgs>(work_args)...);
        pool.wait_all();
        return;
    }

    if constexpr (Threadpool::policy_new_work_v == zx::threadpool_policy_new_work::configurable_and_forbidden_when_stopping)
    {
        // cppcheck-suppress redundantInitialization
        auto optional_future = pool.push_job(std::forward<Work>(work), std::forward<WorkArgs>(work_args)...);
        (*optional_future).wait();
    }
    else
    {
        // cppcheck-suppress redundantInitialization
        auto future = pool.push_job(std::forward<Work>(work), std::forward<WorkArgs>(work_args)...);
        future.wait();
    }
}

TEST_SUITE("Pushing Tasks & Jobs")
{
    TEST_CASE("Ensure functors and arguments don't copy unexpectedly")
    {
        auto test = [](bool is_task, unsigned int expected_moves) {
            CounterState argument;
            CounterState function;

            {
                zx::threadpool pool(1);
                auto logger1 = CounterFunctor("argument", &argument);
                auto logger2 = CounterFunctor("function", &function);
                push_job_or_task_and_wait(is_task, pool, std::move(logger2), std::move(logger1));

                CHECK_EQ(pool.work_executed_total(), 1);

                argument.rvalue_call++;
                CHECK_EQ(argument, function);
                CHECK_EQ(argument.constructor, 1);
                CHECK_EQ(argument.move_constructor, expected_moves); //note: reductions are good, but wanna know when they happen
                CHECK_EQ(argument.copy_constructor, 0);
                CHECK_EQ(argument.copy_assign, 0);
                CHECK_EQ(argument.move_assign, 0);
                CHECK_EQ(argument.lvalue_call, 0);
                CHECK_EQ(argument.const_lvalue_call, 0);
                CHECK_EQ(argument.const_rvalue_call, 0);
            }

            // there's no guarantee that all of the destructors were called after waiting, as the moved-from object could still be in scope
            // therefore we don't check right away, but after the destruction of the threadpool, it should be accurate
            // I don't think blocking until the last moved-from objects destructor finishes is important functionality...?
            // We add 1 as destructor calls should be moves + constructor, which we've already checked is 1
            CHECK_EQ(argument.destructor, expected_moves + 1); //note: reductions are good, but wanna know when they happen
        };

//yeah...
#if defined(_WIN32) && defined(__clang__)
        test(false, 5);
#else
        test(false, 4);
#endif

#if defined(__unix__) || defined(__APPLE__) || defined(__clang__)
        test(true, 6);
#else
        test(true, 5); //todo: why is this less?
#endif
    }

    TEST_CASE("Lambdas with no captures, returns, or parameters")
    {
        auto test = [](bool is_task) {
            zx::threadpool pool(1);
            push_job_or_task_and_wait(is_task, pool, []() mutable {});
            push_job_or_task_and_wait(is_task, pool, []() mutable noexcept {});
            push_job_or_task_and_wait(is_task, pool, []() constexpr {});
            push_job_or_task_and_wait(is_task, pool, []() constexpr noexcept {});
            push_job_or_task_and_wait(is_task, pool, []() {});

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 5);
        };

        test(true);
        test(false);
    }

    TEST_CASE("Lambdas that return with no captures")
    {
        auto test = [](bool is_task) {
            zx::threadpool pool(1);
            push_job_or_task_and_wait(is_task, pool, []() mutable { return 5; });
            push_job_or_task_and_wait(is_task, pool, []() mutable noexcept { return 5; });
            push_job_or_task_and_wait(
                is_task, pool, []() constexpr { return 5; });
            push_job_or_task_and_wait(
                is_task, pool, []() constexpr noexcept { return 5; });
            push_job_or_task_and_wait(is_task, pool, []() { return 5; });

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 5);
        };

        test(true);
        test(false);
    }

    TEST_CASE("Lambdas that return copyable value capture")
    {
        auto test = [](bool is_task) {
            zx::threadpool pool(1);
            int five = 5;
            push_job_or_task_and_wait(is_task, pool, [five]() mutable { CHECK_EQ(five, 5); return five; });
            push_job_or_task_and_wait(is_task, pool, [five]() mutable noexcept { CHECK_EQ(five, 5); return five; });
            push_job_or_task_and_wait(
                is_task, pool, [five]() constexpr { return five; });
            push_job_or_task_and_wait(
                is_task, pool, [five]() constexpr noexcept { return five; });
            push_job_or_task_and_wait(is_task, pool, [five]() { CHECK_EQ(five, 5); return five; });

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 5);
        };

        test(true);
        test(false);
    }

    TEST_CASE("Lambdas that return copyable reference capture")
    {
        auto test = [](bool is_task) {
            zx::threadpool pool(1);
            int five = 5;
            push_job_or_task_and_wait(is_task, pool, [&five]() mutable { CHECK_EQ(five, 5); return five; });
            push_job_or_task_and_wait(is_task, pool, [&five]() mutable noexcept { CHECK_EQ(five, 5); return five; });
            push_job_or_task_and_wait(
                is_task, pool, [&five]() constexpr { return five; });
            push_job_or_task_and_wait(
                is_task, pool, [&five]() constexpr noexcept { return five; });
            push_job_or_task_and_wait(is_task, pool, [&five]() { CHECK_EQ(five, 5); return five; });

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 5);
        };

        test(true);
        test(false);
    }

    TEST_CASE("Lambdas can have unique_ptr arguments")
    {
        threadpool_function2 pool(1);

        pool.push_task([](std::unique_ptr<int> five) mutable { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](std::unique_ptr<int> five) mutable noexcept { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
        pool.push_task([](std::unique_ptr<int> five) { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 3);
    }

    TEST_CASE("Lambdas can have unique_ptr&& arguments")
    {
        //todo: bugs in compilers standard library: https://godbolt.org/z/7EoKqT8eK
        auto test = []() {
            threadpool_function2 pool(1);

            pool.push_task([](std::unique_ptr<int>&& five) mutable { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
            pool.push_task([](std::unique_ptr<int>&& five) mutable noexcept { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
            pool.push_task([](std::unique_ptr<int>&& five) { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 3);
        };

        test();
    }

    TEST_CASE("Lambdas can have auto unique_ptr arguments")
    {
        //todo: bugs in compilers standard library: https://godbolt.org/z/7EoKqT8eK
        auto test = []() {
            threadpool_function2 pool(1);

            pool.push_task([](auto five) mutable { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
            pool.push_task([](auto five) mutable noexcept { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
            pool.push_task([](auto five) { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 3);
        };

        test();
    }

    TEST_CASE("Lambdas can have auto&& unique_ptr arguments")
    {
        //todo: bugs in compilers standard library: https://godbolt.org/z/7EoKqT8eK
        auto test = []() {
            threadpool_function2 pool(1);

            pool.push_task([](auto&& five) mutable { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
            pool.push_task([](auto&& five) mutable noexcept { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));
            pool.push_task([](auto&& five) { CHECK_EQ(*five, 5); return *five; }, std::make_unique<int>(5));

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 3);
        };

        test();
    }

    TEST_CASE("Function2 Lambdas can have unique_ptr captures")
    {
        //todo: bugs in compilers standard library: https://godbolt.org/z/7EoKqT8eK
        auto test = []() {
            threadpool_function2 pool(1);

            auto five = std::make_unique<int>(5);
            pool.push_task([five = std::move(five)]() mutable { CHECK_EQ(*five, 5); return *five; });
            five = std::make_unique<int>(5);
            pool.push_task([five = std::move(five)]() mutable noexcept { CHECK_EQ(*five, 5); return *five; });
            five = std::make_unique<int>(5);
            pool.push_task([five = std::move(five)]() { CHECK_EQ(*five, 5); return *five; });

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 3);
        };

        test();
    }

    TEST_CASE("Function2 Lambdas can move unique_ptr captures")
    {
        //todo: bugs in compilers standard library: https://godbolt.org/z/7EoKqT8eK
        auto test = []() {
            threadpool_function2 pool(1);

            auto five = std::make_unique<int>(5);
            pool.push_task([five = std::move(five)]() mutable { CHECK_EQ(*five, 5); return std::move(five); });
            five = std::make_unique<int>(5);
            pool.push_task([five = std::move(five)]() mutable noexcept { CHECK_EQ(*five, 5); return std::move(five); });

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 2);
        };

        test();
    }

    TEST_CASE("Function2 MoveOnlyFunctor is valid")
    {
        threadpool_function2 pool(1);

        MoveOnlyFunctor mof;
        pool.push_task(std::move(mof));

        pool.wait_all();
        CHECK_EQ(pool.work_executed_total(), 1);
    }

    TEST_CASE("ConstOnlyFunctor is valid")
    {
        auto test = [](bool is_task) {
            zx::threadpool pool(1);

            ConstOnlyFunctor cof;
            push_job_or_task_and_wait(is_task, pool, cof);

            const ConstOnlyFunctor cof2;
            push_job_or_task_and_wait(is_task, pool, cof2);

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 2);
        };

        test(true);
        test(false);
    }

    TEST_CASE("NormalFunctor is valid")
    {
        auto test = [](bool is_task) {
            zx::threadpool pool(1);

            NormalFunctor nf;
            push_job_or_task_and_wait(is_task, pool, nf);

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 1);
        };

        test(true);
        test(false);
    }

    TEST_CASE("NormalFunctorMoveOnlyParam for std::unique_ptr is valid")
    {
        //todo: bugs in compilers standard library: https://godbolt.org/z/7EoKqT8eK
        auto test = []() {
            threadpool_function2 pool(1);

            NormalFunctorMoveOnlyParam<std::unique_ptr<int>> nfmop;
            pool.push_task(nfmop, std::make_unique<int>(5));

            NormalFunctorMoveOnlyParam<std::unique_ptr<int>&&> nfmop3;
            pool.push_task(nfmop3, std::make_unique<int>(5));

            pool.wait_all();
            CHECK_EQ(pool.work_executed_total(), 2);
        };

        test();
    }
}
