/// 编译: g++ -std=c++11 sugar_defer.cpp
/// 运行: ./a.out

#include <iostream>
#include <type_traits>
#include <utility>
#include <functional>

/// 拼接符号
#define CAT_SYMBOL(a, b) a##b
#define __CREATE_DEFER_NAME(name, count) CAT_SYMBOL(name, count)
#define CREATE_DEFER_NAME __CREATE_DEFER_NAME(__defer_, __COUNTER__)
#define DEFER auto CREATE_DEFER_NAME = DeferHelper{} << [&]

/// 离开作用域后执行一个自定义函数
class Defer
{
public:
    Defer(std::function<void()> functor)
        : functor_(functor) {}

    ~Defer()
    {
        functor_();
    }

private:
    std::function<void()> functor_;
};

/// 辅助重载运算符 operator<< 的空类
struct DeferHelper
{
};

/// 运算符重载，用于方便宏快速创建 Defer 实例
auto operator<<(DeferHelper, std::function<void()> functor)
{
    return Defer{functor};
}

int main()
{
    const char* msg = "exit!! %ld\n";
    auto n = rand();
    DEFER
    {
        printf(msg, n);
    };
    
    if (n % 2 == 0)
    {
        printf("Even\n");
        return 0;
    }
    else
    {
        printf("Odd\n");
        return 1;
    }

}