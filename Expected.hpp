#pragma once

#include <utility>
#include <type_traits>
#include <new>

#if true
    struct BadExpectedAccess : public std::exception
    {
        const char* what() const noexcept override
        {
            return "Bad Expected Access";
        }
    };
    inline void ThrowBadExpectedAccess(bool throwException)
    {
        if (throwException)
        {
            throw BadExpectedAccess{};
        }
    }
#else
    inline void ThrowBadExpectedAccess(bool throwException)
    {
    }
#endif


template <typename E>
class Unexpected
{
public:
    constexpr Unexpected(const Unexpected&) = default;

    constexpr Unexpected(Unexpected&&) = default;

    template <typename E2>
    constexpr Unexpected(E2&& error)
        :_error(std::forward<E2>(error))
    {
    }

    E& error() & noexcept { return _error; }

    const E& error() const& noexcept { return _error; }

    E&& error() &&  noexcept { return std::move(_error); }

    const E&& error() const&& noexcept { return std::move(_error); }

private:
    E _error;
};


template <typename T, typename E>
class Expected
{
    static_assert(std::is_nothrow_destructible<T>::value, "T must be nothrow destructible");

public:

    //
    // 構築・破棄
    //

    // コンストラクタ
    constexpr Expected() noexcept
        : _hasValue(false)
        , _storage()
    {
    }


    // エラーの場合のコンストラクタ
    template <typename E2>
    Expected(Unexpected<E2>&& error) noexcept(std::is_nothrow_move_constructible<E2>::value)
        : _hasValue(false)
    {
        std::cout << "error-konnsutorakuta" << std::endl;
        setError(std::forward(error.error()));
    }


    // 正常値の場合のコンストラクタ
    template <typename T2,
        typename std::enable_if<std::is_constructible<T, T2>::value, std::nullptr_t>::type = nullptr>
    Expected(T2&& value) noexcept(std::is_nothrow_constructible<T2>::value)
        : _hasValue(true)
    {
        std::cout << value << "value-konnsutorakuta" << std::endl;
        setValue(std::forward<T2>(value));
    }


    // デストラクタ
    ~Expected() noexcept
    {
        std::cout << "destorakuta~" << std::endl;
        reset();
    }


    // コピーコンストラクタ
    template <typename T2, typename E2,
        typename std::enable_if<std::is_constructible<T, const T2&>::value && std::is_constructible<E, const E2&>::value, std::nullptr_t>::type = nullptr>
    Expected(const Expected<T2, E2>& other) noexcept(
        std::is_nothrow_copy_constructible<T2>::value && 
        std::is_nothrow_copy_constructible<E2>::value)
        : _hasValue(other._hasValue)
    {
        std::cout << "kopi-konnsutorakuta2" << std::endl;
        if (_hasValue)
        {
            setValue(other._storage.value);
        }
        else
        {
            setError(other._storage.error);
        }
    }


    // ムーブコンストラクタ
    template <typename T2, typename E2,
        typename std::enable_if<std::is_constructible<T, T2&&>::value && std::is_constructible<E, E2&&>::value, std::nullptr_t>::type = nullptr>
    Expected(Expected<T2, E2>&& other) noexcept(
        std::is_nothrow_move_constructible<T2>::value && 
        std::is_nothrow_move_constructible<E2>::value)
        :_hasValue(other._hasValue)
    {
        std::cout << "move-konnsutorakuta" << std::endl;
        if (_hasValue)
        {
            setValue(std::move(other._storage.value));
        }
        else
        {
            setError(std::move(other._storage.error));
        }
    }


    //
    // 代入
    //

