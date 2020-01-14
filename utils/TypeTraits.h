#ifndef CORE_TYPE_TRAITS_H_
#define CORE_TYPE_TRAITS_H_

#include <type_traits>

namespace core
{
template <typename A>
using remove_cv = typename std::remove_cv<A>;

template <bool B, class T = void>
using enable_if = typename std::enable_if<B, T>;

template <typename A>
using remove_reference = typename std::remove_reference<A>;

using std::decay;

template <bool B, class T, class U>
using conditional = typename std::conditional<B, T, U>;

template <typename Base, typename Derived>
using is_base_of = typename std::is_base_of<Base, Derived>;

template <typename A, typename B>
using is_same = typename std::is_same<A, B>;

template <typename A>
using is_void = typename std::is_void<A>;

template <typename A>
using remove_pointer = typename std::remove_pointer<A>;

template <typename A>
using is_pointer = typename std::is_pointer<A>;

template <typename A>
using is_floating_point = typename std::is_floating_point<A>;

template <typename A>
using is_integral = typename std::is_integral<A>;

template <typename A>
using is_arithmetic = typename std::is_arithmetic<A>;

template <typename A>
using is_unsigned = typename std::is_unsigned<A>;

template <typename F, typename T>
using is_convertible = typename std::is_convertible<F, T>;

template <typename A>
using is_enum = typename std::is_enum<A>;

template <typename A>
using is_default_constructible = typename std::is_default_constructible<A>;

template <typename A>
using underlying_type = typename std::underlying_type<A>;

#define SCOPED_ENUM_DECLARE_BEGIN(Type) enum class Type : int
#define SCOPED_ENUM_DECLARE_END(Type) ;

}  // namespace core


#endif
