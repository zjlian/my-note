/// 编译: g++ -std=c++11 function.cpp
/// 运行: ./a.out

#include <cstdio>
#include <vector>
#include <iostream>

//////////////// std::function 极简实现 //////////////////////

class FunctionBase
{
public:
    virtual ~FunctionBase() = default;
    virtual void Call() = 0;
};

template <typename T>
class FunctionImpl : public FunctionBase
{
public:
    FunctionImpl(T target)
        : target_(target) {}

    void Call() override
    {
        target_();
    }

private:
    T target_;
};

class Function
{
public:
    template <typename T>
    Function(T target)
    {
        impl_ = new FunctionImpl<T>{target};
    }

    ~Function()
    {
        if (impl_)
        {
            delete impl_;
        }
    }

    Function(const Function &) = delete;

    Function(Function &&other)
        : impl_(other.impl_)
    {
        other.impl_ = nullptr;
    }

    Function &operator=(const Function &) = delete;

    Function &operator=(Function &&other)
    {
        impl_ = other.impl_;
        other.impl_ = nullptr;
        return *this;
    }

    void operator()()
    {
        if (impl_)
        {
            impl_->Call();
        }
    }

private:
    FunctionBase *impl_ = nullptr;
};

////////////////////////////////////////////////////

/// 普通自由函数
void FreeFunc()
{
    printf("FreeFunc\n");
}

/// 实现了圆括号运算符的仿函数对象
struct ObjectFunc
{
    void operator()()
    {
        printf("ObjectFunc\n");
    }
};

/// 全局变量的 Lambda 实例
auto GlobalLambda = []
{
    printf("LambdaFunc\n");
};

struct BigObject
{
    char data_[1024];
};

int main()
{
    std::vector<Function> func_list;
    func_list.emplace_back(FreeFunc);
    func_list.emplace_back(ObjectFunc{});
    func_list.emplace_back(GlobalLambda);

    for (auto &fn : func_list)
    {
        fn();
    }

    BigObject b1;
    BigObject b2;
    BigObject b3;
    BigObject b4;

    auto lambda = [=]
    {
        printf("aa");
        printf("%c", b1.data_[0]);
        printf("%c", b2.data_[0]);
        printf("%c", b3.data_[0]);
        printf("%c", b4.data_[0]);
    };

    printf("%ld\n", sizeof(lambda));
}