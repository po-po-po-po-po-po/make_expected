#include <iostream>
#include <cassert>
#include "Expected.hpp"

template<typename T, typename U>
void EXPECT_EQ(T var1, U var2)
{
    assert(static_cast<bool>(var1 == var2));
}

void EXPECT_TRUE(bool result)
{
    assert(result);
}

void EXPECT_FALSE(bool result)
{
    assert(!result);
}


struct Copyable
{
    int value;

    Copyable(int value = 0)
        : value(value){};

    ~Copyable() noexcept {};

    Copyable(const Copyable& other)
        : value(other.value){};

    Copyable(Copyable&&) noexcept = delete;

    Copyable& operator=(const Copyable& other)
    {
        value = other.value;
        return *this;
    };

    Copyable& operator=(Copyable&&) noexcept = delete;
};


struct CopyableDerived
{
    int value;

    CopyableDerived(int value = 0)
        : value(value){};

    ~CopyableDerived() noexcept {};

    CopyableDerived(const Copyable& other)
        : value(other.value)
    {
    }    // implicit conversion from Copyable to Copyable1

    CopyableDerived(CopyableDerived&&) noexcept = delete;

    CopyableDerived& operator=(const CopyableDerived& other)
    {
        value = other.value;
        return *this;
    };

    CopyableDerived& operator=(CopyableDerived&&) noexcept = delete;
};


struct Uncopyable
{
    int value;

    Uncopyable(int value = 0)
        : value(value){};

    ~Uncopyable() noexcept {};

    Uncopyable(const Uncopyable&) = delete;

    Uncopyable(Uncopyable&& other) noexcept
        : value(other.value){};

    Uncopyable& operator=(const Uncopyable&) = delete;

    Uncopyable& operator=(Uncopyable&& other) noexcept
    {
        value = other.value;
        return *this;
    };
};


struct UncopyableDerived
{

    int value;

    UncopyableDerived(int value = 0)
        : value(value){};

    ~UncopyableDerived() noexcept {};

    UncopyableDerived(const UncopyableDerived&) = delete;

    UncopyableDerived(Uncopyable&& other) noexcept
        : value(other.value)
    {
    }    // implicit conversion from Uncopyable to Uncopyable1

    UncopyableDerived& operator=(const UncopyableDerived&) = delete;

    UncopyableDerived& operator=(UncopyableDerived&& other) noexcept
    {
        value = other.value;
        return *this;
    };
};


void p(int number)
{
    std::cout << "now : " << number << std::endl;
}

int main()
{
    {
        // {
        //     Expected<Copyable, Copyable> a;
        //     EXPECT_FALSE(a);
        //     p(1);
        // }
        // {
        //     Expected<Copyable, Copyable> a{100};
        //     EXPECT_TRUE(a);
        //     p(2);
        //     EXPECT_EQ(a->value, 100);
        //     p(3);
        // }
        {
            Expected<Copyable, Copyable> a = 100;
            std::cout << a->value << std::endl;
        }
        {
            Expected<int, Copyable> a = 100;
            std::cout << *a << std::endl;
        }
    }
}
