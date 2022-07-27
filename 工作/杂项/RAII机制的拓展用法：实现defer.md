# RAII 机制的拓展用法：实现 defer

## defer 的作用和效果
defer 是一种程序流程控制机制，跟 if..else while switch..case 等关键字属于同一类别的东西。

defer 的作用是在程序执行即将离开一个作用域之前，执行一段自定义的逻辑。

观察下面的 go 语言代码：
``` golang
func SayHallo() {
    fmt.Println("hallo")
    defer func() {
        fmt.Println("defer")
    }
    fmt.Println("hallo")
}
```
<details>
<summary>程序运行结果</summary>
<pre>
hallo
hallo
defer   
</pre>
</details>

---

在 c++ 中同样有办法实现相同的流程控制机制，使用代码如下:
```c++
void SayHallo()
{
    std::cout << "hello" << std::endl;
    DEFER
    {
        std::cout << "defer" << std::endl;
    };
    std::cout << "hello" << std::endl;
}
```
本文的主要内容就是讲解如何实现这个功能。

## 为什么需要 defer
defer 的主要作用之一是简化资源释放回收的代码，降低编写代码的脑力成本。

下面是一段文件操作的伪代码：
```c++
Result ReadFile()
{
    int32_t fd = open("xxx");
    ...
    if (...)
    {
        ...
        close(fd);
        return xxx;
    }
    ...
    if (...)
    {
        ...
        close(fd);
        return xxx;
    }
    ...
    if (...)
    {
        ...
        close(fd);
        return xxx;
    }
    ...
}
```
函数 ReadFile() 会打开一个文件并读取解析文件的内容，函数内存在多处 return 返回读取的结果，每个 return 前都需要手动关闭文件，避免资源泄漏。这类多处 return 的代码就是制造的 bug 温床。   
代码量太大，逻辑太绕，都有可能让维护代码的人大脑宕机，遗漏了必要的 close() 调用，导致资源泄漏，程序朝着非预期的方向运行。

如果有 defer，上面的代码就能改成下面的样子：
```c++
Result ReadFile()
{
    int32_t fd = open("xxx");
    // 使用 defer，在离开作用域前调用 close()
    DEFER
    {
        close(fd);
    };
    ...
    if (...)
    {
        ...
        return xxx;
    }
    ...
}
```
整个函数只需要写一次 close()，无论有多少个 return，都能保证离开作用域前自动调用 close()。

## 什么是 RAII（Resource Acquisition Is Initialization）
RAII 是拥有析构函数的编程语言特有的一种惯用法，在 c++ 标准库中大量应用了这一方法。   
最著名的必须是 std::unique_ptr 等智能指针了，智能指针构造时分配内存，析构时回收内存，避免直接去用对开发人员有一定脑力负担的 new/delete 分配回收内存。   
类似的还有，std::fstream 构造时打开文件，析构时关闭文件。std::lock_guard 构造时上锁，析构时解锁。

从上面的几个例子应该能看出 RAII 这一惯用法的核心思想，用一个类去管理系统资源，当这个类初始化时向系统申请特定的资源，在析构时将资源归还系统。   
资源管理类本身是有一定要求的，实例必须是分配在栈空间上，生命周期明确，能够自动回收并调用析构函数。   
RAII 是 c++ 中最简单的资源管理方法，更多的细节和定义可以自行查阅文章末尾参考部分链接 **[1]** 的文档。   

RAII 配合上 c++11 引入的左值引用、右值引用和移动语义等机制，可以派生出抽象层级更高的所有权机制，用于更灵活更复杂的资源管理。

## 实现一个最简单的 defer
一个最简单的 Defer class 的实现，就是在构造时初始化一个 std::function 实例，在析构时执行。   
这也是为什么说 defer 是 RAII 的拓展用法的原因，因为 defer 并不是严格符合 RAII 语义的，RAII 的定义是初始化时获取资源，析构时释放资源。   

