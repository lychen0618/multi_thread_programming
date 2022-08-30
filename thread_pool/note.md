#### 最基本的线程池（只支持```function<void()>```）
```c++
class thread_pool {
    std::atomic_bool done;
    threadsafe_queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    // 就是对每个joinable的thread，调用一下join
    join_threads joiner;
    void worker_thread() {
        while (!done) {
            std::function<void()> task;
            if (work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const thread_count = std::thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                threads.push_back(
                    std::thread(&thread_pool::worker_thread, this));
            }
        } catch (...) {
            done = true;
            throw;
        }
    }
    ~thread_pool() { done = true; }

    template <typename FunctionType>
    void submit(FunctionType f) {
        work_queue.push(std::function<void()>(f));
    }
};
```

#### 支持返回任务的```future```的线程池
```c++
class thread_pool {
    // 我理解是packaged_task的模板实参未知，所以需要function_wrapper
    thread_safe_queue<function_wrapper> work_queue;
    void worker_thread() {
        while (!done) {
            function_wrapper task;
            if (work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

public:
    template <typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(
        FunctionType f) {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        work_queue.push(std::move(task));
        return res;
    }
    // rest as before
};
```

#### 避免cache ping-pong的线程池
* cache ping-pong
    如果运行在不同处理器上的多个线程共享一个数据对象，那么当该数据对象被修改时，那么对该数据的修改会在多个处理器的缓存间不断传输（一个处理器做出了修改，那么另一个处理器需要停下来等待该修改在自己的缓存中生效）。

但是这个方法会造成任务分布不均（一个线程很多local的任务，其他线程空转）

```c++
class thread_pool {
    threadsafe_queue<function_wrapper> pool_work_queue;
    typedef std::queue<function_wrapper> local_queue_type;
    static thread_local std::unique_ptr<local_queue_type> local_work_queue;
    void worker_thread() {
        local_work_queue.reset(new local_queue_type);
        while (!done) {
            run_pending_task();
        }
    }

public:
    template <typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(
        FunctionType f) {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(f);
        std::future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_work_queue->push(std::move(task));
        } else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }
    void run_pending_task() {
        function_wrapper task;
        if (local_work_queue && !local_work_queue->empty()) {
            task = std::move(local_work_queue->front());
            local_work_queue->pop();
            task();
        } else if (pool_work_queue.try_pop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }
    // rest as before
};
```

#### work stealing