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

可以观察下面的为代码：
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

## 什么是 RAII（Resource Acquisition Is Initialization）
RAII 是拥有析构函数的编程语言特有的一种惯用法，在 c++ 标准库中大量应用了这一方法。   
最著名的必须是 std::unique_ptr 等智能指针了，智能指针构造时分配内存，析构时回收内存，避免直接去用对开发人员有一定脑力负担的 new/delete 分配回收内存。   
类似的还有，std::fstream 构造时打开文件，析构时关闭文件。std::lock_guard 构造时上锁，析构时解锁。

从上面的几个例子应该能看出 RAII 这一惯用法的核心思想，用一个类去管理系统资源，当这个类初始化时向系统申请特定的资源，在析构时将资源归还系统。   
资源管理类的实例必须是分配在栈空间上，生命周期明确，能够自动回收并调用析构函数。   
RAII 是 c++ 中最简单的资源管理方法，更多的细节和定义可以自行查阅文章末尾参考部分链接 **[1]** 的文档。   

RAII 配合上 c++11 引入的左值引用、右值引用和移动语义等机制，可以派生出抽象层级更高的所有权机制，用于更灵活更复杂的资源管理。

## 实现一个最简单的 defer



## 参考
- [1] [cppreference RAII](https://zh.cppreference.com/w/cpp/language/raii)
- [2] [cppreference std::experimental::scope_exit](https://zh.cppreference.com/w/cpp/experimental/scope_exit)
- [3] [Hands-On Design Patterns With C++（十一）ScopeGuard（上）](https://zhuanlan.zhihu.com/p/148234561)
- [4] [深入聊一聊C/C++中宏展开过程](https://zhuanlan.zhihu.com/p/125062325)