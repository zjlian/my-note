/// 编译: g++ -std=c++11 simple_defer.cpp
/// 运行: ./a.out

#include <functional>
#include <iostream>

class Defer
{
public:
    /// Defer 构造时接收一个任意可调用类型的实例，存到类型为 std::function 的 func_ 里
    Defer(std::function<void()> func)
        : func_(func) {}

    /// 析构时调用 func_
    ~Defer()
    {
        func_();
    }

private:
    std::function<void()> func_;
};

int main(int, const char **)
{
    std::cout << "hello" << std::endl;

    // 声明一个 lambda，传递给 Defer 的构造函数
    auto defer_operator = []()
    {
        std::cout << "defer" << std::endl;
    };
    Defer defer{defer_operator};

    std::cout << "hello" << std::endl;
}

/**
 * 程序运行结果：
 * hello
 * hello
 * defer
 */
