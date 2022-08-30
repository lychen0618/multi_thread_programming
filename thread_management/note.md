### c++标准库
#### 线程创建
```c++
// 默认初始化，说明该线程对象没有绑定一个执行过程，和unique_ptr默认初始化类似
std::thread mythread0;

void func(){
    // do something
}
std::thread my_thread1(func);

class Task{
public:
    void operator() () const{
        // do something
    }
};

Task task;
// 可调用函数对象会被拷贝进线程的空间
std::thread my_thread2(task);
// 使用临时对象初始化线程时需要用{}，不然编译器会把它解释成函数声明
std::thread my_thread3{Task()};
// lambda对象可以使用()
// thread对象在被销毁前一定要调用了join()或detach()，不然就会成为僵尸进程

// 参数传递
// the arguments are copied into internal storage, where they can be accessed by the newly created thread of execution, and then passed to the callable object or function as rvalues as if they were temporaries.
void f(int i,std::string const& s);
void oops(int some_param)
{
    char buffer[1024];
    sprintf(buffer, "%i",some_param);
    // 这里传递进去了一个指针，但是oops函数退出时，这个指针所指的空间会失效
    // 调用f的时候会用这个const char*指针所指向的字符数组的内容创建一个临时的string对象，然后f的实参引用这个临时对象
    std::thread t(f,3,buffer);
    // 最好这样使用
    // std::thread t(f,3,std::string(buffer));
    // 如果要传递引用参数，那么需要用std::ref()
    t.detach();
}
// 使用成员函数创建线程
class X
{
public:
    void do_lengthy_work();
};
X my_x;
std::thread t(&X::do_lengthy_work,&my_x);
// 使用move触发移动语义。当使用变量（即使是个右值引用变量）时，变量是一个左值；而临时对象是右值，所以不用用move转换
// thread和unique_ptr都只能移动，不能拷贝，因为它们的拷贝构造函数和拷贝赋值运算符都是删除的函数
void process_big_object(std::unique_ptr<big_object>);
std::unique_ptr<big_object> p(new big_object);
p->prepare_data(42);
std::thread t(process_big_object,std::move(p));
/* By specifying std::move(p) in the std::thread constructor, the ownership of big_ object is transferred first into internal storage for the newly created thread and then into process_big_object. */
```
#### 等待线程结束
```c++
my_thread1.join();
// 一个线程只能被join一次，可以用joinable()来判断是否可以join
// join放置的位置要注意
// 1. 放置在catch中
// 2. 使用RAII创建一个thread_guard
```

#### 分离线程
```detach()```

#### 线程标识符
```c++
// 类型 std::thread::id
// 可以被拷贝和比较，也可以作为关键字，因为标准库提供了hash<std::thread::id>
std::thread t(func);
// 如果t没有绑定一个执行过程，那么会返回一个默认构造的id对象
t.get_id();

// 另一种方式
std::this_thread::get_id()
```

### pthread库