代码如下：
```c++ 
class Defer
{
public:
    /// Defer 构造时接收一个任意可调用类型的实例，
    /// 存到类型为 std::function 的 func_ 里
    Defer(std::function<void()> func)
        : func_(func) {}

    /// 析构时调用 func_
    ~Defer()
    {
        func_();
    }

private:
    /// 保存 Defer 析构时需要执行的代码
    std::function<void()> func_;
};
```
从上面的代码可以看出，Defer class 的实现是非常简单的，仅仅是保存一个函数到 std::function 里，当 Defer 实例析构时调用这个函数。   

使用代码如下：
```c++
int main(int, const char **)
{
    std::cout << "hello" << std::endl;

    // 创建一个 lambda，传递给 Defer 的构造函数
    auto defer_operator = []()
    {
        std::cout << "defer" << std::endl;
    };
    Defer defer{defer_operator};

    std::cout << "hello" << std::endl;
}
```
第一版的 Defer class 虽然实现很简单，但用起来并不是那么方便，如果同一作用域内需要多个 defer，还要手动给他们设置不同的变量名。    
下一章节将会讲解如何利用运算符重载和宏实现语法糖，简化 defer 的使用。

## 使用运算符重载和宏实现语法糖
先来看看期望的用运算符重载创建一个 Defer 实例是什么样的：
```c++
auto defer = CreateDefer{} << [&]()
{
    std::cout << "defer" << std::endl;
};
```
上面的代码中重载了二元运算符 operator<<，左侧是一个 CreateDefer 实例，右侧是一个 lambda 表达式，运算符会返回一个 Defer 实例。

要实现这样的代码，只需要再提供两个东西，一个是没有任何内容的空类 CreateDefer class，另一个是重载 operator<< 的实现。
```c++
/// 空类，用于辅助运算符重载
class CreateDefer{};

Defer operator<<(CreateDefer, std::function<void()> func)
{
    return Defer{func};
}
```
重载的 operator<< 的第一个参数不需要形参名，接收一个 CreateDefer 实例只是为了能够调用到这个运算符重载的实现。   
重载的实现简单构造返回一个 Defer 实例就行了。   

有了上面几行代码，下一步需要做的就是用宏生成 `auto defer = CreateDefer{} << [&]()` 部分的代码。   
这一部分代码不能用宏简单替换，如果在同一作用域内写了超过两个 `DEFER { ... };`，会出现重复定义同名变量的编译错误，所以必须要能够生成唯一的变量名。   
``` c++
...
#define DEFER auto defer = CreateDefer{} << [&]()
...
DEFER {};
// 第二次使用时会报重复定义 defer 这个名称
DEFER {};
```
这里用的是 `__COUNTER__` 来保证唯一性，这个宏每被展开一次，其内容都会自增。   
多次使用 DEFER 生成的变量名应该是类似 "__defer_0" "__defer_1" 这这样的。   
全部的宏定义的写法如下：
```c++
// 拼接符号名
#define CAT_SYMBOL(a, b) a##b
// 辅助展开宏变量 __COUNTER__
#define __CREATE_DEFER_NAME(a, b) CAT_SYMBOL(a, b)
// 拼接 __defer_ 和 __COUNTER__ 的内容，组成新的变量名
// 生成的变量名会是 __defer_0 __defer_1 __defer_2 ... 
#define CREATE_DEFER_NAME __CREATE_DEFER_NAME(__defer_, __COUNTER__)
// 最后再用 CREATE_DEFER_NAME 生成变量名
#define DEFER auto CREATE_DEFER_NAME = CreateDefer{} << [&]()
```
有了上面四行宏定义后，就能做到文章开头的语法效果了。
```c++
void SayHallo()
{
    std::cout << "hello" << std::endl;
    DEFER
    {
        std::cout << "defer" << std::endl;
    };
    std::cout << "hello" << std::endl;
}
```
上面四行宏定义中，拼接变量名的过程多了一个 `__CREATE_DEFER_NAME` 作为中间层，为什么需要这样的中间层，这就涉及到了 c 语言宏展开的结束条件。   
宏展开的结束条件有 4 个：   
（一）最普通的，已经没有任何宏符号了，不需要再继续展开   
（二）下一步是转换字符串   
（三）下一步是符号拼接   
（四）出现直接或间接的递归   

