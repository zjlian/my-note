/// 编译: g++ -std=c++11 sugar_defer.cpp
/// 运行: ./a.out

// #include <functional>
// #include <iostream>

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

/// 辅助二元运算符重载的空类
struct CreateDefer
{
};

/// 利用运算符重载创建 Defer 实例
/// 只要是二元运算符都行，具体哪个无所谓
/// 期望的效果是:
/// auto defer = CreateDefer{} << [&]() { /* do something */ };
Defer operator<<(CreateDefer, std::function<void()> func)
{
    return Defer{func};
}

/// 使用宏生成 " auto defer = CreateDefer{} << [&]() " 部分
#define CAT_SYMBOL(a, b) a b
#define __CREATE_DEFER_NAME(a, b) CAT_SYMBOL(a, b)
#define CREATE_DEFER_NAME __CREATE_DEFER_NAME(__defer_, __COUNTER__)
#define DEFER auto CREATE_DEFER_NAME = CreateDefer{} << [&]()

int main(int, const char **)
{
    std::cout << "hello" << std::endl;
    auto defer = CreateDefer{} << [&]()
    {
        std::cout << "defer1" << std::endl;
    };

    DEFER
    {
        std::cout << "defer2" << std::endl;
    };

    DEFER
    {
        std::cout << "defer3" << std::endl;
    };

    std::cout << "hello" << std::endl;

    __CREATE_DEFER_NAME(__defer_, __COUNTER__)
    CAT_SYMBOL(__defer_, __COUNTER__)

}

/**
 * 程序运行结果：
 * hello
 * hello
 * defer3
 * defer2
 * defer1
 */