    // 代入演算子
    template <typename T2, typename E2>
    constexpr Expected& operator=(Expected<T2, E2>&& other) noexcept(
        std::is_nothrow_move_constructible<T2>::value && 
        std::is_nothrow_move_constructible<E2>::value)
    {
        if (this != &other)
        {
            if (_hasValue && other._hasValue)
            {
                _storage.value = std::forward<T2>(other._storage.value);
            }
            else if (!_hasValue && !other._hasValue)
            {
                _storage.error = std::forward<E2>(other._storage.error);
            }
            else if (_hasValue)
            {
                reset();
                setError(std::forward<T2>(other._storage.error));
            }
            else
            {
                reset();
                setValue(std::forward<E2>(other._storage.value));
            }
            _hasValue = std::move(other._hasValue);
        }
        return *this;
    }


    // エラー値を代入するときはunexpectedにラップして代入する
    template <typename E2>
    constexpr Expected& operator=(Unexpected<E2>&& error)
    {
        reset();

        _hasValue = false;

        setError(std::forward<E2>(error.error()));

        return *this;
    }


    // 正常値を直接代入できるようにする
    constexpr Expected& operator=(T&& value) noexcept(std::is_nothrow_move_constructible<T>::value)
    {
        reset();

        _hasValue = true;

        setValue(std::forward<T>(value));

        return *this;
    }


    // 他の Expected オブジェクトとデータを入れ替える
    constexpr friend void swap(Expected& first, Expected& second) noexcept
    {
        if (first._hasValue)
        {
            if (second._hasValue)
            {
                std::swap(first._storage.value, second._storage.value);
            }
            else
            {
                E temp = std::move(second._storage.error);
                new (&second._storage.value) T(std::move(first._storage.value));
                new (&first._storage.error) E(std::move(temp));
            }
        }
        else
        {
            if (second._hasValue)
            {
                T temp = std::move(second._storage.value);
                new (&second._storage.error) E(std::move(first._storage.error));
                new (&first._storage.value) T(std::move(temp));
            }
            else
            {
                std::swap(first._storage.error, second._storage.error);
            }
        }
        std::swap(first._hasValue, second._hasValue);
    }


    //
    // 値の観測
    //

    // 正常値へのメンバアクセス
    T* operator->() noexcept
    {
        return &_storage.value;
    }
    const T* operator->() const noexcept
    {
        return &_storage.value;
    }


    // 正常値への間接参照
    T& operator*() & noexcept
    {
        return _storage.value;
    }

    const T& operator*() const& noexcept
    {
        return _storage.value;
    }

    T&& operator*() && noexcept
    {
        return std::move(_storage.value);
    }

    const T&& operator*() const&& noexcept
    {
        return std::move(_storage.value);
    }


    // 正常値を保持しているかを判定する
    constexpr operator bool() const noexcept
    {
        return _hasValue;
    }


    // 正常値を保持しているかを判定する
    constexpr bool hasValue() const noexcept 
    {
        return _hasValue; 
    }


    // 正常値を取得する
    T& value() &
    {
        ThrowBadExpectedAccess(!_hasValue);
        return _storage.value;
    }

    const T& value() const& 
    {
        ThrowBadExpectedAccess(!_hasValue);
        return _storage.value; 
    }

    T&& value() &&
    {
        ThrowBadExpectedAccess(!_hasValue);
        return std::move(_storage.value);
    }

    const T&& value() const&&
    {
        ThrowBadExpectedAccess(!_hasValue);
        return std::move(_storage.value);
    }


    // エラー値を取得する
    E& error() &
    { 
        ThrowBadExpectedAccess(_hasValue);
        return _storage.error; 
    }
    
    const E& error() const&
    { 
        ThrowBadExpectedAccess(_hasValue);
        return _storage.error; 
    }

    E&& error() &&
    {
        ThrowBadExpectedAccess(_hasValue);
        return std::move(_storage.error);
    }

    const E&& error() const&&
    {
        ThrowBadExpectedAccess(_hasValue);
        return std::move(_storage.value);
    }


    // 正常値もしくは指定された値を取得する
    constexpr T value_or(T&& default_value) const& noexcept(std::is_nothrow_move_constructible<T>::value)
    {
        return _hasValue ? _storage.value : std::move(default_value);
    }

