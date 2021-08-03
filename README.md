# Threadpool

[![Threadpool](https://github.com/Zephilinox/Threadpool/actions/workflows/threadpool.yml/badge.svg)](https://github.com/Zephilinox/Threadpool/actions/workflows/threadpool.yml)

A configurable but slow threadpool, backed by `std::queue` and `std::mutex`

C++17 is required

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

The default number of threads is `std::max(std::min(std::thread::hardware_concurrency(), 1U) - 1U, 1U)` which will be between 1 and 1 less than the max number of logical cores on your system.

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
```

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

Changing the policy to `zx::threadpool_policy_pending_work::leave_work_unfinished` will cause any pending work to be ignored when stopping, therefore work that was pushed will never execute.

Note that when `leave_work_unfinished` is used the `std::future` returned from `push_job` may throw with a [broken_promise exception](https://en.cppreference.com/w/cpp/thread/future_errc), as the job isn't guaranteed to execute.

```cpp
std::optional<std::future<void>> maybe_future;

{
    zx::threadpool<zx::threadpool_policy_pending_work::leave_work_unfinished> pool(1);
    maybe_future = pool.push_job([](){ /* do nothing */ });
}

if (maybe_future)
    maybe_future->wait(); //may throw a broken_promise exception
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

The messages can be customised by providing your own tracing class instead of providing the tracing class `zx::threadpool_tracing_logger` with a logger class. You also aren't limited to logging, the internals of the threadpool could be be modified or inspected.