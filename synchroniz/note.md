```std::this_thread::sleep_for(std::chrono::milliseconds(100));```
### 条件变量
* ```condition_variable```
    * ```wait```
    * ```notify_one```
    * ```notify_all```

* ```condition_variable_any```：可以用于非```mutex```的其他锁

### future
* ```std::future<>```(只可移动的) ```std::shared_future<>```(可拷贝的)
    * 代表一个未来的事件是否发生，以及事件相关的信息，std::future<void>代表事件没有相关数据
    * future不是线程安全的，但是多个线程可以有自己的```std::shared_future<>```，尽管指向相同的数据
    * 可以用于得到一个后台线程的返回值

获取```future```的多种方式：
1. ```async```
    * 启动一个后台线程，返回一个```future```
    * 可以设置属性：
        1. ```std::launch::deferred```：推迟，直到调用wait或get
        2. ```std::launch::async```：立即启动线程运行

2. ```packaged_task<>```
    ```c++
    std::packaged_task<> ties a future to a function or callable object. When the std:: packaged_task<> object is invoked, it calls the associated function or callable object and makes the future ready, with the return value stored as the associated data. 
    ```

3. ```promise```
    ```c++
    std::promise<T> provides a means of setting a value (of type T) that can later be read through an associated std::future<T> object.
    ```

    * ```set_value```
    * ```set_exception```

### 限制阻塞时间
* 时钟（头文件\<chrono\>）

* 两种方式
    * 限制运行时间
    * 限制运行到某个时间点

* 主动休眠
    1. ```std::this_thread::sleep_until()```
    2. ```std::this_thread::sleep_for()```

* 超时（4.3.4）

### 其他
* 等待```future```(实验特性)
    1. 等待不止一个```future```
    2. 等待集合中第一个就绪的```future```

* latch和barrier
* 消息传递