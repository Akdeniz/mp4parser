#ifndef CORE_OPTIONAL_H_
#define CORE_OPTIONAL_H_

#include <memory>
#include <stdexcept>
#include "TypeTraits.h"
#include "Move.h"
#include <new>


struct NullOptional_t
{
};
struct InPlace_t
{
};

namespace
{
const InPlace_t InPlace = {};

}

namespace core
{
template <typename valueType>
class optional
{
public:
    optional() : _hasValue(false) {}

    optional(NullOptional_t) : _hasValue(false) {}

    template <
        typename T,
        typename std::enable_if<
            std::is_same<
                typename std::remove_cv<typename std::remove_reference<T>::type>::type,
                valueType>::value,
            int>::type = 0>
    optional(T&& t) : _hasValue(true)
    {
        _construct(core::forward<T>(t));
    }

    optional(const optional& other) : _hasValue(other._hasValue)
    {
        if (has_value())
        {
            _construct(other._value);
        }
    }

    optional(optional&& other) : _hasValue(other._hasValue)
    {
        if (has_value())
        {
            _construct(std::move(other._value));
        }
    }

    template <typename U>
    optional(const optional<U>& other) : _hasValue(other._hasValue)
    {
        if (has_value())
        {
            construct(other._value);
        }
    }

    template <typename U>
    optional(optional<U>&& other) : _hasValue(other._hasValue)
    {
        if (has_value())
        {
            _construct(std::move(other._value));
        }
    }

    template <typename... Args>
    optional(InPlace_t, Args&&... args) : _hasValue(true)
    {
        _construct(core::forward<Args>(args)...);
    }

    ~optional() { reset(); }

    optional& operator=(NullOptional_t)
    {
        reset();
        return *this;
    }

    optional& operator=(const optional& other)
    {
        _assign(other);
        return *this;
    }

    optional& operator=(optional&& other)
    {
        _assign(std::move(other));
        return *this;
    }

    template <typename U>
    optional& operator=(const optional<U>& other)
    {
        _assign(other);
        return *this;
    }

    template <typename U>
    optional& operator=(optional<U>&& other)
    {
        _assign(std::move(other));
        return *this;
    }

    template <
        typename T,
        typename std::enable_if<
            std::is_same<
                typename std::remove_cv<typename std::remove_reference<T>::type>::type,
                valueType>::value,
            int>::type = 0>
    optional& operator=(T&& other)
    {
        if (has_value())
        {
            _value = core::forward<T>(other);
        }
        else
        {
            _construct(core::forward<T>(other));
            _hasValue = true;
        }
        return *this;
    }

    void reset()
    {
        if (has_value())
        {
            _value.~valueType();
            _hasValue = false;
        }
    }

    bool has_value() const { return _hasValue; }
    explicit operator bool() const { return has_value(); }

    valueType& get() &
    {
        _check_value();
        return _value;
    }

    valueType& get_or(valueType& default_value) &
    {
        if (!_hasValue)
        {
            return default_value;
        }
        _check_value();
        return _value;
    }

    const valueType& get() const&
    {
        _check_value();
        return _value;
    }

    const valueType& get_or(const valueType& default_value) const&
    {
        if (!_hasValue)
        {
            return default_value;
        }
        _check_value();
        return _value;
    }

    valueType&& get() &&
    {
        _check_value();
        return std::move(_value);
    }
    const valueType&& get() const&&
    {
        _check_value();
        return std::move(_value);
    }

    valueType& operator*() & { return get(); }
    const valueType& operator*() const& { return get(); }
    valueType&& operator*() && { return get(); }
    const valueType&& operator*() const&& { return get(); }

    valueType* operator->() { return &get(); }
    const valueType* operator->() const { return &get(); }

private:
    bool _hasValue;
    struct dummy
    {
    };
    union {
        dummy _dummy;
        valueType _value;
    };

    void _check_value() const
    {
        if (!_hasValue)
        {
            throw std::runtime_error("Dereferencing optional with no value");
        }
    }

    template <typename... Args>
    void _construct(Args&&... args)
    {
        new (&_value) valueType(core::forward<Args>(args)...);
    }

    template <typename OtherOptional>
    void _assign(OtherOptional&& other)
    {
        if (has_value())
        {
            if (other.has_value())
            {
                _value = core::forward<OtherOptional>(other).get();
            }
            else
            {
                reset();
            }
        }
        else
        {
            if (other.has_value())
            {
                _construct(core::forward<OtherOptional>(other).get());
                _hasValue = true;
            }
        }
    }
};

template <class T>
bool operator==(const optional<T>& x, const optional<T>& y)
{
    return x.has_value() != y.has_value() ? false : (x.has_value() ? *x == *y : true);
}

template <class T>
bool operator==(const optional<T>& x, NullOptional_t) noexcept
{
    return (!x);
}

template <class T>
bool operator==(NullOptional_t, const optional<T>& x) noexcept
{
    return (!x);
}

template <class T>
bool operator==(const optional<T>& x, const T& v)
{
    return x.has_value() ? *x == v : false;
}

template <class T>
bool operator==(const T& v, const optional<T>& x)
{
    return x.has_value() ? v == *x : false;
}

template <typename valueType, typename... Args>
inline optional<valueType> make_optional(Args&&... args)
{
    return optional<valueType>(InPlace, core::forward<Args>(args)...);
}

template <typename... Args>
bool has_value(const optional<Args...>& value)
{
    return value.has_value();
}

const NullOptional_t NullOptional =
    {};  // see http://open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#253

}  // namespace core

#endif  // CORE_OPTIONAL_H_
