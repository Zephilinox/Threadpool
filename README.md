# Threadpool

[![Threadpool](https://github.com/Zephilinox/Threadpool/actions/workflows/threadpool.yml/badge.svg)](https://github.com/Zephilinox/Threadpool/actions/workflows/threadpool.yml) [![codecov](https://codecov.io/gh/Zephilinox/Threadpool/branch/main/graph/badge.svg?token=n9P9btRBNe)](https://codecov.io/gh/Zephilinox/Threadpool)

A configurable but slow header-only threadpool, backed by `std::queue` and `std::mutex`

C++23 is required

# Basic Usage

```cpp
#include <threadpool/threadpool.hpp>
#include <iostream>

int main()
{
    zx::threadpool pool(2);

    pool.push_task([](){
        std::cout << "Hello, ";
    });

    pool.push_task([](){
        std::cout << "World!\n";
    });
}
```

Here we specified 2 threads but otherwise the default number of threads will be between 1 and 1 less than the max number of logical cores on your system.

# Installing

Simply copy `include/threadpool/threadpool.hpp` to your codebase. You can optionally use the provided Tracers in `include/threadpool/tracers/`, but they are not required and not used by default.

# Work

### Jobs

A Job is a unit of work that can be tracked via `std::future`.
You can block waiting for the future to become ready, regardless of if the job returns a value or void.

```cpp
zx::threadpool pool(1);
auto maybe_future = pool.push_job([](){ /* do nothing */ });
if (maybe_future)
    maybe_future->wait(); //block waiting for job to complete
```

### Tasks

A Task is a unit of work that can't be tracked.
By design, there's no way to wait for that specific task to complete.

You could instead wait for all work to complete. Note that if other threads are adding work while you wait, or there is previous work being completed, you will need to wait for all of that work to also complete.

```cpp
zx::threadpool pool(1);
pool.push_task([](){ /* do nothing */ });
pool.wait_all();
// or wait for every piece of work to begin executing
pool.wait_all_pending();
```

### Processing

There are multiple options for processing queued up work, if you don't want to leave it under the threadpools control.

```cpp
zx::threadpool pool(0);
pool.push_task([](){ /* do nothing*/ });
// help everything along, without waiting for every piece of work to finish
pool.process_all_pending();
// or just once
pool.process_once();
// or cooperatively, forever
while (true)
{
    pool.process_once();
    std::this_thread::yield();
}
```

In these cases the processing occurs "outside of" the Threadpool, which has the following effects:

1. The thread_index is equal to the number of threads, i.e `pool.thread_count()`
2. The work is considered to be handled "by others", i.e `pool.work_executed_by_others()`

Note that this currently isn't as optimal as allowing the Threadpool to handle the processing automatically, as it has to construct and destruct a worker each time

### Statistics

While the Threadpool is running it will gather some statistics about the work being done.

```cpp
zx::threadpool pool;
for (unsigned int thread_index : poolthread_count())
{
    pool.work_executed(thread_index);
}

pool.work_executed_by_others();
pool.work_executed_total();
pool.work_pending();
pool.work_executing();
pool.work_total();
```

### Other

There are a few ways to control or inspect the Threadpool at runtime:

```cpp
zx::threadpool pool;
pool.is_allowing_new_work();
pool.is_stopping_or_stopped();
pool.is_stopped();

pool.allow_new_work(false);
// returns false, never executed
pool.push_task([](){});

pool.allow_new_work(true);
// returns true, scheduled for execution
pool.push_task([](){});
```

At compile time, we have policies.

# Policies

### New Work

The `zx::threadpool_policy_new_work` policy determines whether pushing new work to the threadpool can fail. By default this is `configurable_and_forbidden_when_stopping` which allows users to toggle when the threadpool is accepting new work, as well as forbid new work from being pushed while the threadpool is stopping.

Note that this default policy requires `push_job` to return an optional future, and for `push_task` to return a boolean.

Changing the policy to `zx::threadpool_policy_new_work::always_allow` will cause `push_job` to return an ordinary `std::future`, and `push_task` to return nothing.

```cpp
zx::threadpool<zx::threadpool_policy_pending_work::wait_for_work_to_finish, zx::threadpool_policy_new_work::always_allow> pool(1);
auto future = pool.push_job([](){ /* do nothing */ });
future.wait(); //block waiting for job to complete
```

### Pending Work

The `zx::threadpool_policy_pending_work` policy determines whether work in the queue when the threadpool is stopping should be ignored, or completed. By default this is `wait_for_work_to_finish` which will cause the destructor to block until all jobs have completed.

Note that if the `New Work` policy is used to `always_allow` that work can be added while the destructor blocks, which could cause the destructor to never complete.

Changing the policy to `zx::threadpool_policy_pending_work::leave_work_unfinished` will cause any pending work to be ignored when stopping, therefore work that was pushed will never execute unless it is already executing.

Note that when `leave_work_unfinished` is used the `std::future` returned from `push_job` may throw with a [broken_promise exception](https://en.cppreference.com/w/cpp/thread/future_errc) when the job is not executed. This is to prevent waiting forever on the returned future.

```cpp
std::optional<std::future<void>> maybe_future;

{
    zx::threadpool<zx::threadpool_policy_pending_work::leave_work_unfinished> pool(1);
    maybe_future = pool.push_job([](){ /* do nothing */ });
}

 //will throw a broken_promise exception if the work didn't execute before the pool was destroyed
if (maybe_future)
    maybe_future->wait();

```

# Tracing

The threadpool supports specifying a custom tracing class that is a `friend` of the threadpool, which will contain static methods that will be called during its operation.

A default tracing class is provided which formats messages and calls a user-defined logging class. By default a `zx::Threadpool` object has tracing disabled, and will not affect performance.

Note that the tracing functions will be called from different threads, and therefore thread safety must be maintained.

```cpp
class MyConsoleLogger
{
public:
    enum class LogLevel
    {
        none = 0,
        critical = 1,
        error = 2,
        info = 3,
        debug = 4,
    };

    static std::string log_level_to_string(LogLevel level)
    {
        switch (level)
        {
            case LogLevel::none: return "[NONE]";
            case LogLevel::critical: return "[CRITICAL]";
            case LogLevel::error: return "[ERROR]";
            case LogLevel::info: return "[INFO]";
            case LogLevel::debug: return "[DEBUG]";
            default: return "[UNKNOWN]";
        }
    }

    static void log(LogLevel level, const std::string& str)
    {
        static std::mutex mutex;
        std::scoped_lock lock(mutex);
        std::cout << log_level_to_string(level) << " " << str << "\n";
    }
};

template <
    zx::threadpool_policy_pending_work A = zx::threadpool_policy_pending_work::wait_for_work_to_finish,
    zx::threadpool_policy_new_work B = zx::threadpool_policy_new_work::configurable_and_forbidden_when_stopping>
using ThreadpoolConsoleTracing = zx::threadpool<A, B, zx::threadpool_tracing_logger<MyConsoleLogger>>;


int main()
{
    ThreadpoolConsoleTracing pool(1);
}
```

will output

```cpp
[INFO] threadpool: construction started. spawning 1 worker threads
[INFO] threadpool: construction finished. worker threads spawned
[INFO] threadpool: started destruction. is_stopping = true
[INFO] threadpool: has stopped
[INFO] threadpool: joining worker thread 1/1 after executing 0 units of work
[INFO] threadpool: 0 units of work were executed by others
[INFO] threadpool: finished destruction
```

The messages can be customised by providing your own tracing class instead of providing the tracing class `zx::threadpool_tracing_logger` with a logger class. You also aren't limited to logging, the internals of the threadpool could be modified or inspected.

# Advanced

The Threadpool is actually defined as:

```cpp
template <
    threadpool_policy_pending_work pending_work_policy = threadpool_policy_pending_work::wait_for_work_to_finish,
    threadpool_policy_new_work new_work_policy = threadpool_policy_new_work::configurable_and_forbidden_when_stopping,
    typename Tracer = void,
    typename Function = std::function<void()>>
class threadpool final
{
    // ...
}
```

As such you also have control over the type-erased backend used to store tasks and jobs within the Threadpool.
One such alternative implementation is https://github.com/Naios/function2. Here is how that could be defined:

```cpp
template <
    zx::threadpool_policy_pending_work A = zx::threadpool_policy_pending_work::wait_for_work_to_finish,
    zx::threadpool_policy_new_work B = zx::threadpool_policy_new_work::configurable_and_forbidden_when_stopping,
    typename C = void>
using threadpool_fu2 = zx::threadpool<A, B, C, fu2::unique_function<void()>>;
```

Which you can use like normal, but with different performance characteristics and only supporting move-only types

```cpp
threadpool_fu2 pool;
pool.push_task([](){});
```