    constexpr T value_or(T&& default_value) const&& noexcept(std::is_nothrow_move_constructible<T>::value)
    {
        return _hasValue ? std::move(_storage.value) : std::move(default_value);
    }


    // エラー値もしくは指定された値を取得する
    constexpr E error_or(E&& default_error) const& noexcept(std::is_nothrow_move_constructible<E>::value)
    {
        return _hasValue ? std::move(default_error) : _storage.error;
    }

    constexpr E error_or(E&& default_error) const&& noexcept(std::is_nothrow_move_constructible<E>::value)
    {
        return _hasValue ? std::move(default_error) : std::move(_storage.error);
    }


    //
    // モナド操作
    //

    // 正常値に対して関数を適用する
    template <typename F>
    constexpr auto and_then(F&& f) -> decltype(f(std::declval<T>()))
    {
        if (_hasValue)
        {
            return f(value());
        }
        else
        {
            return Unexpected<E>(error());
        }
    }


    // エラー値に対して関数を適用する	
    template <typename F>
    constexpr auto or_else(F&& f) -> decltype(f(std::declval<E>()))
    {
        if (_hasValue)
        {
            return value();
        }
        else
        {
            return f(error());
        }
    }


    // 正常値を変換する
    template <typename F>
    constexpr auto transform(F&& f) -> Expected<decltype(f(std::declval<T>())), E>
    {
        if (_hasValue)
        {
            return Expected<decltype(f(std::declval<T>())), E>{f(value())};
        }
        else
        {
            return Unexpected<E>{error()};
        }
    }


    // エラー値を変換する
    template <typename F>
    constexpr auto transform_error(F&& f) -> Expected<T, decltype(f(std::declval<E>()))>
    {
        if (_hasValue)
        {
            return value();
        }
        else
        {
            return Unexpected<decltype(f(std::declval<E>()))>{f(error())};
        }
    }


    //
    // 比較
    //

    // 等値比較
    template <typename T2, typename E2>
    friend constexpr bool operator==(const Expected& x, const Expected<T2, E2>& y)
    {
        if (x._hasValue != y._hasValue)
        {
            return false;
        }
        else if (x._hasValue)
        {
            return x._storage.value == y._storage.value;
        }
        else
        {
            return x._storage.error == y._storage.error;
        }
    }

    template <typename T2>
    friend constexpr bool operator==(const Expected& x, const T2& v)
    {
        return x._hasValue && x._storage.value == v;
    }

    template <typename E2>
    friend constexpr bool operator==(const Expected& x, const Unexpected<E2>& e)
    {
        return !x._hasValue && x._storage.error == e.error();
    }


private:
    bool _hasValue;

    union Storage
    {
        char dummy;
        T value;
        E error;

        Storage()
            :dummy()
        {}

        ~Storage(){}
    } _storage;


    void reset() noexcept
    {
        if (_hasValue)
        {
            _storage.value.~T();
        }
        else
        {
            _storage.error.~E();
        }
        _hasValue = false;
    }


    template <typename U>
    void setValue(U&& value) noexcept
    {
        std::cout << value << std::endl;
        new (&_storage.value) T(std::forward<U>(value));
    }

    template <typename U>
    void setError(U&& error) noexcept
    {
        new (&_storage.error) E(std::forward<U>(error));
    }
};


template <typename T, typename E>
Expected<T, E> make_expected(const T& value)
{
    return Expected<T, E>(value);
}

template <typename T, typename E>
Expected<T, E> make_expected(T&& value)
{
    return Expected<T, E>(std::move(value));
}

template <typename T, typename E>
Expected<T, E> make_Unexpected(const E& error)
{
    return Expected<T, E>(Unexpected<E>(error));
}

template <typename T, typename E>
Expected<T, E> make_Unexpected(E&& error)
{
    return Expected<T, E>(Unexpected<E>(std::move(error)));
}
