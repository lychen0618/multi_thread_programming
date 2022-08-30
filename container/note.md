设计线程安全的容器的准则：
* thread safe
* enabling genuine concurrent access

### 基于锁的线程安全的容器
#### 栈
```c++
#include <exception>
struct empty_stack : std::exception {
    const char* what() const throw();
};
template <typename T>
class threadsafe_stack {
private:
    std::stack<T> data;
    mutable std::mutex m;

public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;
    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty())
            throw empty_stack();
        std::shared_ptr<T> const res(
            std::make_shared<T>(std::move(data.top())));
        data.pop();
        return res;
    }
    void pop(T& value) {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty())
            throw empty_stack();
        value = std::move(data.top());
        data.pop();
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};
// exception-safe
// 1. 加锁可能会抛出异常（申请资源失败），但是这很少出现，即使出现了也不会有问题，因为加锁是接口的第一行，不会对容器做实际的操作（释放锁不会异常）
// 2. push也可能由于申请内存而出异常，但是正确性会由stack保证
// 3. 第一个pop中创建res可能抛异常，但是也没问题，因为没有修改容器数据
// 4. 第二个pop构建value时可能会由于拷贝（移动）构造运算符抛异常，但是也没问题，因为没有修改容器数据

// 死锁：会调用元素的构造函数等外部代码

// 构造函数和析构函数不安全，需要用户保证容器创建好后才会有多线程访问，容器析构时其他线程不再访问容器

// 上面的这个设计不好
// 1. 锁的粒度大
// 2. 用户程序需要处理抛出的empty_stack异常并忙等待
```

#### 队列
基本版本
```c++
template <typename T>
class threadsafe_queue {
private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T>> data_queue;
    std::condition_variable data_cond;

public:
    threadsafe_queue() {}
    void push(T new_value) {
        std::shared_ptr<T> data(
            std::make_shared<T>(std::move(new_value)));
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        value = std::move(*data_queue.front());
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }
    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
// 队列的元素用shared_ptr的原因：如果要返回一个shared_ptr<T>，避免调用make_shared申请内存时抛异常。如果抛了异常，那么被唤醒的线程没有正确取出元素，其他睡眠的线程也得不到通知，因为notify_one已经调用过了。
```

升级版本

### 无锁的线程安全的容器