需要 `__CREATE_DEFER_NAME` 就是因为出现了上述的情况（三），宏 `CAT_SYMBOL` 用于符号拼接，如果没有 `__CREATE_DEFER_NAME`，宏展开过程将会是这样的：
```c++
// 宏定义
#define CAT_SYMBOL(a, b) a##b
#define CREATE_DEFER_NAME CAT_SYMBOL(__defer_, __COUNTER__)

// CREATE_DEFER_NAME 的展开流程
CREATE_DEFER_NAME

        |
        v

CAT_SYMBOL(__defer_, __COUNTER__)

        |
        v

__defer_##__COUNTER__

        |
        v

__defer___COUNTER__
```
由于在宏 `CAT_SYMBOL` 展开后出现了符号拼接操作，展开结束，最后完成符号拼接操作将 `__defer_` 和 `__COUNTER__` 组合在一起，整个预处理就算完成了。   
最终展开的结果是 "\_\_defer\_\_\_COUNTER\_\_" 肯定不是我们想要的，这样跟直接写死 "defer" 没有任何区别。   

所以才需要多一个在中间多插入一个宏，帮助 `__COUNTER__` 完成展开，在去进行符号拼接。过程如下：
```c++
// 宏定义
#define CAT_SYMBOL(a, b) a##b
#define __CREATE_DEFER_NAME(a, b) CAT_SYMBOL(a, b)
#define CREATE_DEFER_NAME __CREATE_DEFER_NAME(__defer_, __COUNTER__)

// CREATE_DEFER_NAME 的展开流程
CREATE_DEFER_NAME

        |
        v

__CREATE_DEFER_NAME(__defer_, __COUNTER__)

        |
        v

__CREATE_DEFER_NAME(__defer_, 0)

        |
        v

CAT_SYMBOL(__defer_, 0)

        |
        v

__defer_##0

        |
        v

__defer_0
```
有了 `__CREATE_DEFER_NAME` 后，才能避免 `__COUNTER__` 直接进入符号拼接，从而有机会被展开成为实际的值。   
在第一次用 DEFER 的使用会生成符号 "__defer_0"，第二次时生成 "__defer_1"，以此类推。

## 使用模板消除非必要的运行时开销
空口无凭，下面直接通过编译器编译输出的汇编指令，看看优化到底有没有效果。   
编译参数都是 `-std=c++17 -O3`   
优化前：https://godbolt.org/z/Tf4n44qe3   
优化后：https://godbolt.org/z/3xT9jEznn   

优化前后最明显的差别就是汇编指令的数量了，优化前有 150+ 行的汇编指令，而优化后只有 40+ 行。    
当然了程序性能是不能用代码行数来评判的，更多的是需要实际分析和做基准测试做对比。   
优化前的汇编指令里能看到大量的寻址操作和对地址进行调用操作，而优化后的汇编没有一条指令是多余的，干净利落直截了当。    

那么，优化前的这么多汇编指令都是干什么的？为什么会出现这么多寻址和调用？









## 参考
- [1] [cppreference RAII](https://zh.cppreference.com/w/cpp/language/raii)
- [2] [cppreference std::experimental::scope_exit](https://zh.cppreference.com/w/cpp/experimental/scope_exit)
- [3] [Hands-On Design Patterns With C++（十一）ScopeGuard（上）](https://zhuanlan.zhihu.com/p/148234561)
- [4] [深入聊一聊C/C++中宏展开过程](https://zhuanlan.zhihu.com/p/125062325)