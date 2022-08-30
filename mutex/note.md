### 锁的基本使用
* ```lock_guard```
```c++
// 创建
std::mutex l;
// 加锁  尝试加锁 解锁
l.lock();
l.try_lock();
l.unlock();

// 推荐下面的使用方法
std::lock_guard<std::mutex> guard(l);

// c++17 引入了一种新的lock guard，std::scoped_lock

/*
Don’t pass pointers and references to protected data outside the scope of the lock, whether by returning them from a function, storing them in externally visible memory, or passing them as arguments to user-supplied functions.
*/

// 如果要在const成员函数中使用mutex，那么定义的时候要加上mutable
```

* ```unique_lock```
```c++
// 比lock_guard消耗大，但是提供更好的灵活性
// 全局锁，或者一个类对象的内部锁
std::mutex m1, m2;

// 局部需要加锁时
// unique_lock记录了锁的状态。创建时，如果加锁了，那么不会再加锁；析构时，如果没有加锁，那么不会unlock
// 传入defer_lock标志，那么表示创建时不调用锁的lock
std::unique_lock<std::mutex> lock_a(m1, std::defer_lock);
std::unique_lock<std::mutex> lock_b(m2, std::defer_lock);
std::lock(lock_a, lock_b); // 这个函数避免死锁

// unique_lock的一个使用场景：需要传递锁的所有权；控制加锁的粒度，当不需要时就调用unlock()
// you can call unlock() when the code no longer needs access to the shared data and then call lock() again if access is required later in the code
```

* ```shared_lock``` (c++14引入)
    * 和```unique_lock```的不同是，支持多个```shared_lock```对象共享一个锁

### 避免死锁
* 锁的使用不当造成死锁
```c++
// 全局锁，或者一个类对象的内部锁
std::mutex m1, m2;

// 局部需要加锁时
std::lock(m1, m2); // 这个函数避免死锁
// adopt_lock 让 guard 不去调用 lock()
std::lock_guard<std::lock> lg1(m1, std::adopt_lock);
std::lock_guard<std::lock> lg1(m1, std::adopt_lock);

// lock函数可以在需要给所有锁加锁时避免死锁，但是分别加锁就不行了。需要程序自己定义并控制好加锁的顺序
```

* 两个线程分别join对方

### 其他
* 只有初始化的时候需要加锁（单例模式的类）
```c++
std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag;
void init_resource()
{
    resource_ptr.reset(new some_resource);
}
void foo() {
    // call_once的第二个参数可以是任意可调用对象（成员函数也可以，但是后面要加上this）
    std::call_once(resource_flag,init_resource);
    resource_ptr->do_something();
}

// 静态局部变量会在程序第一次执行到它时初始化，编译器保证了这个初始化只会执行一次
```

* 保护很少被改变的共享数据
```c++
// 使用读写锁来保护（c++14和c++17才提供）
// 使用的锁的类型是 std::shared_lock
// 需要修改数据的程序使用lock_guard或unique_lock；不需要的程序使用shared_lock

```

* 递归互斥锁
```c++
// 在一个线程中，支持连续多次调用lock()
// 调用几次lock(), 就要调用几次unlock()。不过可以使用lock_guard（unique_lock）负责
// 使用场景：一个类的每个成员函数都会加锁，但是一个成员函数需要调用另一个成员函数
std::recursive_mutex rm;
```