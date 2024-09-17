#pragma once

// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <cstring>
#include <ciso646>
#include <type_traits>
#include <utility>
#include <limits> // std::numeric_limits
#include <exception> // std::exception
#include <cstdint>
#include <functional> // std::invoke, std::hash

#if __has_include(<opt/option_fwd.hpp>)
    #include <opt/option_fwd.hpp>
#else
namespace opt {
template<class T>
class option;

template<class T, class = void>
struct option_traits;

template<class>
struct is_option { static constexpr bool value = false; };
template<class T>
struct is_option<option<T>> { static constexpr bool value = true; };

template<class T>
inline constexpr bool is_option_v = is_option<T>::value;

struct option_tag {};

namespace impl {
    struct none_tag {};
}
struct none_t {
    constexpr explicit none_t(impl::none_tag) {}
};
inline constexpr none_t none{impl::none_tag{}};
}
#endif

#ifdef __INTEL_COMPILER
    #define OPTION_CLANG 0
    #define OPTION_GCC   0
    #define OPTION_MSVC  0
    #define OPTION_INTEL 1
#elif defined(_MSC_VER)
    #ifdef __clang__
        #define OPTION_CLANG 1
    #else
        #define OPTION_CLANG 0
    #endif
    #define OPTION_GCC   0
    #define OPTION_MSVC  1
    #define OPTION_INTEL 0
#elif defined(__clang__)
    #define OPTION_CLANG 1
    #define OPTION_GCC   0
    #define OPTION_MSVC  0
    #define OPTION_INTEL 0
#elif defined(__GNUC__) || defined(__GNUG__)
    #define OPTION_CLANG 0
    #define OPTION_GCC   1
    #define OPTION_MSVC  0
    #define OPTION_INTEL 0
#else
    #define OPTION_CLANG 0
    #define OPTION_GCC   0
    #define OPTION_MSVC  0
    #define OPTION_INTEL 0
#endif

#if defined(_LIBCPP_VERSION)
    #define OPTION_LIBCPP 1
    #define OPTION_LIBSTDCPP 0
    #define OPTION_STL 0
    #define OPTION_UNKNOWN_STD 0
#elif defined(__GLIBCXX__)
    #define OPTION_LIBCPP 0
    #define OPTION_LIBSTDCPP 1
    #define OPTION_STL 0
    #define OPTION_UNKNOWN_STD 0
#elif defined(_MSC_VER)
    #define OPTION_LIBCPP 0
    #define OPTION_LIBSTDCPP 0
    #define OPTION_STL 1
    #define OPTION_UNKNOWN_STD 0
#else
    #define OPTION_LIBCPP 0
    #define OPTION_LIBSTDCPP 0
    #define OPTION_STL 0
    #define OPTION_UNKNOWN_STD 1
#endif

#ifdef __has_attribute
    #define OPTION_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
    #define OPTION_HAS_ATTRIBUTE(x) (0)
#endif

#ifndef __has_cpp_attribute
    #define OPTION_LIFETIMEBOUND
#elif __has_cpp_attribute(msvc::lifetimebound)
    #define OPTION_LIFETIMEBOUND [[msvc::lifetimebound]]
#elif __has_cpp_attribute(clang::lifetimebound)
    #define OPTION_LIFETIMEBOUND [[clang::lifetimebound]]
#else
    #define OPTION_LIFETIMEBOUND
#endif

#if OPTION_HAS_ATTRIBUTE(pure)
    #define OPTION_PURE __attribute__((pure))
#elif OPTION_MSVC
    #define OPTION_PURE __declspec(noalias)
#else
    #define OPTION_PURE
#endif

// Define to 1 to use only quiet NaN in `opt::option<T>`, where T is floating point type.
// Define to 0 to use signaling NaN if available, or use quiet NaN instead.
// Default is 0.
#ifndef OPTION_USE_QUIET_NAN
    #define OPTION_USE_QUIET_NAN 0
#endif

#ifdef OPTION_USE_PFR
    #if OPTION_USE_PFR
        #if __has_include(OPTION_PFR_FILE)
            #include OPTION_PFR_FILE
            #if defined(BOOST_PFR_ENABLED)
                #if BOOST_PFR_ENABLED
                    #define OPTION_HAS_PFR
                    #define OPTION_PFR_NAMESPACE ::boost::pfr::
                #endif
            #elif defined(PFR_ENABLED)
                #if PFR_ENABLED
                    #define OPTION_HAS_PFR
                    #define OPTION_PFR_NAMESPACE ::pfr::
                #endif
            #else
                #error "The 'boost.pfr' or 'ptr' library have wrong installation. The 'BOOST_PFR_ENABLED' and 'PFR_ENABLED' macro are not defined"
            #endif
        #else
            #error "The 'boost.pfr' or 'pfr' library were not found. Define the 'OPTION_PFR_FILE' macro to specify a custom path to the 'boost.pfr' or 'pfr' library header"
        #endif
    #endif
#else // !defined(OPTION_USE_BOOST_PFR)
    #ifdef OPTION_PFR_FILE
        #if __has_include(OPTION_PFR_FILE)
            #include OPTION_PFR_FILE
            #if BOOST_PFR_ENABLED
                #define OPTION_HAS_PFR
            #endif
        #endif
    #else
        #if __has_include(<boost/pfr.hpp>)
            #include <boost/pfr.hpp>
            #if BOOST_PFR_ENABLED
                #define OPTION_HAS_PFR
                #define OPTION_PFR_NAMESPACE ::boost::pfr::
            #endif
        #elif __has_include(<pfr.hpp>)
            #include <pfr.hpp>
            #if PFR_ENABLED
                #define OPTION_HAS_PFR
                #define OPTION_PFR_NAMESPACE ::pfr::
            #endif
        #endif
    #endif
#endif

#ifndef OPTION_USE_BUILTIN_TRAITS
    #define OPTION_USE_BUILTIN_TRAITS 1
#endif

#ifndef OPTION_LIBASSERT_FILE
    #define OPTION_LIBASSERT_FILE <libassert/assert.hpp>
#endif

#ifdef OPTION_USE_LIBASSERT
    #if OPTION_USE_LIBASSERT
        #if __has_include(OPTION_LIBASSERT_FILE)
            #include OPTION_LIBASSERT_FILE
            #define OPTION_HAS_LIBASSERT
        #else
            #error "The 'libassert' library was not found. Define the 'OPTION_LIBASSERT_FILE' macro to specify a custom path to the 'libassert' library header"
        #endif 
    #endif
#else
    #if __has_include(OPTION_LIBASSERT_FILE)
        #include OPTION_LIBASSERT_FILE
        #define OPTION_HAS_LIBASSERT
    #endif
#endif

// Macro `OPTION_VERIFY` is used in `opt::option<T>::get`, `opt::option<T>::operator*`.
// You can also redefine `OPTION_VERIFY` to specify custom behavior when something goes unexpected.
#ifndef OPTION_VERIFY
    #ifndef NDEBUG
        #ifdef OPTION_HAS_LIBASSERT
            #define OPTION_VERIFY(expression, message) LIBASSERT_ASSUME(expression, message)
        #else
            #if OPTION_CLANG
                #define OPTION_DEBUG_BREAK __builtin_debugtrap()
            #elif OPTION_MSVC || OPTION_INTEL
                #define OPTION_DEBUG_BREAK __debugbreak()
            #elif OPTION_GCC
                #define OPTION_DEBUG_BREAK __builtin_trap()
            #else
                #include <csignal>
                #if defined(SIGTRAP)
                    #define OPTION_DEBUG_BREAK ::std::raise(SIGTRAP)
                #else
                    #define OPTION_DEBUG_BREAK ::std::raise(SIGABRT)
                #endif
            #endif
            // Print an error message and call a debug break if the expression is evaluated as false
            #include <cstdio>
            #define OPTION_VERIFY(expression, message) \
                ((expression) ? (void)0 : ( \
                    (void)::std::fprintf(stderr, "%s:%d: assertion '%s' failed: %s\n", __FILE__, __LINE__, #expression, message), \
                    (void)::std::fflush(stderr), \
                    (void)OPTION_DEBUG_BREAK) \
                )
        #endif
    #else
        #if OPTION_MSVC
            #define OPTION_VERIFY(expression, message) __assume(expression)
        #elif OPTION_CLANG
            #define OPTION_VERIFY(expression, message) __builtin_assume(expression)
        #elif OPTION_GCC
            #if __GNUC__ >= 13
                #define OPTION_VERIFY(expression, message) __attribute__((__assume__(expression)))
            #else
                #define OPTION_VERIFY(expression, message) if (expression) {} else { __builtin_unreachable(); }
            #endif
        #else
            #define OPTION_VERIFY(expression, message)
        #endif
    #endif
#endif

#ifdef _MSVC_LANG
    #if _MSVC_LANG > __cplusplus
        #define OPTION_CXX_VER _MSVC_LANG
    #else
        #define OPTION_CXX_VER __cplusplus
    #endif
#else
    #define OPTION_CXX_VER __cplusplus
#endif

#if OPTION_CXX_VER >= 202002L
    #define OPTION_IS_CXX20 1
#else
    #define OPTION_IS_CXX20 0
#endif

#if OPTION_IS_CXX20
    #define OPTION_CONSTEXPR_CXX20 constexpr
#else
    #define OPTION_CONSTEXPR_CXX20 inline
#endif

#if OPTION_GCC || OPTION_CLANG
    #define OPTION_CURRENT_FUNCTION() __PRETTY_FUNCTION__
#elif OPTION_MSVC
    #define OPTION_CURRENT_FUNCTION() __builtin_FUNCSIG()
#endif

#ifdef OPTION_CURRENT_FUNCTION
    #define OPTION_CAN_REFLECT_ENUM 1
#else
    #define OPTION_CAN_REFLECT_ENUM 0
#endif

#ifndef OPTION_FORWARD_DECLARE_STD
    #define OPTION_FORWARD_DECLARE_STD 1
#endif

#if OPTION_FORWARD_DECLARE_STD
    #if OPTION_LIBCPP && defined(_LIBCPP_BEGIN_NAMESPACE_STD) && defined(_LIBCPP_END_NAMESPACE_STD)
        #define OPTION_STD_NAMESPACE_BEGIN _LIBCPP_BEGIN_NAMESPACE_STD
        #define OPTION_STD_NAMESPACE_END _LIBCPP_END_NAMESPACE_STD
        #define OPTION_STD_NAMESPACE_CXX11_BEGIN
        #define OPTION_STD_NAMESPACE_CXX11_END
    #elif OPTION_LIBSTDCPP && defined(_GLIBCXX_VISIBILITY) && defined(_GLIBCXX_BEGIN_NAMESPACE_VERSION) && defined(_GLIBCXX_END_NAMESPACE_VERSION)
        #define OPTION_STD_NAMESPACE_BEGIN namespace std _GLIBCXX_VISIBILITY(default) { _GLIBCXX_BEGIN_NAMESPACE_VERSION
        #define OPTION_STD_NAMESPACE_END _GLIBCXX_END_NAMESPACE_VERSION }
        #define OPTION_STD_NAMESPACE_CXX11_BEGIN inline _GLIBCXX_BEGIN_NAMESPACE_CXX11
        #define OPTION_STD_NAMESPACE_CXX11_END _GLIBCXX_END_NAMESPACE_CXX11
    #elif OPTION_STL && defined(_STD_BEGIN) && defined(_STD_END)
        #define OPTION_STD_NAMESPACE_BEGIN \
            _STD_BEGIN \
            _Pragma("warning(push)") \
            _Pragma("warning(disable : 4099 4643)")
        #define OPTION_STD_NAMESPACE_END \
            _Pragma("warning(pop)") \
            _STD_END
        #define OPTION_STD_NAMESPACE_CXX11_BEGIN
        #define OPTION_STD_NAMESPACE_CXX11_END
    #else
        #define OPTION_STD_NAMESPACE_BEGIN namespace std {
        #define OPTION_STD_NAMESPACE_END }
        #define OPTION_STD_NAMESPACE_CXX11_BEGIN
        #define OPTION_STD_NAMESPACE_CXX11_END
    #endif

OPTION_STD_NAMESPACE_BEGIN
    template<class CharT, class Traits>
    class basic_string_view; // Defined in header <string_view>

OPTION_STD_NAMESPACE_CXX11_BEGIN
    template<class CharT, class Traits, class Allocator>
    class basic_string; // Defined in header <string>
OPTION_STD_NAMESPACE_CXX11_END

    template<class T, class Allocator>
    class vector; // Defined in header <vector>

    template<class T, class Deleter>
    class unique_ptr; // Defined in header <memory>

    template<class T>
    struct default_delete; // Defined in header <memory>

    template<class T, std::size_t N>
    struct array; // Defined in header <array>

    template<class... Types>
    class variant; // Defined in header <variant>

#if OPTION_MSVC
    template<std::size_t I, class... Types>
    constexpr auto get_if(variant<Types...>*) noexcept;
    template<std::size_t I, class... Types>
    constexpr auto get_if(const variant<Types...>*) noexcept;
#else
    template<std::size_t N, class T>
    struct variant_alternative;

    template<std::size_t I, class... Types>
    constexpr add_pointer_t<typename variant_alternative<I, variant<Types...>>::type> get_if(variant<Types...>*) noexcept;
    template<std::size_t I, class... Types>
    constexpr add_pointer_t<const typename variant_alternative<I, variant<Types...>>::type> get_if(const variant<Types...>*) noexcept;
#endif
    struct input_iterator_tag; // Defined in header <iterator>
    struct output_iterator_tag; // Defined in header <iterator>
    struct forward_iterator_tag; // Defined in header <iterator>
    struct bidirectional_iterator_tag; // Defined in header <iterator>
    struct random_access_iterator_tag; // Defined in header <iterator>
#if OPTION_IS_CXX20
    struct contiguous_iterator_tag; // Defined in header <iterator>
#endif
OPTION_STD_NAMESPACE_END
#else
    #include <string_view>
    #include <string>
    #include <vector>
    #include <memory>
    #include <array>
    #include <variant>
#endif

#if OPTION_IS_CXX20
    #include <memory> // for std::construct_at
#endif

namespace opt {

template<class T>
inline constexpr bool has_option_traits = opt::option_traits<T>::max_level >= 1;

namespace impl {
    struct dummy_type_for_traits {};
}
template<>
struct option_traits<impl::dummy_type_for_traits> {};

namespace impl {
    template<class T, class... Args>
    constexpr void construct_at(T* ptr, Args&&... args) {
        if constexpr (std::is_trivially_move_assignable_v<T>) {
            *ptr = T{std::forward<Args>(args)...};
        } else {
#if OPTION_IS_CXX20
            std::construct_at(ptr, std::forward<Args>(args)...);
#else
            ::new(static_cast<void*>(ptr)) T{std::forward<Args>(args)...};
#endif
        }
    }

    template<class T>
    constexpr void destroy_at(T* ptr) {
        ptr->~T();
    }

    template<class T, class Traits, class = std::uintmax_t>
    inline constexpr bool has_get_level_method = false;
    template<class T, class Traits>
    inline constexpr bool has_get_level_method<T, Traits, decltype(static_cast<std::uintmax_t>(Traits::get_level(std::declval<const T*>())))>
        = noexcept(static_cast<std::uintmax_t>(Traits::get_level(std::declval<const T*>())));

    template<class T, class Traits, class = void>
    inline constexpr bool has_set_level_method = false;
    template<class T, class Traits>
    inline constexpr bool has_set_level_method<T, Traits, decltype(Traits::set_level(std::declval<T*>(), std::declval<std::uintmax_t>()))>
        = noexcept(Traits::set_level(std::declval<T*>(), std::declval<std::uintmax_t>()));

    template<class T, class = void>
    inline constexpr bool has_sentinel_member = false;
    template<class T>
    inline constexpr bool has_sentinel_member<T, std::void_t<decltype(std::declval<T&>().SENTINEL)>> = true;

    template<class T, class = void>
    inline constexpr bool has_sentinel_enumerator = false;
    template<class T>
    inline constexpr bool has_sentinel_enumerator<T, std::void_t<decltype(T::SENTINEL)>> = true;

    template<class T, class = void>
    inline constexpr bool has_sentinel_start_enumerator = false;
    template<class T>
    inline constexpr bool has_sentinel_start_enumerator<T, std::void_t<decltype(T::SENTINEL_START)>> = true;

    template<class T, class = void>
    inline constexpr bool has_sentinel_end_enumerator = false;
    template<class T>
    inline constexpr bool has_sentinel_end_enumerator<T, std::void_t<decltype(T::SENTINEL_END)>> = true;

    template<class T, class = void>
    inline constexpr bool is_custom_tuple_like = false;
    template<class T>
    inline constexpr bool is_custom_tuple_like<T, std::void_t<decltype(std::tuple_size<T>::value)>> = true;

    template<class...>
    using expand_to_true = std::true_type;
    template<class...>
    using expand_to_false = std::false_type;

    template<class... Pred>
    expand_to_true<std::enable_if_t<Pred::value>...> and_helper(int);
    template<class...>
    std::false_type and_helper(...);

    template<class... Pred>
    using and_ = decltype(and_helper<Pred...>(0));

    template<class... Pred>
    expand_to_false<std::enable_if_t<!Pred::value>...> or_helper(int);
    template<class...>
    std::true_type or_helper(...);

    template<class... Pred>
    using or_ = decltype(or_helper<Pred...>(0));

    template<class Pred>
    using not_ = std::bool_constant<!Pred::value>;

    template<bool>
    struct if_impl {
        template<class If, class Else>
        using type = If;
    };
    template<>
    struct if_impl<false> {
        template<class If, class Else>
        using type = Else;
    };
    template<bool Condition, class If, class Else>
    using if_ = typename if_impl<Condition>::template type<If, Else>;

#if OPTION_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#elif OPTION_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wclass-memaccess"
    #pragma GCC diagnostic ignored "-Wuninitialized"
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    template<class To, class From>
    To ptr_bit_cast(const From* const from) noexcept {
        static_assert(sizeof(To) == sizeof(From));

        To res;
        std::memcpy(&res, from, sizeof(To));
        return res;
    }
    template<class Src, class Dest>
    void ptr_bit_copy(Dest* const dest, const Src& src) noexcept {
        static_assert(sizeof(Src) == sizeof(Dest));

        std::memcpy(dest, &src, sizeof(Dest));
    }
    template<class To, class From>
    To ptr_bit_cast_least(const From* const from) noexcept {
        static_assert(sizeof(From) >= sizeof(To));

        To res;
        std::memcpy(&res, from, sizeof(To));
        return res;
    }
    template<class Src, class Dest>
    void ptr_bit_copy_least(Dest* const dest, const Src& src) noexcept {
        static_assert(sizeof(Dest) >= sizeof(Src));
        std::memcpy(dest, &src, sizeof(Src));
    }
#if OPTION_CLANG
    #pragma clang diagnostic pop
#elif OPTION_GCC
    #pragma GCC diagnostic pop
#endif

    template<std::uintmax_t level, class Type, std::size_t var_index, std::size_t index, class... Ts>
    struct select_max_level_traits_impl;

#if OPTION_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4296) // 'operator' : expression is always false
#endif

    template<
        std::uintmax_t level,
        class Type,
        std::size_t var_index,
        std::size_t index,
        class T, class... Ts
    >
    struct select_max_level_traits_impl<level, Type, var_index, index, T, Ts...>
        : select_max_level_traits_impl<
            (opt::option_traits<T>::max_level > level) ? opt::option_traits<T>::max_level : level,
            if_<(opt::option_traits<T>::max_level > level), T, Type>,
            var_index + 1,
            (opt::option_traits<T>::max_level > level) ? var_index : index,
            Ts...
        >
    {};

    template<std::uintmax_t level_, class Type, std::size_t var_index, std::size_t index_>
    struct select_max_level_traits_impl<level_, Type, var_index, index_> {
        static constexpr std::uintmax_t level = level_;
        static constexpr std::size_t index = index_;
        using type = Type;
    };

    template<class... Ts>
    struct select_max_level_traits
        : select_max_level_traits_impl<0, dummy_type_for_traits, 0, 0, Ts...> {};

#ifdef OPTION_HAS_PFR
    template<class Struct>
    inline constexpr bool pfr_is_option_reflectable =
        OPTION_PFR_NAMESPACE is_implicitly_reflectable_v<Struct, option_tag>;

    template<class Struct>
    struct unpack_tuple_select_max_level_traits;
    template<class... Ts>
    struct unpack_tuple_select_max_level_traits<std::tuple<Ts&...>>
        : select_max_level_traits<Ts...> {};
#endif

#if OPTION_MSVC
    #pragma warning(pop)
#endif

#if OPTION_CAN_REFLECT_ENUM
    template<auto... Enumerators>
    constexpr std::uintmax_t max_enum_value_impl2(const std::uintmax_t max) {
        const char* const name = OPTION_CURRENT_FUNCTION();
#if !OPTION_GCC
        const std::size_t size = __builtin_strlen(name);

        const char* const start = __builtin_char_memchr(name, '<', size) + 1;
    #if OPTION_CLANG && __clang_major__ <= 12
        if (*start < '0' || *start > '9') { return 0; }
    #else
        if (*start != '(') { return 0; }
    #endif

        std::uintmax_t current = max;
        for (const char* it = start;;) {
            current -= 1;
            it = __builtin_char_memchr(it, ',', size - std::size_t(it - name));

            if (it == nullptr) { return 0; }
    #if OPTION_CLANG
        #if __clang_major__ <= 12
            it += 2;
            if (*it < '0' || *it > '9') { break; }
        #else
            it += 2;
            if (*it != '(') { break; }
            it = __builtin_char_memchr(it, ')', size - std::size_t(it - name));
            if (it[1] == ':') { break; }
        #endif
    #else
            it += 1;
            if (*it != '(') { break; }
    #endif
        }
#else
        std::size_t start = 0;
        while (name[start] != '{') {
            start += 1;
        }
        if (name[start + 1] != '(') { return 0; }

        std::uintmax_t current = max;
        for (std::size_t i = start + 1;;) {
            current -= 1;
            for (; name[i] != ','; ++i) {
                if (name[i] == '}') { return 0; }
            }
            i += 2; // skip whitespace
            if (name[i] != '(') { break; }
        }
#endif
        // https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
        // Round up to the next highest power of 2
        current--;
        current |= current >> 1;
        current |= current >> 2;
        current |= current >> 4;
        current |= current >> 8;
        // current |= current >> 16;
        // current |= current >> 32;
        current++;

        return current;
    }

    template<class E, std::uintmax_t Max, class T, T... Vals>
    constexpr std::uintmax_t max_enum_value_impl1(std::integer_sequence<T, Vals...>) {
        return max_enum_value_impl2<static_cast<E>(Max - 1 - Vals)...>(Max);
    }
    template<class E, std::uintmax_t Max>
    constexpr std::uintmax_t max_enum_value() {
        return max_enum_value_impl1<E, Max>(
            std::make_integer_sequence<std::uintmax_t, Max>{}
        );
    }
#endif

    template<class TupleLike, class IndexSeq = std::make_integer_sequence<std::size_t, std::tuple_size<TupleLike>::value>>
    struct unpack_tuple_like_select_max_level_traits;

    template<class TupleLike, std::size_t... Index>
    struct unpack_tuple_like_select_max_level_traits<TupleLike, std::integer_sequence<std::size_t, Index...>>
        : select_max_level_traits<std::tuple_element_t<Index, TupleLike>...> {};

    namespace tuple_like_get_impl {
#if OPTION_MSVC
        template<std::size_t>
        void get();
#else
        template<std::size_t>
        void get() = delete;
#endif

        template<std::size_t I, class T>
        constexpr auto call(T& x, int) -> decltype(x.template get<I>()) {
            return x.template get<I>();
        }
        template<std::size_t I, class T>
        constexpr auto call(T& x, long) -> decltype(get<I>(x)) {
            return get<I>(x);
        }
    }

    template<std::size_t I, class T>
    constexpr auto& tuple_like_get(T& x) {
        return tuple_like_get_impl::call<I>(x, 1);
    }

    enum class option_strategy {
        none,
        other,
        bool_,
        reference_wrapper,
        pair,
        tuple,
        array,
        avaliable_option,
        unavaliable_option,
        reference,
        pointer_64,
        pointer_32,
        float64_sNaN,
        float64_qNaN,
        float32_sNaN,
        float32_qNaN,
        polymorphic,
        string_view,
#if !OPTION_UNKNOWN_STD
        string,
        vector,
#endif
        unique_ptr,
        member_pointer_32,
        member_pointer_64,
        sentinel_member,
        tuple_like,
        enumeration_sentinel,
        enumeration_sentinel_start,
        enumeration_sentinel_start_end,
#if OPTION_CAN_REFLECT_ENUM
        enumeration,
#endif
#ifdef OPTION_HAS_PFR
        reflectable,
#endif
    };

    template<class T>
    struct dispatch_specializations {
        static constexpr option_strategy value = option_strategy::other;
    };
    template<class T>
    struct dispatch_specializations<opt::option<T>> {
        static constexpr option_strategy value
            = opt::option_traits<T>::max_level >= 1
                ? option_strategy::avaliable_option
                : option_strategy::unavaliable_option;
    };
    template<>
    struct dispatch_specializations<bool> {
        static constexpr option_strategy value = option_strategy::bool_;
    };
    template<class T>
    struct dispatch_specializations<std::reference_wrapper<T>> {
        static constexpr option_strategy value = option_strategy::reference_wrapper;
    };
    template<class First, class Second>
    struct dispatch_specializations<std::pair<First, Second>> {
        static constexpr option_strategy value = option_strategy::pair;
    };
    template<class T0, class... Ts>
    struct dispatch_specializations<std::tuple<T0, Ts...>> {
        static constexpr option_strategy value = option_strategy::tuple;
    };
    template<class T, std::size_t N>
    struct dispatch_specializations<std::array<T, N>> {
        static constexpr option_strategy value = (N == 0) ? option_strategy::none : option_strategy::array;
    };
    template<class Elem, class Traits>
    struct dispatch_specializations<std::basic_string_view<Elem, Traits>> {
        static constexpr option_strategy value = option_strategy::string_view;
    };
#if !OPTION_UNKNOWN_STD
    template<class Elem, class Traits, class Allocator>
    struct dispatch_specializations<std::basic_string<Elem, Traits, Allocator>> {
        static constexpr bool ebo_allocator = std::is_empty_v<Allocator> && !std::is_final_v<Allocator>;
        static constexpr option_strategy value = ebo_allocator ? option_strategy::string : option_strategy::none;
    };
    template<class Elem, class Allocator>
    struct dispatch_specializations<std::vector<Elem, Allocator>> {
        static constexpr bool ebo_allocator = std::is_empty_v<Allocator> && !std::is_final_v<Allocator>;
        static constexpr option_strategy value = ebo_allocator ? option_strategy::vector : option_strategy::none;
    };
#endif
    template<class Elem>
    struct dispatch_specializations<std::unique_ptr<Elem, std::default_delete<Elem>>> {
        static constexpr option_strategy value = option_strategy::unique_ptr;
    };

    template<class T>
    constexpr option_strategy detemine_option_strategy() {
        using st = option_strategy;
        constexpr st dispatch_st = dispatch_specializations<T>::value;

        if constexpr (dispatch_st != st::other) {
            return dispatch_st;
        } else
        if constexpr (std::is_reference_v<T>) {
            return st::reference;
        } else
        if constexpr (std::is_pointer_v<T>) {
            if constexpr (sizeof(T) == 8) {
                return st::pointer_64;
            } else
            if constexpr (sizeof(T) == 4) {
                return st::pointer_32;    
            } else
            return st::none;
        } else
        if constexpr (std::is_member_pointer_v<T>) {
            if constexpr (sizeof(T) == 4) {
                return st::member_pointer_32;
            } else
            if constexpr (sizeof(T) >= 8) {
                return st::member_pointer_64;
            } else
            return st::none;
        } else
        if constexpr (std::is_floating_point_v<T>) {
            using limits = std::numeric_limits<T>;
            if constexpr (!limits::is_iec559 || (!limits::has_signaling_NaN && !limits::has_quiet_NaN)) {
                return st::none;
            } else
            if constexpr (sizeof(T) == 8) {
                if constexpr (sizeof(std::uintptr_t) == 4) {
                    return st::float64_qNaN;
                } else {
                    return limits::has_signaling_NaN ? st::float64_sNaN : st::float64_qNaN;
                }
            } else
            if constexpr (sizeof(T) == 4) {
                if constexpr (sizeof(std::uintptr_t) == 4) {
                    return st::float32_qNaN;
                } else {
                    return limits::has_signaling_NaN ? st::float32_sNaN : st::float32_qNaN;
                }
            } else
            return st::none;
        } else
        if constexpr (std::is_polymorphic_v<T> && sizeof(T) >= sizeof(std::uintptr_t)) {
            return st::polymorphic;
        } else
        if constexpr (has_sentinel_member<T>) {
            return st::sentinel_member;
        } else
        if constexpr (std::is_enum_v<T>) {
            if constexpr (has_sentinel_enumerator<T>) {
                return st::enumeration_sentinel;
            } else
            if constexpr (has_sentinel_start_enumerator<T>) {
                if constexpr (has_sentinel_end_enumerator<T>) {
                    return st::enumeration_sentinel_start_end;
                } else {
                    return st::enumeration_sentinel_start;
                }
            } else
#if OPTION_CAN_REFLECT_ENUM
            if constexpr (std::is_unsigned_v<std::underlying_type_t<T>>) {
                return st::enumeration;    
            } else
#endif
            return st::none;
        } else
        if constexpr (is_custom_tuple_like<T>) {
            return st::tuple_like;
        } else
#ifdef OPTION_HAS_PFR
        if constexpr (pfr_is_option_reflectable<T>) {
            return st::reflectable;
        } else
#endif
        return st::none;
    }

    template<class T, option_strategy strategy = detemine_option_strategy<T>()>
    struct internal_option_traits {
        static constexpr std::uintmax_t max_level = 0;
    };

#if OPTION_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconversion"
    #pragma clang diagnostic ignored "-Wsign-conversion"
#elif OPTION_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wconversion"
    #pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

    template<>
    struct internal_option_traits<bool, option_strategy::bool_> {
    private:
        using uint_bool = std::uint_least8_t;
    public:
        static constexpr std::uintmax_t max_level = 254;

        static std::uintmax_t get_level(const bool* const value) noexcept {
            uint_bool u8_value = impl::ptr_bit_cast<uint_bool>(value);
            u8_value -= 2;
            return u8_value < max_level ? u8_value : std::uintmax_t(-1);
        }
        static void set_level(bool* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, uint_bool(level + 2));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::reference> {
    private:
        using unref = std::remove_reference_t<T>;
    public:
        static constexpr std::uintmax_t max_level = 255;

        OPTION_PURE static std::uintmax_t get_level(const unref* const* const value) noexcept {
            const auto uptr = impl::ptr_bit_cast<std::uintptr_t>(value);
            return uptr < max_level ? uptr : std::uintmax_t(-1);
        }
        static void set_level(unref** const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uintptr_t(level));
        }
    };

    template<class T>
    struct internal_option_traits<std::reference_wrapper<T>, option_strategy::reference_wrapper> {
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const std::reference_wrapper<T>* const value) noexcept {
            const auto ptr = impl::ptr_bit_cast<std::uintptr_t>(value);
            return ptr < max_level ? ptr : std::uintmax_t(-1);
        }
        static void set_level(std::reference_wrapper<T>* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uintptr_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::pointer_64> {
    private:
        static constexpr std::uint64_t ptr_offset = 0xF8E1B1825D5D6C67;
    public:
        static constexpr std::uintmax_t max_level = 512;

        static std::uintmax_t get_level(const T* const value) noexcept {
            auto uint = impl::ptr_bit_cast<std::uint64_t>(value);
            uint -= ptr_offset;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, ptr_offset + std::uintptr_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::pointer_32> {
    private:
        static constexpr std::uint32_t ptr_offset = 0xFFFF'FFFF - 31;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            auto uint = impl::ptr_bit_cast<std::uint32_t>(value);
            uint -= (ptr_offset - max_level);
            return uint <= max_level ? max_level - uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, ptr_offset - std::uintptr_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::float64_sNaN> {
    private:
        static constexpr std::uint64_t nan_start = 0b1'11111111111'0110110001111001111101010101101100001000100110001111;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            std::uint64_t uint = impl::ptr_bit_cast<std::uint64_t>(value);
            uint -= nan_start;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, nan_start + std::uint64_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::float64_qNaN> {
    private:
        static constexpr std::uint64_t nan_start = 0b1'11111111111'1011111100100110010000110000101110110011010101010111;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            std::uint64_t uint = impl::ptr_bit_cast<std::uint64_t>(value);
            uint -= nan_start;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, nan_start + std::uint64_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::float32_sNaN> {
    private:
        static constexpr std::uint32_t nan_start = 0b1'11111111'01111110110100110101111u;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            std::uint32_t uint = impl::ptr_bit_cast<std::uint32_t>(value);
            uint -= nan_start;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, nan_start + std::uint32_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::float32_qNaN> {
    private:
        static constexpr std::uint32_t nan_start = 0b1'11111111'10000111110111110110101u;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            std::uint32_t uint = impl::ptr_bit_cast<std::uint32_t>(value);
            uint -= nan_start;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, nan_start + std::uint32_t(level));
        }
    };

    template<class First, class Second>
    struct internal_option_traits<std::pair<First, Second>, option_strategy::pair> {
    private:
        using first_traits = opt::option_traits<First>;
        using second_traits = opt::option_traits<Second>;

        struct selected_first { using traits = first_traits; using type = First; };
        struct selected_second { using traits = second_traits; using type = Second; };

        static constexpr bool first_is_max = first_traits::max_level > second_traits::max_level;

        using selected = if_<first_is_max, selected_first, selected_second>;
        using traits = typename selected::traits;
    public:
        static constexpr std::uintmax_t max_level = traits::max_level;

        static constexpr std::uintmax_t get_level(const std::pair<First, Second>* const value) noexcept {
            if constexpr (first_is_max) {
                return traits::get_level(std::addressof(value->first));
            } else {
                return traits::get_level(std::addressof(value->second));
            }
        }
        static constexpr void set_level(std::pair<First, Second>* const value, const std::uintmax_t level) noexcept {
            if constexpr (first_is_max) {
                traits::set_level(std::addressof(value->first), level);
            } else {
                traits::set_level(std::addressof(value->second), level);
            }
        }
    };
    template<class... Ts>
    struct internal_option_traits<std::tuple<Ts...>, option_strategy::tuple> {
    private:
        using select_traits = select_max_level_traits<Ts...>;
        using type = typename select_traits::type;
        using traits = ::opt::option_traits<type>;

        static constexpr std::size_t tuple_index = select_traits::index;
    public:
        static constexpr std::uintmax_t max_level = select_traits::level;

        static constexpr std::uintmax_t get_level(const std::tuple<Ts...>* const value) noexcept {
            return traits::get_level(std::addressof(std::get<tuple_index>(*value)));
        }
        static constexpr void set_level(std::tuple<Ts...>* const value, const std::uintmax_t level) noexcept {
            traits::set_level(std::addressof(std::get<tuple_index>(*value)), level);
        }
    };

    template<class T, std::size_t N>
    struct internal_option_traits<std::array<T, N>, option_strategy::array> {
    private:
        using traits = opt::option_traits<T>;
    public:
        static constexpr std::uintmax_t max_level = traits::max_level;

        static constexpr std::uintmax_t get_level(const std::array<T, N>* const value) noexcept {
            return traits::get_level(std::addressof((*value)[0]));
        }
        static constexpr void set_level(std::array<T, N>* const value, const std::uintmax_t level) noexcept {
            traits::set_level(std::addressof((*value)[0]), level);
        }
    };

#ifdef OPTION_HAS_PFR
    template<class T>
    struct internal_option_traits<T, option_strategy::reflectable> {
    private:
        using select_traits = unpack_tuple_select_max_level_traits<
            decltype(OPTION_PFR_NAMESPACE structure_tie(std::declval<T&>()))
        >;
        static constexpr std::size_t index = select_traits::index;

        using type = typename select_traits::type;
        using traits = opt::option_traits<type>;
    public:
        static constexpr std::uintmax_t max_level = select_traits::level;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            return traits::get_level(std::addressof(OPTION_PFR_NAMESPACE get<index>(*value)));
        }
        static constexpr void set_level(T* const value, const std::uintmax_t level) noexcept {
            traits::set_level(std::addressof(OPTION_PFR_NAMESPACE get<index>(*value)), level);
        }
    };
#endif
    template<class T>
    struct internal_option_traits<T, option_strategy::polymorphic> {
    private:
        static constexpr std::uintptr_t vtable_ptr_start = std::uintptr_t(-1) - 89151;
    public:
        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const T* const value) noexcept {
            std::uintptr_t vtable_ptr = impl::ptr_bit_cast_least<std::uintptr_t>(value);
            vtable_ptr -= vtable_ptr_start;
            return vtable_ptr < max_level ? vtable_ptr : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy_least(value, std::uintptr_t(vtable_ptr_start + level));
        }
    };

    template<class Elem, class Traits>
    struct internal_option_traits<std::basic_string_view<Elem, Traits>, option_strategy::string_view> {
    private:
        static constexpr std::uintptr_t sentinel_ptr = std::uintptr_t(-1) - 32185;
    public:
        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const std::basic_string_view<Elem, Traits>* const value) noexcept {
            if (value->data() == reinterpret_cast<const Elem*>(sentinel_ptr)) {
                return value->size();
            } else {
                return std::uintmax_t(-1);
            }
        }
        static void set_level(std::basic_string_view<Elem, Traits>* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::construct_at(value, reinterpret_cast<const Elem*>(sentinel_ptr), std::size_t(level));
        }
    };
    template<class Elem>
    struct internal_option_traits<std::unique_ptr<Elem, std::default_delete<Elem>>, option_strategy::unique_ptr> {
    private:
        static constexpr std::uintptr_t sentinel_ptr = std::uintptr_t(-1) - 46508;
    public:
        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const std::unique_ptr<Elem, std::default_delete<Elem>>* const value) noexcept {
            std::uintptr_t uint = reinterpret_cast<std::uintptr_t>(value->get());
            uint -= sentinel_ptr;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(std::unique_ptr<Elem, std::default_delete<Elem>>* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::construct_at(value, reinterpret_cast<Elem*>(sentinel_ptr + level));
        }
    };

    // https://rants.vastheman.com/2021/09/21/msvc/
    // https://devblogs.microsoft.com/oldnewthing/20040209-00/?p=40713
    // 
    // Pointer to member data representations (?)
    // struct {
    //     uint32_t offset;
    // };
    // nullptr: offset = -1
    // 
    // Pointer to member function representations
    // 
    // Single inheritance
    // struct {
    //     uintptr_t ptr;
    // };
    // Multiple inheritance
    // struct {
    //     uintptr_t ptr;
    //     int adj;
    // };
    // Virtual inheritance
    // struct {
    //     uintptr_t ptr;
    //     int adj;
    //     int vindex;
    // };
    // Unknown inheritance
    // struct {
    //     uintptr_t ptr;
    //     int adj;
    //     int vadj;
    //     int vindex;
    // };

    // https://itanium-cxx-abi.github.io/cxx-abi/abi.html#member-pointers
    // 
    // Pointer to member data representations
    // struct {
    //     ptrdiff_t offset;
    // };
    // nullptr: offset = -1
    //
    // Pointer to member function representations
    // struct {
    //     fnptr_t ptr;
    //     ptrdiff_t adj;
    // };
    // nullptr: ptr = 0

    template<class T, class U>
    struct internal_option_traits<T U::*, option_strategy::member_pointer_32> {
    private:
        using type = T U::*;
        static constexpr std::uint32_t offset_start = 0xFFFF'FC17;
    public:
        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const type* const value) noexcept {
            std::uint32_t uint = impl::ptr_bit_cast<std::uint32_t>(value);
            uint -= offset_start;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(type* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uint32_t(level + offset_start));
        }
    };
    template<class T, class U>
    struct internal_option_traits<T U::*, option_strategy::member_pointer_64> {
    private:
        using type = T U::*;
        static constexpr std::uint64_t offset_start = 0xFFFFFFFF'D3B2F9B2;
    public:
        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const type* const value) noexcept {
            std::uint64_t uint = impl::ptr_bit_cast_least<std::uint64_t>(value);
            uint -= offset_start;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(type* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy_least(value, std::uint64_t(level + offset_start));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::sentinel_member> {
    private:
        using member_type = decltype(std::declval<T&>().SENTINEL);
        static_assert(std::is_unsigned_v<member_type>, ".SENTINEL member must be an unsigned integer type");
    public:
        static constexpr std::uintmax_t max_level = std::uintmax_t(~member_type(0));

        static std::uintmax_t get_level(const T* const value) noexcept {
            // return value->SENTINEL != 0 ? value->SENTINEL - 1 : std::uintmax_t(-1);
            return std::uintmax_t(std::uintmax_t(value->SENTINEL) - 1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            value->SENTINEL = member_type(level + 1);
        }
    };
#if OPTION_CAN_REFLECT_ENUM
    template<class T>
    struct internal_option_traits<T, option_strategy::enumeration> {
    private:
        static constexpr std::uintmax_t probe_value = sizeof(T) == 1 ? 256 : 1024;
// #################################################################################################
// MSVC:  If you have error C2131 set the compiler option /constexpr:stepsN to a higher value.
// CLANG: If you have 'error: constexpr variable 'max_enumerator_value' must be initialized
//        by a constant expression', set the compiler option -fconstexpr-steps=N to a higher value.
// GCC:   If you have 'error: `constexpr` loop iteration count exceeds limit of XXX',
//        set the compiler option -fconstexpr-loop-limit=N to a higher value.
// #################################################################################################
        static constexpr std::uintmax_t max_enumerator_value = impl::max_enum_value<T, probe_value>();

        using underlying = std::underlying_type_t<T>;
    public:
        static constexpr std::uintmax_t max_level =
            max_enumerator_value == 0 ? 0 : probe_value - max_enumerator_value;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            underlying uint = impl::ptr_bit_cast<underlying>(value);
            uint -= max_enumerator_value;
            return uint < max_level ? uint : std::uintmax_t(-1);
        }
        static constexpr void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, underlying(level + max_enumerator_value));
        }
    };
#endif
    template<class T>
    struct internal_option_traits<T, option_strategy::enumeration_sentinel> {
    private:
        static constexpr T sentinel_value = T::SENTINEL;
    public:
        static constexpr std::uintmax_t max_level = 1;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            return *value == sentinel_value ? 0 : std::uintmax_t(-1);
        }
        static constexpr void set_level(T* const value, [[maybe_unused]] const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level == 0, "Level is out of range");
            *value = sentinel_value;
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::enumeration_sentinel_start> {
    private:
        using underlying = std::underlying_type_t<T>;
        static constexpr underlying sentinel_range_start = static_cast<underlying>(T::SENTINEL_START);
    public:
        static constexpr std::uintmax_t max_level
            = (std::numeric_limits<underlying>::max)() - std::uintmax_t(sentinel_range_start) + 1;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            underlying val = impl::ptr_bit_cast<underlying>(value);
            val -= sentinel_range_start;
            return std::uintmax_t(val) < max_level ? std::uintmax_t(val) : std::uintmax_t(-1);
        }
        static constexpr void set_level(T* const value, [[maybe_unused]] const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, underlying(sentinel_range_start + level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::enumeration_sentinel_start_end> {
    private:
        using underlying = std::underlying_type_t<T>;
        static constexpr underlying sentinel_range_start = static_cast<underlying>(T::SENTINEL_START);
        static constexpr underlying sentinel_range_end = static_cast<underlying>(T::SENTINEL_END);
    public:
        static constexpr std::uintmax_t max_level = sentinel_range_end - sentinel_range_start + 1;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            underlying val = impl::ptr_bit_cast<underlying>(value);
            val -= sentinel_range_start;
            return std::uintmax_t(val) < max_level ? std::uintmax_t(val) : std::uintmax_t(-1);
        }
        static constexpr void set_level(T* const value, [[maybe_unused]] const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, underlying(sentinel_range_start + level));
        }
    };

    template<class T>
    struct internal_option_traits<T, option_strategy::tuple_like> {
    private:
        using select_traits = unpack_tuple_like_select_max_level_traits<T>;
        using type = typename select_traits::type;
        using traits = opt::option_traits<type>;

        static constexpr std::size_t index = select_traits::index;
    public:
        static constexpr std::uintmax_t max_level = select_traits::level;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            return traits::get_level(std::addressof(tuple_like_get<index>(*value)));
        }
        static constexpr void set_level(T* const value, const std::uintmax_t level) noexcept {
            traits::set_level(std::addressof(tuple_like_get<index>(*value)), level);
        }
    };

#if !OPTION_UNKNOWN_STD
    template<class Elem, class Traits, class Allocator>
    struct internal_option_traits<std::basic_string<Elem, Traits, Allocator>, option_strategy::string> {
    private:
        static constexpr bool is_x64 = (sizeof(std::size_t) == 8);
#if OPTION_STL
    #if _ITERATOR_DEBUG_LEVEL == 0
        static constexpr std::size_t size_offset = (is_x64 ? 16 : 16);
        static constexpr std::size_t capacity_offset = (is_x64 ? 24 : 20);
        static_assert(sizeof(std::basic_string<Elem, Traits, Allocator>) == (is_x64 ? 32 : 24));
    #else
        static constexpr std::size_t size_offset = (is_x64 ? 24 : 20);
        static constexpr std::size_t capacity_offset = (is_x64 ? 32 : 24);
        static_assert(sizeof(std::basic_string<Elem, Traits, Allocator>) == (is_x64 ? 40 : 28));
    #endif
#elif OPTION_LIBCPP
    #ifdef _LIBCPP_ABI_ALTERNATE_STRING_LAYOUT
        static constexpr std::size_t size_offset = (is_x64 ? 8 : 4);
        static constexpr std::size_t capacity_offset = (is_x64 ? 16 : 8);
        // capacity is 0
        // is_long is true
        #ifdef _LIBCPP_BIG_ENDIAN
        static constexpr std::size_t magic_capacity = 1;
        #else
        static constexpr std::size_t magic_capacity = std::size_t(1) << (sizeof(std::size_t) * CHAR_BIT - 1);
        #endif
    #else
        static constexpr std::size_t size_offset = (is_x64 ? 8 : 4);
        static constexpr std::size_t capacity_offset = 0;
        // capacity is 0
        // is_long is true
        #ifdef _LIBCPP_BIG_ENDIAN
        static constexpr std::size_t magic_capacity = std::size_t(1) << (sizeof(std::size_t) * CHAR_BIT - 1);
        #else
        static constexpr std::size_t magic_capacity = 1;
        #endif
    #endif
        static_assert(sizeof(std::basic_string<Elem, Traits, Allocator>) == (is_x64 ? 24 : 12));
#elif OPTION_LIBSTDCPP
        static constexpr std::size_t data_offset = 0;
        static constexpr std::size_t size_offset = (is_x64 ? 8 : 4);
        static_assert(sizeof(std::basic_string<Elem, Traits, Allocator>) == (is_x64 ? 32 : 24));
#endif
    public:
        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const std::basic_string<Elem, Traits, Allocator>* const value) noexcept {
#if OPTION_STL
            std::size_t size{};
            std::memcpy(&size, reinterpret_cast<const std::uint8_t*>(value) + size_offset, sizeof(std::size_t));
            std::size_t capacity{};
            std::memcpy(&capacity, reinterpret_cast<const std::uint8_t*>(value) + capacity_offset, sizeof(std::size_t));
            return capacity == 0 ? size : std::uintmax_t(-1);
#elif OPTION_LIBCPP
            std::size_t cap_and_is_long{};
            std::memcpy(&cap_and_is_long, reinterpret_cast<const std::uint8_t*>(value) + capacity_offset, sizeof(std::size_t));
            std::size_t size{};
            std::memcpy(&size, reinterpret_cast<const std::uint8_t*>(value) + size_offset, sizeof(std::size_t));
            return cap_and_is_long == magic_capacity ? size : std::uintmax_t(-1);
#elif OPTION_LIBSTDCPP
            const void* data{};
            std::memcpy(&data, reinterpret_cast<const std::uint8_t*>(value) + data_offset, sizeof(void*));
            std::size_t size{};
            std::memcpy(&size, reinterpret_cast<const std::uint8_t*>(value) + size_offset, sizeof(std::size_t));
            return data == nullptr ? size : std::uintmax_t(-1);
#endif
        }
        static void set_level(std::basic_string<Elem, Traits, Allocator>* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
#if OPTION_STL
            const std::size_t size = static_cast<std::size_t>(level);
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + size_offset, &size, sizeof(std::size_t));
            const std::size_t capacity = 0;
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + capacity_offset, &capacity, sizeof(std::size_t));
#elif OPTION_LIBCPP
            const std::size_t cap_and_is_long = magic_capacity;
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + capacity_offset, &cap_and_is_long, sizeof(std::size_t));
            const std::size_t size = static_cast<std::size_t>(level);
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + size_offset, &size, sizeof(std::size_t));
#elif OPTION_LIBSTDCPP
            const void* const data = nullptr;
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + data_offset, &data, sizeof(void*));
            const std::size_t size = static_cast<std::size_t>(level);
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + size_offset, &size, sizeof(std::size_t));
#endif
        }
    };
    template<class T, class Allocator>
    struct internal_option_traits<std::vector<T, Allocator>, option_strategy::vector> {
        static constexpr std::size_t ptr_size = sizeof(std::uintptr_t);
    public:
        static constexpr std::uintmax_t max_level = 255;

        static constexpr std::uintmax_t get_level(const std::vector<T, Allocator>* const value) noexcept {
            std::uintptr_t first{};
            std::memcpy(&first, reinterpret_cast<const std::uint8_t*>(value) + 0, ptr_size);
            std::uintptr_t last{};
            std::memcpy(&last, reinterpret_cast<const std::uint8_t*>(value) + ptr_size, ptr_size);
            return first == 1 ? last : std::uintmax_t(-1);
        }
        static constexpr void set_level(std::vector<T, Allocator>* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");

            const std::uintptr_t first = 1;
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + 0, &first, ptr_size);
            const std::uintptr_t last = static_cast<std::uintptr_t>(level);
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + ptr_size, &last, ptr_size);
        }
    };
#endif

#if OPTION_CLANG
    #pragma clang diagnostic pop
#elif OPTION_GCC
    #pragma GCC diagnostic pop
#endif
}

template<class T, class>
struct option_traits : impl::internal_option_traits<T> {};

namespace impl {


    template<class T>
    using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

    // See https://github.com/microsoft/STL/pull/878#issuecomment-639696118
    struct nontrivial_dummy {
        constexpr nontrivial_dummy() noexcept {}
    };

    struct construct_from_invoke_tag {
        explicit construct_from_invoke_tag() = default;
    };

    template<class T>
    struct is_tuple_like_impl : std::false_type {};
    template<class T, std::size_t N>
    struct is_tuple_like_impl<std::array<T, N>> : std::true_type {};
    template<class T1, class T2>
    struct is_tuple_like_impl<std::pair<T1, T2>> : std::true_type {};
    template<class... Ts>
    struct is_tuple_like_impl<std::tuple<Ts...>> : std::true_type {};

    template<class T>
    inline constexpr bool is_tuple_like = is_tuple_like_impl<T>::value;

    template<class Tuple>
    struct tuple_like_of_options_t;

    template<class... Ts>
    struct tuple_like_of_options_t<std::tuple<Ts...>> {
        using type = std::tuple<opt::option<Ts>...>;
    };
    template<class T1, class T2>
    struct tuple_like_of_options_t<std::pair<T1, T2>> {
        using type = std::pair<opt::option<T1>, opt::option<T2>>;
    };
    template<class T, std::size_t N>
    struct tuple_like_of_options_t<std::array<T, N>> {
        using type = std::array<opt::option<T>, N>;
    };
    template<class Tuple>
    using tuple_like_of_options = typename tuple_like_of_options_t<Tuple>::type;

    template<class, class T, class... Args>
    struct is_direct_list_initializable_impl {
        static constexpr bool value = false;
    };
    template<class T, class... Args>
    struct is_direct_list_initializable_impl<std::void_t<decltype(T{std::declval<Args>()...})>, T, Args...> {
        static constexpr bool value = true;
    };
    template<class T, class... Args>
    struct is_direct_list_initializable {
        static constexpr bool value = is_direct_list_initializable_impl<void, T, Args...>::value;
    };

    template<class T, class... Args>
    inline constexpr bool is_direct_list_initializable_v = is_direct_list_initializable<T, Args...>::value;

    template<class T>
    struct member_type {
        static_assert(!sizeof(T), "Expected pointer to data member");
    };
    template<class C, class T>
    struct member_type<T C::*> { using type = T; };

    template<class T, class U>
    struct copy_reference {
        using type = std::remove_reference_t<T>&&;
    };
    template<class T, class U>
    struct copy_reference<T&, U&&> {
        using type = T&&;
    };
    template<class T, class U>
    struct copy_reference<T, U&> {
        using type = T&;
    };
    template<class T, class U>
    using copy_reference_t = typename copy_reference<T, U>::type;

    enum class base_strategy {
        has_traits             = 1,
        trivially_destructible = 2
    };

    constexpr base_strategy operator|(const base_strategy left, const base_strategy right) {
        using type = std::underlying_type_t<base_strategy>;
        return base_strategy(type(left) | type(right));
    }

    template<class T>
    constexpr base_strategy detemine_base_strategy() noexcept {
        using st = base_strategy;

        constexpr bool trivially_destructible = std::is_trivially_destructible_v<T>;

        if constexpr (has_option_traits<T>) {
            if constexpr (trivially_destructible) {
                return st::has_traits | st::trivially_destructible;
            } else {
                return st::has_traits;
            }
        } else
        if constexpr (trivially_destructible) {
            return st::trivially_destructible;
        } else {
            return st{};
        }
    }

    template<class T,
        base_strategy strategy = detemine_base_strategy<T>()
    >
    struct option_destruct_base;

    template<class T>
    struct option_destruct_base<T,
        base_strategy::trivially_destructible
    > {
        union {
            nontrivial_dummy dummy;
            T value;
        };
        bool has_value_flag;

        constexpr option_destruct_base() noexcept
            : dummy{}, has_value_flag(false) {}

        template<class... Args>
        constexpr option_destruct_base(const std::in_place_t, Args&&... args)
            : value{std::forward<Args>(args)...}, has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))}, has_value_flag(true) {}

        constexpr void reset() noexcept {
            has_value_flag = false;
        }
        OPTION_PURE constexpr bool has_value() const noexcept {
            return has_value_flag;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value_flag, "Value is non-empty");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            has_value_flag = true;
        }
        template<class U>
        constexpr void assign(U&& other) {
            value = std::forward<U>(other);
        }
    };
    template<class T>
    struct option_destruct_base<T,
        base_strategy{}
    > {
        union {
            nontrivial_dummy dummy;
            T value;
        };
        bool has_value_flag;

        constexpr option_destruct_base() noexcept
            : dummy(), has_value_flag(false) {}

        template<class... Args>
        constexpr option_destruct_base(const std::in_place_t, Args&&... args)
            : value{std::forward<Args>(args)...}, has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))}, has_value_flag(true) {}

        OPTION_CONSTEXPR_CXX20 ~option_destruct_base() noexcept(std::is_nothrow_destructible_v<T>) {
            if (has_value_flag) {
                impl::destroy_at(std::addressof(value));
            }
        }

        constexpr void reset() {
            if (has_value_flag) {
                impl::destroy_at(std::addressof(value));
                has_value_flag = false;
            }
        }
        OPTION_PURE constexpr bool has_value() const noexcept {
            return has_value_flag;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value_flag, "Value is non-empty");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            has_value_flag = true;
        }
        template<class U>
        constexpr void assign(U&& other) {
            value = std::forward<U>(other);
        }
    };
#if OPTION_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    template<class T>
    struct option_destruct_base<T,
        base_strategy::has_traits | base_strategy::trivially_destructible
    > {
        union {
            nontrivial_dummy dummy;
            T value;
        };
        using traits = opt::option_traits<T>;

        static_assert(impl::has_get_level_method<T, traits>, "The static method 'get_level' in 'opt::option_traits' does not exist or has an invalid function signature");
        static_assert(impl::has_set_level_method<T, traits>, "The static method 'set_level' in 'opt::option_traits' does not exist or has an invalid function signature");
        
        constexpr option_destruct_base() noexcept
            : dummy{} {
            traits::set_level(std::addressof(value), 0);
            OPTION_VERIFY(!has_value(), "After the default construction, the value is in an empty state.");
        }
        template<class... Args>
        constexpr option_destruct_base(const std::in_place_t, Args&&... args)
            : value{std::forward<Args>(args)...} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }

        constexpr void reset() noexcept {
            traits::set_level(std::addressof(value), 0);
            OPTION_VERIFY(!has_value(), "After resetting, the value is in an empty state.");
        }
        OPTION_PURE constexpr bool has_value() const noexcept {
            const std::uintmax_t level = traits::get_level(std::addressof(value));
            OPTION_VERIFY(level == std::uintmax_t(-1) || level < traits::max_level, "Invalid level");
            return level == std::uintmax_t(-1);
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value(), "Value is non-empty");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class U>
        constexpr void assign(U&& other) {
            value = std::forward<U>(other);
            OPTION_VERIFY(has_value(), "After assignment, the value is in an empty state");
        }
    };
    template<class T>
    struct option_destruct_base<T,
        base_strategy::has_traits
    > {
        union {
            nontrivial_dummy dummy;
            T value;
        };
        using traits = opt::option_traits<T>;

        static_assert(impl::has_get_level_method<T, traits>, "The static method 'get_level' in 'opt::option_traits' does not exist or has an invalid function signature");
        static_assert(impl::has_set_level_method<T, traits>, "The static method 'set_level' in 'opt::option_traits' does not exist or has an invalid function signature");
        
        constexpr option_destruct_base() noexcept
            : dummy{} {
            traits::set_level(std::addressof(value), 0);
            OPTION_VERIFY(!has_value(), "After the default construction, the value is in an empty state.");
        }
        template<class... Args>
        constexpr option_destruct_base(const std::in_place_t, Args&&... args)
            : value{std::forward<Args>(args)...} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        OPTION_CONSTEXPR_CXX20 ~option_destruct_base() noexcept(std::is_nothrow_destructible_v<T>) {
            if (has_value()) {
                impl::destroy_at(std::addressof(value));
            }
        }

        constexpr void reset() noexcept {
            if (has_value()) {
                impl::destroy_at(std::addressof(value));
                traits::set_level(std::addressof(value), 0);
                OPTION_VERIFY(!has_value(), "After resetting, the value is in an empty state.");
            }
        }
        OPTION_PURE constexpr bool has_value() const noexcept {
            const std::uintmax_t level = traits::get_level(std::addressof(value));
            OPTION_VERIFY(level == std::uintmax_t(-1) || level < traits::max_level, "Invalid level");
            return level == std::uintmax_t(-1);
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value(), "Value is non-empty");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class U>
        constexpr void assign(U&& other) {
            value = std::forward<U>(other);
            OPTION_VERIFY(has_value(), "After assignment, the value is in an empty state");
        }
    };
#if OPTION_GCC
    #pragma GCC diagnostic pop
#endif

    template<class T, bool is_reference /*false*/ = std::is_reference_v<T>>
    class option_storage_base : public option_destruct_base<std::remove_const_t<T>> {
        using base = option_destruct_base<std::remove_const_t<T>>;
        using traits = opt::option_traits<T>;
    public:
        using base::base;
        using base::has_value;
        using base::reset;
        using base::construct;
        using base::assign;

        OPTION_PURE constexpr T& get() & noexcept { return base::value; }
        OPTION_PURE constexpr const T& get() const& noexcept { return base::value; }
        OPTION_PURE constexpr T&& get() && noexcept { return std::move(base::value); }
        OPTION_PURE constexpr const T&& get() const&& noexcept { return std::move(base::value); }

        // logic of assigning opt::option<T&> from a value
        template<class U>
        constexpr void assign_from_value(U&& other) {
            if (has_value()) {
                assign(std::forward<U>(other));
            } else {
                construct(std::forward<U>(other));
            }
        }
        // logic of assigning opt::option<T&> from another opt::option<U&>
        template<class Option>
        constexpr void assign_from_option(Option&& other) {
            if (other.has_value()) {
                if (has_value()) {
                    assign(std::forward<Option>(other).get());
                } else {
                    construct(std::forward<Option>(other).get());
                }
            } else {
                reset();
            }
        }
        // Precondition: has_value() == false
        template<class Option>
        constexpr void construct_from_option(Option&& other) {
            if (other.has_value()) {
                construct(std::forward<Option>(other).get());
            }
        }
    };

    // "specialization" of opt::option<T&>
    // std::option<T&> is currently ill-formed, but opt::option<T&> allows it
    template<class T>
    class option_storage_base<T, /*is_reference=*/true> {
        using raw_type = std::remove_reference_t<T>;

        template<class U>
        static constexpr bool can_bind_reference() {
            using raw_u = std::remove_reference_t<U>;
            if constexpr (std::is_lvalue_reference_v<T>) {
                return std::is_lvalue_reference_v<U> && std::is_convertible_v<raw_u*, raw_type*>;
            } else { // std::is_rvalue_reference_v<T>
                return !std::is_lvalue_reference_v<U> && std::is_convertible_v<raw_u*, raw_type*>;
            }
        }
        template<class U>
        static constexpr raw_type* ref_to_ptr(U&& other) noexcept {
            using raw_u = std::remove_reference_t<U>;
            if constexpr (std::is_same_v<raw_u, std::reference_wrapper<std::remove_const_t<raw_type>>>
                       || std::is_same_v<raw_u, std::reference_wrapper<raw_type>>) {
                return std::addressof(other.get());
            } else {
                static_assert(can_bind_reference<U>(),
                    "Cannot construct a reference element from a possible temporary object");
                return std::addressof(other);
            }
        }
    public:
        raw_type* value;

        constexpr option_storage_base() noexcept
            : value{nullptr} {}

        template<class Arg>
        constexpr option_storage_base(const std::in_place_t, Arg&& arg) noexcept
            : value{ref_to_ptr(std::forward<Arg>(arg))} {}

        template<class F, class Arg>
        constexpr option_storage_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{ref_to_ptr(std::invoke(std::forward<F>(f), std::forward<Arg>(arg)))} {}

        OPTION_PURE constexpr bool has_value() const noexcept {
            return value != nullptr;
        }
        constexpr void reset() noexcept {
            value = nullptr;
        }

        OPTION_PURE constexpr T& get() const& noexcept { return *value; }
        OPTION_PURE constexpr T&& get() const&& noexcept { return std::move(*value); }

        // Precondition: has_value() == false
        template<class Arg>
        constexpr void construct(Arg&& arg) noexcept {
            value = ref_to_ptr(std::forward<Arg>(arg));
        }
        // Precondition: has_value() == false
        template<class Option>
        constexpr void construct_from_option(Option&& other) {
            if (other.has_value()) {
                construct(std::forward<Option>(other).get());
            }
        }

        template<class U>
        constexpr void assign_from_value(U&& other) {
            // always assign as reference
            construct(std::forward<U>(other));
        }

        template<class Option>
        constexpr void assign_from_option(Option&& other) {
            using option_value_type = typename impl::remove_cvref<Option>::value_type;

            if (other.has_value()) {
                if constexpr (std::is_reference_v<option_value_type>) {
                    construct(other.get());
                } else {
                    construct(std::forward<Option>(other).get());
                }
            } else {
                reset();
            }
        }
    };

    template<class T,
        bool /*true*/ = std::is_trivially_copy_constructible_v<T>,
        bool = std::is_copy_constructible_v<T>
    >
    struct option_copy_base : option_storage_base<T> {
        using option_storage_base<T>::option_storage_base;
    };
    template<class T>
    struct option_copy_base<T, false, true> : option_storage_base<T> {
        using option_storage_base<T>::option_storage_base;
        using value_type = T;

        option_copy_base() = default;
        option_copy_base(option_copy_base&&) = default;
        option_copy_base& operator=(const option_copy_base&) = default;
        option_copy_base& operator=(option_copy_base&&) = default;

        constexpr option_copy_base(const option_copy_base& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            this->construct_from_option(other);
        }
    };
    template<class T, bool ignore>
    struct option_copy_base<T, ignore, false> : option_storage_base<T> {
        using option_storage_base<T>::option_storage_base;

        option_copy_base() = default;
        option_copy_base(option_copy_base&&) = default;
        option_copy_base& operator=(const option_copy_base&) = default;
        option_copy_base& operator=(option_copy_base&&) = default;

        option_copy_base(const option_copy_base&) = delete;
    };

    template<class T,
        bool /*true*/ = std::is_trivially_move_constructible_v<T>,
        bool = std::is_move_constructible_v<T>
    >
    struct option_move_base : option_copy_base<T> {
        using option_copy_base<T>::option_copy_base;
    };
    template<class T>
    struct option_move_base<T, false, true> : option_copy_base<T> {
        using option_copy_base<T>::option_copy_base;
        using value_type = T;

        option_move_base() = default;
        option_move_base(const option_move_base&) = default;
        option_move_base& operator=(const option_move_base&) = default;
        option_move_base& operator=(option_move_base&&) = default;

        constexpr option_move_base(option_move_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
            this->construct_from_option(std::move(other));
        }
    };
    template<class T, bool ignore>
    struct option_move_base<T, ignore, false> : option_copy_base<T> {
        using option_copy_base<T>::option_copy_base;

        option_move_base() = default;
        option_move_base(const option_move_base&) = default;
        option_move_base& operator=(const option_move_base&) = default;
        option_move_base& operator=(option_move_base&&) = default;

        option_move_base(option_move_base&&) = delete;
    };

    template<class T,
        bool /*true*/ = or_<
            std::is_reference<T>,
            and_<
                std::is_trivially_copy_assignable<T>,
                std::is_trivially_copy_constructible<T>,
                std::is_trivially_destructible<T>
            >
        >::value,
        bool = or_<std::is_reference<T>, and_<std::is_copy_constructible<T>, std::is_copy_assignable<T>>>::value
    >
    struct option_copy_assign_base : option_move_base<T> {
        using option_move_base<T>::option_move_base;
    };
    template<class T>
    struct option_copy_assign_base<T, false, true> : option_move_base<T> {
        using option_move_base<T>::option_move_base;
        using value_type = T;

        option_copy_assign_base() = default;
        option_copy_assign_base(const option_copy_assign_base&) = default;
        option_copy_assign_base(option_copy_assign_base&&) = default;
        option_copy_assign_base& operator=(option_copy_assign_base&&) = default;

        constexpr option_copy_assign_base& operator=(const option_copy_assign_base& other)
            noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
            this->assign_from_option(other);
            return *this;
        }
    };
    template<class T, bool ignore>
    struct option_copy_assign_base<T, ignore, false> : option_move_base<T> {
        using option_move_base<T>::option_move_base;

        option_copy_assign_base() = default;
        option_copy_assign_base(const option_copy_assign_base&) = default;
        option_copy_assign_base(option_copy_assign_base&&) = default;
        option_copy_assign_base& operator=(option_copy_assign_base&&) = default;

        option_copy_assign_base& operator=(const option_copy_assign_base&) = delete;
    };

    template<class T,
        bool /*true*/ = or_<
            std::is_reference<T>,
            and_<
                std::is_trivially_move_assignable<T>,
                std::is_trivially_move_constructible<T>,
                std::is_trivially_destructible<T>
            >
        >::value,
        bool = or_<std::is_reference<T>, and_<std::is_move_constructible<T>, std::is_move_assignable<T>>>::value
    >
    struct option_move_assign_base : option_copy_assign_base<T> {
        using option_copy_assign_base<T>::option_copy_assign_base;
    };
    template<class T>
    struct option_move_assign_base<T, false, true> : option_copy_assign_base<T> {
        using option_copy_assign_base<T>::option_copy_assign_base;
        using value_type = T;

        option_move_assign_base() = default;
        option_move_assign_base(const option_move_assign_base&) = default;
        option_move_assign_base(option_move_assign_base&&) = default;
        option_move_assign_base& operator=(const option_move_assign_base&) = default;

        constexpr option_move_assign_base& operator=(option_move_assign_base&& other)
            noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
            this->assign_from_option(std::move(other));
            return *this;
        }
    };
    template<class T, bool ignore>
    struct option_move_assign_base<T, ignore, false> : option_copy_assign_base<T> {
        using option_copy_assign_base<T>::option_copy_assign_base;

        option_move_assign_base() = default;
        option_move_assign_base(const option_move_assign_base&) = default;
        option_move_assign_base(option_move_assign_base&&) = default;
        option_move_assign_base& operator=(const option_move_assign_base&) = default;

        option_move_assign_base& operator=(option_move_assign_base&&) = delete;
    };

    template<class T>
    class option_iterator {
    public:
#if OPTION_IS_CXX20
        using iterator_concept = std::contiguous_iterator_tag;
#endif
        using iterator_category = std::random_access_iterator_tag;
        using value_type = std::remove_cv_t<T>;
        using reference = T&;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
    private:
        pointer ptr{};
    public:
        option_iterator() = default;
        explicit constexpr option_iterator(const pointer ptr_) noexcept : ptr{ptr_} {}

        [[nodiscard]] constexpr operator option_iterator<const T>() const noexcept {
            return option_iterator<const T>{ptr};
        }

        [[nodiscard]] constexpr reference operator*() const noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot dereference value-initialized option iterator");
            return *ptr;
        }
        [[nodiscard]] constexpr pointer operator->() const noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot dereference value-initialized option iterator");
            return ptr;
        }
        constexpr option_iterator& operator++() noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot increment value-initialized option iterator");
            ++ptr;
            return *this;
        }
        constexpr option_iterator operator++(int) noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot increment value-initialized option iterator");
            auto tmp{*this};
            ++ptr;
            return tmp;
        }
        constexpr option_iterator& operator--() noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot decrement value-initialized option iterator");
            --ptr;
            return *this;
        }
        constexpr option_iterator operator--(int) noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot decrement value-initialized option iterator");
            auto tmp{*this};
            --ptr;
            return tmp;
        }
        constexpr option_iterator& operator+=(const difference_type offset) noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot seek value-initialized option iterator");
            ptr += offset;
            return *this;
        }
        [[nodiscard]] constexpr option_iterator operator+(const difference_type offset) const noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot seek value-initialized option iterator");
            return option_iterator{ptr + offset};
        }
        [[nodiscard]] friend constexpr option_iterator operator+(const difference_type offset, const option_iterator& it) noexcept {
            OPTION_VERIFY(it.ptr != nullptr, "Cannot seek value-initialized option iterator");
            return option_iterator{it.ptr + offset};
        }
        constexpr option_iterator& operator-=(const difference_type offset) noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot seek value-initialized option iterator");
            ptr -= offset;
            return *this;
        }
        [[nodiscard]] constexpr option_iterator operator-(const difference_type offset) const noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot seek value-initialized option iterator");
            return option_iterator{ptr - offset};
        }
        [[nodiscard]] constexpr difference_type operator-(const option_iterator& right) const noexcept {
            return static_cast<difference_type>(ptr - right.ptr);
        }
        [[nodiscard]] constexpr reference operator[](const difference_type offset) const noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot dereference value-initialized option iterator");
            return *(ptr + offset);
        }
        [[nodiscard]] constexpr bool operator==(const option_iterator& right) const noexcept {
            return ptr == right.ptr;
        }
        [[nodiscard]] constexpr bool operator!=(const option_iterator& right) const noexcept {
            return ptr != right.ptr;
        }
        [[nodiscard]] constexpr bool operator<(const option_iterator& right) const noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot compare value-initialized option iterator");
            OPTION_VERIFY(right.ptr != nullptr, "Cannot compare value-initialized option iterator");
            return ptr < right.ptr;
        }
        [[nodiscard]] constexpr bool operator>(const option_iterator& right) const noexcept {
            OPTION_VERIFY(ptr != nullptr, "Cannot compare value-initialized option iterator");
            OPTION_VERIFY(right.ptr != nullptr, "Cannot compare value-initialized option iterator");
            return ptr > right.ptr;
        }
        [[nodiscard]] constexpr bool operator<=(const option_iterator& right) const noexcept {
            return ptr <= right.ptr;
        }
        [[nodiscard]] constexpr bool operator>=(const option_iterator& right) const noexcept {
            return ptr >= right.ptr;
        }
    };
}

class bad_access : public std::exception {
public:
    bad_access() noexcept = default;
    bad_access(const bad_access&) noexcept = default;

    const char* what() const noexcept override {
        return "Bad opt::option access";
    }
};

namespace impl {
    [[noreturn]] inline void throw_bad_access() {
#if defined(__cpp_exceptions) && __cpp_exceptions >= 199711L
        throw bad_access{};
#else
        OPTION_VERIFY(false, "opt::bad_access was thrown but exceptions are disabled");
#endif
    }
}

namespace impl::option {
    template<class, class>
    struct is_not_same { static constexpr bool value = true; };
    template<class T>
    struct is_not_same<T, T> { static constexpr bool value = false; };

    template<class T1, class T2>
    using exclusive_disjunction = std::bool_constant<bool(T1::value) != bool(T2::value)>;

    template<class T, class U, class is_explicit>
    using enable_constructor_5 = std::enable_if_t<
        and_<
            std::is_constructible<T, U&&>,
            is_not_same<impl::remove_cvref<U>, opt::option<T>>,
            not_<and_<
                std::is_same<impl::remove_cvref<T>, bool>,
                opt::is_option<impl::remove_cvref<U>>
            >>,
            exclusive_disjunction<is_explicit, std::is_convertible<U&&, T>>
        >::value
    , int>;

    template<class T, class First, class... Args>
    using enable_constructor_6 = std::enable_if_t<
        and_<
            is_direct_list_initializable<T, First, Args...>,
            is_not_same<remove_cvref<First>, opt::option<T>>
        >::value
    , int>;

    template<class T, class U>
    using enable_assigment_operator_4 = std::enable_if_t<
        and_<
            is_not_same<remove_cvref<U>, opt::option<T>>,
            std::is_constructible<T, U>,
            std::is_assignable<T&, U>,
            not_<and_<std::is_scalar<T>, std::is_same<T, std::decay_t<U>>>>
        >::value
    , int>;

    template<class T, class U, class UOpt = opt::option<U>>
    using is_constructible_from_option =
        or_<
            std::is_constructible<T, UOpt&>,
            std::is_constructible<T, const UOpt&>,
            std::is_constructible<T, UOpt&&>,
            std::is_constructible<T, const UOpt&&>,
            std::is_convertible<UOpt&, T>,
            std::is_convertible<const UOpt&, T>,
            std::is_convertible<UOpt&&, T>,
            std::is_convertible<const UOpt&&, T>
        >;

    template<class T, class U, class UOpt = opt::option<U>>
    using is_assignable_from_option =
        or_<
            std::is_assignable<T&, UOpt&>,
            std::is_assignable<T&, const UOpt&>,
            std::is_assignable<T&, UOpt&&>,
            std::is_assignable<T&, const UOpt&&>
        >;

    template<class T, class U, class is_explicit>
    using enable_constructor_8 = std::enable_if_t<
        and_<
            std::is_convertible<T, const U&>,
            not_<or_<
                std::is_same<std::remove_cv_t<T>, bool>,
                is_constructible_from_option<T, U>
            >>,
            exclusive_disjunction<is_explicit, std::is_convertible<const U&, T>>
        >::value
    , int>;

    template<class T, class U, class is_explicit>
    using enable_constructor_9 = std::enable_if_t<
        and_<
            std::is_convertible<T, U&&>,
            not_<or_<
                std::is_same<std::remove_const_t<T>, bool>,
                is_constructible_from_option<T, U>
            >>,
            exclusive_disjunction<is_explicit, std::is_convertible<U&&, T>>
        >::value
    , int>;

    template<class T, class U>
    using enable_assigment_operator_5 = std::enable_if_t<
        and_<
            not_<or_<
                is_constructible_from_option<T, U>,
                is_assignable_from_option<T, U>
            >>,
            or_<std::is_reference<T>, and_<
                std::is_constructible<T, const U&>,
                std::is_assignable<T&, const U&>
            >>
        >::value
    , int>;

    template<class T, class U>
    using enable_assigment_operator_6 = std::enable_if_t<
        and_<
            not_<or_<
                is_constructible_from_option<T, U>,
                is_assignable_from_option<T, U>
            >>,
            or_<std::is_reference<T>, and_<
                std::is_constructible<T, U&&>,
                std::is_assignable<T&, U&&>
            >>
        >::value
    , int>;

    template<class T>
    inline constexpr bool nothrow_assigment_operator_2 =
        and_<
            std::is_nothrow_copy_constructible<T>,
            std::is_nothrow_copy_assignable<T>,
            std::is_nothrow_destructible<T>
        >::value;
    template<class T>
    inline constexpr bool nothrow_assigment_operator_3 =
        and_<
            std::is_nothrow_move_constructible<T>,
            std::is_nothrow_move_assignable<T>,
            std::is_nothrow_destructible<T>
        >::value;
    template<class T, class U>
    inline constexpr bool nothrow_assigment_operator_4 =
        and_<
            std::is_nothrow_assignable<T&, U&&>,
            std::is_nothrow_constructible<T, U&&>,
            std::is_nothrow_destructible<T>
        >::value;

    // implementation of opt::option<T>::and_then(F&&)
    template<class Self, class F>
    constexpr auto and_then(Self&& self, F&& f) {
        using invoke_res = impl::remove_cvref<std::invoke_result_t<F, decltype(*std::forward<Self>(self))>>;
        static_assert(opt::is_option_v<invoke_res>, "The return type of function F must be a specialization of opt::option");
        if (self.has_value()) {
            return std::invoke(std::forward<F>(f), *std::forward<Self>(self));
        } else {
            return invoke_res{opt::none};
        }
    }

    // implementation of opt::option<T>::map(F&&)
    // map(F&&) -> option<U> : F(T&&) -> U
    template<class T, class Self, class F>
    constexpr auto map(Self&& self, F&& f) {
        using f_result = std::remove_cv_t<std::invoke_result_t<F, decltype(std::forward<Self>(self).get())>>;
        if (self.has_value()) {
            return opt::option<f_result>{construct_from_invoke_tag{}, std::forward<F>(f), *std::forward<Self>(self)};
        }
        return opt::option<f_result>{opt::none};
    }

    // implementation of opt::option<T>::map_or(U&&, F&&)
    template<class T, class Self, class U, class F>
    constexpr impl::remove_cvref<U> map_or(Self&& self, U&& default_value, F&& f) {
        if (self.has_value()) {
            return std::invoke(std::forward<F>(f), std::forward<Self>(self).get());
        }
        return std::forward<U>(default_value);
    }

    template<class T, class Self, class D, class F>
    constexpr auto map_or_else(Self&& self, D&& d, F&& f) {
        using d_result = std::invoke_result_t<D>;
        using f_result = std::invoke_result_t<F, decltype(std::forward<Self>(self).get())>;
        static_assert(std::is_same_v<d_result, f_result>,
            "The type of the invoke result functions D and F must be the same");
        if (self.has_value()) {
            return std::invoke(std::forward<F>(f), std::forward<Self>(self).get());
        }
        return std::invoke(std::forward<D>(d));
    }

    // implementation of opt::option<T>::or_else(F&&)
    template<class T, class Self, class F>
    constexpr opt::option<T> or_else(Self&& self, F&& f) {
        using f_result = std::invoke_result_t<F>;
        static_assert(std::is_same_v<impl::remove_cvref<f_result>, opt::option<T>>,
            "The function F must return an opt::option<T>");
        if (self.has_value()) {
            return std::forward<Self>(self);
        }
        return std::invoke(std::forward<F>(f));
    }

    // implementation of opt::option<T>::value_or_throw()
    template<class Self>
    constexpr auto&& value_or_throw(Self&& self) {
        if (!self.has_value()) { throw_bad_access(); }
        return *std::forward<Self>(self);
    }

    template<class ValueType, class Self>
    constexpr auto flatten(Self&& self) {
        // this is for a nice error message if Self is not an opt::option<opt::option<T>>
        constexpr bool is_option_option = opt::is_option_v<ValueType>;
        if constexpr (is_option_option) {
            if (self.has_value() && self->has_value()) {
                return ValueType{std::forward<Self>(self)->get()};
            }
            return ValueType{opt::none};
        } else {
            static_assert(is_option_option, "To flatten opt::option<T>, T must be opt::option<U>");
        }
    }
    template<class Self, class P>
    constexpr bool has_value_and(Self&& self, P&& predicate) {
        if (self.has_value()) {
            return std::invoke(std::forward<P>(predicate), std::forward<Self>(self).get());
        }
        return false;
    }

    template<class Self, class F>
    constexpr Self&& inspect(Self&& self, F&& f) {
        if (self.has_value()) {
            std::invoke(std::forward<F>(f), self.get());
        }
        return std::forward<Self>(self);
    }

    template<class TupleLikeOfOptions, class Self, std::size_t... Idx>
    constexpr auto unzip_impl(Self&& self, std::index_sequence<Idx...>) {
        if (self.has_value()) {
            return TupleLikeOfOptions{
                opt::option{std::get<Idx>(std::forward<Self>(self).get())}...
            };
        } else {
            return TupleLikeOfOptions{
                opt::option<std::tuple_element_t<Idx, typename remove_cvref<Self>::value_type>>{opt::none}...
            };
        }
    }

    template<class Self>
    constexpr auto unzip(Self&& self) {
        using tuple_like_type = impl::remove_cvref<typename impl::remove_cvref<Self>::value_type>;
        static_assert(is_tuple_like<tuple_like_type>,
            "To unzip opt::option<T>, T must be tuple-like."
            "A type T that satisfies tuple-like must be a specialization of "
            "std::array, std::pair, std::tuple");

        return unzip_impl<impl::tuple_like_of_options<tuple_like_type>>(
            std::forward<Self>(self),
            std::make_index_sequence<std::tuple_size<tuple_like_type>::value>{}
        );
    }

    template<class Self, class F>
    constexpr impl::remove_cvref<Self> filter(Self&& self, F&& f) {
        if (self.has_value() && bool(std::invoke(std::forward<F>(f), self.get()))) {
            return std::forward<Self>(self).get();
        }
        return opt::none;
    }

    template<class T, class U>
    using enable_swap = std::enable_if_t<
        and_<
            std::is_move_constructible<T>,
            std::is_move_constructible<U>,
            std::is_swappable_with<T, U>
        >::value
    >;
    template<class T, class U>
    using nothrow_swap = and_<
        std::is_nothrow_move_constructible<T>,
        std::is_nothrow_move_constructible<U>,
        std::is_nothrow_swappable_with<T, U>
    >;
}

template<class T>
class option : private impl::option_move_assign_base<T>
{
    using base = impl::option_move_assign_base<T>;

    template<class, impl::option_strategy> friend struct impl::internal_option_traits;
    template<class> friend class option;
public:
    static_assert(!std::is_same_v<impl::remove_cvref<T>, opt::none_t>,
        "In opt::option<T>, T cannot be opt::none_t."
        "If you using CTAD (Class template argument deduction),"
        "you should specify the type for an empty opt::option<T>.");
    static_assert(!std::is_same_v<impl::remove_cvref<T>, std::in_place_t>,
        "In opt::option<T>, T cannot be std::in_place_t."
        "If you using CTAD (Class template argument deduction),"
        "you should specify the type.");

    static_assert(!std::is_void_v<T>,
        "T cannot be (possibly cv-qualified) `void`");
    static_assert(std::is_destructible_v<T>,
        "T must be destructible");

    using value_type = T;
    using iterator = impl::option_iterator<T>;
    using const_iterator = impl::option_iterator<const T>;

    // Default constructor.
    // Constructors an object that does not contain a value.
    // Postcondition: has_value() == false
    option() = default;

    // Constructors an object that does not contain a value.
    // Postcondition: has_value() == false
    constexpr option(opt::none_t) noexcept : base() {}

    // Copy constructor.
    // If other `opt::option` (first parameter) contains a value, initializes current contained value
    // with other's contained value. If other does not contain a value, constructs an empty `opt::option`.
    // Deleted if `!std::is_copy_constructible_v<T>`.
    // Trivial if `std::is_trivially_copy_constructible_v<T>`.
    // Postcondition: has_value() == other.has_value()
    option(const option&) = default;

    // Move constructor.
    // If other `opt::option` (first parameter) contains a value, initializes current contained value
    // with other's contained value using `std::move`. If other does not contain a value, constructs an empty `opt::option`.
    // Deleted if `!std::is_move_constructible_v<T>`.
    // Trivial if `std::is_trivially_move_constructible_v<T>`.
    // Postcondition: has_value() == other.has_value()
    option(option&&) = default;

    // Constructs the `opt::option` that contains a value from `val`.
    // Explicit if `!std::is_convertible_v<U&&, T>`.
    // Postcondition: has_value() == true
    template<class U = T, impl::option::enable_constructor_5<T, U, /*is_explicit=*/std::true_type> = 0>
    constexpr explicit option(U&& val) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : base(std::in_place, std::forward<U>(val)) {}
    template<class U = T, impl::option::enable_constructor_5<T, U, /*is_explicit=*/std::false_type> = 0>
    constexpr option(U&& val) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : base(std::in_place, std::forward<U>(val)) {}

    // Constructs the `opt::option` that contains a value,
    // initialized from the arguments `first`, `args...`.
    // Postcondition: has_value() == true
    template<class First, class... Args, impl::option::enable_constructor_6<T, First, Args...> = 0>
    constexpr option(First&& first, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, First, Args...>)
        : base(std::in_place, std::forward<First>(first), std::forward<Args>(args)...) {}

#if !OPTION_GCC
    template<class... Args>
    constexpr explicit option(const std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : base(std::in_place, std::forward<Args>(args)...) {}

    template<class U, class... Args>
    constexpr explicit option(const std::in_place_t, std::initializer_list<U> ilist, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)
        : base(std::in_place, ilist, std::forward<Args>(args)...) {}
#else
    template<class InPlaceT, class... Args, std::enable_if_t<std::is_same_v<InPlaceT, std::in_place_t>, int> = 0>
    constexpr explicit option(const InPlaceT, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : base(std::in_place, std::forward<Args>(args)...) {}

    template<class InPlaceT, class U, class... Args, std::enable_if_t<std::is_same_v<InPlaceT, std::in_place_t>, int> = 0>
    constexpr explicit option(const InPlaceT, std::initializer_list<U> ilist, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)
        : base(std::in_place, ilist, std::forward<Args>(args)...) {}
#endif

    // Constructs the `opt::option` that contains a value,
    // direct-initialized from `f` function result.
    // Postcondition: has_value() == true
    template<class F, class Arg>
    constexpr explicit option(impl::construct_from_invoke_tag, F&& f, Arg&& arg)
        : base(impl::construct_from_invoke_tag{}, std::forward<F>(f), std::forward<Arg>(arg)) {}

    // Converting copy constructor.
    // If `other` contains a value, copy constructs a contained value.
    // If `other` does not contain a value, constructs an empty `opt::option`.
    // Explicit if `!std::is_convertible_v<const U&, T>`.
    // Postcondition: has_value() == other.has_value()
    template<class U, impl::option::enable_constructor_8<T, U, /*is_explicit=*/std::false_type> = 0>
    constexpr option(const option<U>& other) noexcept(std::is_nothrow_constructible_v<T, const U&>) {
        base::construct_from_option(other);
    }
    template<class U, impl::option::enable_constructor_8<T, U, /*is_explicit=*/std::true_type> = 0>
    constexpr explicit option(const option<U>& other) noexcept(std::is_nothrow_constructible_v<T, const U&>) {
        base::construct_from_option(other);
    }
    // Converting move constructor.
    // If `other` containes a value, move constructs a contained value.
    // If `other` does not contain a value, constructs an empty `opt::option`.
    // Explicit if `!std::is_convertible_v<U&&, T>`.
    // Postcondition: has_value() == other.has_value()
    template<class U, impl::option::enable_constructor_9<T, U, /*is_explicit=*/std::false_type> = 0>
    constexpr option(option<U>&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
        base::construct_from_option(std::move(other));
    }
    template<class U, impl::option::enable_constructor_9<T, U, /*is_explicit=*/std::true_type> = 0>
    constexpr explicit option(option<U>&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
        base::construct_from_option(std::move(other));
    }

    // If this `opt::option` containes a value, the contained value is destroyed by calling `reset()`.
    // Postcondition: has_value() == false
    constexpr option& operator=(opt::none_t) noexcept(std::is_nothrow_destructible_v<T>) {
        reset();
        return *this;
    }

    // Copy assigment operator.
    // this | other | action
    //  NO  |  NO   | Do nothing
    //  NO  |  YES  | Copy construct
    //  YES |  NO   | Destroy by calling `reset()`
    //  YES |  YES  | Copy assign
    // Deleted if `!(std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>)`.
    // Trivial if `std::is_trivially_copy_constructible_v<T>
    //          && std::is_trivially_copy_assignable_v<T>
    //          && std::is_trivially_destructible_v<T>`.
    // Postcondition: has_value() == other.has_value()
    option& operator=(const option&) = default;

    // Move assigment operator.
    // If contains a value:
    // this | other | action
    //  NO  |  NO   | Do nothing
    //  NO  |  YES  | Move construct
    //  YES |  NO   | Destroy by calling `reset()`
    //  YES |  YES  | Move assign
    // Deleted if `!(std::is_move_constructible_v<T> || std::is_move_assignable_v<T>)`.
    // Trivial if `std::is_trivially_move_constructible_v<T>
    //          && std::is_trivially_move_assignable_v<T>
    //          && std::is_trivially_destructible_v<T>`
    // Postcondition: has_value() == other.has_value()
    option& operator=(option&&) = default;

    // Assigns the `opt::option` from a `value`.
    // If this `opt::option` contains a value, then assign it from the `value`; otherwise, construct it from the `value`
    // Postcondition: has_value() == true
    template<class U = T, impl::option::enable_assigment_operator_4<T, U> = 0>
    constexpr option& operator=(U&& val) noexcept(impl::option::nothrow_assigment_operator_4<T, U>) {
        base::assign_from_value(std::forward<U>(val));
        return *this;
    }

    // Assigns the `opt::option` from an `other` `opt::option` (first parameter).
    // If contains a value:
    // this | other | action
    //  NO  |  NO   | Do nothing
    //  NO  |  YES  | Construct using `other`
    //  YES |  NO   | Destroy by calling `reset()`
    //  YES |  YES  | Assign using `other`
    // Postcondition: has_value() == other.has_value()
    template<class U, impl::option::enable_assigment_operator_5<T, U> = 0>
    constexpr option& operator=(const option<U>& other) {
        base::assign_from_option(other);
        return *this;
    }
    // Assigns the `opt::option` from an moved `other` `opt::option` (first parameter).
    // If contains a value:
    // this | other | action
    //  NO  |  NO   | Do nothing
    //  NO  |  YES  | Construct using moved `other`
    //  YES |  NO   | Destroy by calling `reset()`
    //  YES |  YES  | Assign using moved `other`
    // Postcondition: has_value() == other.has_value()
    template<class U, impl::option::enable_assigment_operator_6<T, U> = 0>
    constexpr option& operator=(option<U>&& other) {
        base::assign_from_option(std::move(other));
        return *this;
    }

    [[nodiscard]] constexpr iterator begin() noexcept {
        return iterator{has_value() ? std::addressof(base::value) : nullptr};
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept {
        return const_iterator{has_value() ? std::addressof(base::value) : nullptr};
    }
    [[nodiscard]] constexpr iterator end() noexcept {
        return iterator{has_value() ? (std::addressof(base::value) + 1) : nullptr};
    }
    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return const_iterator{has_value() ? (std::addressof(base::value) + 1) : nullptr};
    }

    // Destroys the contained value.
    // If this `opt::option` contains a value, destroy the contained value; otherwise, do nothing
    // Same as `std::optional`
    // Postcondition: has_value() == false
    constexpr void reset() noexcept(std::is_nothrow_destructible_v<T>) {
        base::reset();
    }

    // Constructs the contained value.
    // If this contains a value before the `emplace`, destroy that contained value.
    // Initializes the contained value with `std::forward<Args>(args)...` as constructor parameters
    // Same as `std::optional`
    // Postcondition: has_value() == true
    template<class... Args>
    constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_destructible_v<T>)
        OPTION_LIFETIMEBOUND {
        reset();
        base::construct(std::forward<Args>(args)...);
        return *(*this);
    }

    // Checks whether the `opt::option` contains a value.
    // Returns `true` if this `opt::option` contains a value;
    // otherwise return `false` if this `opt::option` does not contain a value
    // Same as `std::optional`
    [[nodiscard]] OPTION_PURE constexpr bool has_value() const noexcept {
        return base::has_value();
    }
    [[nodiscard]] OPTION_PURE constexpr explicit operator bool() const noexcept {
        return base::has_value();
    }

    // Returns `true` if the `opt::option` contains a value
    // and invocation of the `predicate` with the contained as an argument value evaluates to `true`;
    // otherwise return `false`
    // Same as Rust's `std::option::Option<T>::is_some_and`
    template<class P>
    [[nodiscard]] constexpr bool has_value_and(P&& predicate) & { return impl::option::has_value_and(*this, std::forward<P>(predicate)); }
    template<class P>
    [[nodiscard]] constexpr bool has_value_and(P&& predicate) const& { return impl::option::has_value_and(*this, std::forward<P>(predicate)); }
    template<class P>
    [[nodiscard]] constexpr bool has_value_and(P&& predicate) && { return impl::option::has_value_and(std::move(*this), std::forward<P>(predicate)); }
    template<class P>
    [[nodiscard]] constexpr bool has_value_and(P&& predicate) const&& { return impl::option::has_value_and(std::move(*this), std::forward<P>(predicate)); }

    // Takes the value out of the `opt::option`.
    // If this `opt::option` contains a value return it with move
    // and destroy which is left in `opt::option`;
    // otherwise return an empty `opt::option`
    // Same as Rust's `std::option::Option<T>::take`
    // Postcondition: has_value() == false
    [[nodiscard]] constexpr option<T> take() {
        option<T> tmp{std::move(*this)};
        reset();
        return tmp;
    }

    // Takes the value out of the `opt::option`
    // if invocation of the `predicate` with the contained value as an argument evaluates to `true`.
    // This method similar to conditional `opt::option<T>::take`
    // Same as Rust's `std::option::Option<T>::take_if`
    template<class P>
    [[nodiscard]] constexpr option<T> take_if(P&& predicate) {
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
        if (has_value() && bool(std::invoke(std::forward<P>(predicate), get()))) {
            return take();
        }
        return opt::none;
    }

    // Invokes the `f` with a contained value
    // If this `opt::option` contains a value invoke `f` with the contained value as an argument;
    // otherwise do nothing.
    // Returns `*this`.
    // Does not modify the contained value inside.
    // Same as Rust's `std::option::Option<T>::inspect`
    template<class F>
    constexpr option& inspect(F&& f) & { return impl::option::inspect(*this, std::forward<F>(f)); }
    template<class F>
    constexpr const option& inspect(F&& f) const& { return impl::option::inspect(*this, std::forward<F>(f)); }
    template<class F>
    constexpr option&& inspect(F&& f) && { return impl::option::inspect(std::move(*this), std::forward<F>(f)); }
    template<class F>
    constexpr const option&& inspect(F&& f) const&& { return impl::option::inspect(std::move(*this), std::forward<F>(f)); }

    // Returns a reference to the contained value.
    // This method does not check whether the `opt::option` contains a value
    // on "Release" configuration.
    // Calls the `OPTION_VERIFY` macro if this `opt::option` does not contain the value.
    // Same as `std::optional<T>::operator*`.
    // Precondition: has_value() == true
    [[nodiscard]] OPTION_PURE constexpr T& get() & noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return base::get();
    }
    [[nodiscard]] OPTION_PURE constexpr const T& get() const& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return base::get();
    }
    [[nodiscard]] OPTION_PURE constexpr T&& get() && noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return static_cast<T&&>(base::get());
    }
    [[nodiscard]] OPTION_PURE constexpr const T&& get() const&& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return static_cast<const T&&>(base::get());
    }
    // Returns a pointer to the contained value.
    // Calls the `OPTION_VERIFY` macro if this `opt::option` does not contain the value
    // Returns `std::addressof` of this `opt::option` contained value.
    // Same as `std::optional<T>::operator->`.
    // Precondition: has_value() == true
    [[nodiscard]] OPTION_PURE constexpr std::add_pointer_t<const T> operator->() const noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::addressof(get());
    }
    [[nodiscard]] OPTION_PURE constexpr std::add_pointer_t<T> operator->() noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::addressof(get());
    }
    // Returns a reference to the contained value.
    // Calls the `OPTION_VERIFY` macro if this `opt::option` does not contain the value
    // Same as `std::optional<T>::operator*` or `opt::option<T>::get`.
    // Precondition: has_value() == true
    [[nodiscard]] OPTION_PURE constexpr T& operator*() & noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return get();
    }
    [[nodiscard]] OPTION_PURE constexpr const T& operator*() const& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return get();
    }
    [[nodiscard]] OPTION_PURE constexpr T&& operator*() && noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return static_cast<T&&>(get());
    }
    [[nodiscard]] OPTION_PURE constexpr const T&& operator*() const&& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return static_cast<const T&&>(get());
    }

    // Returns a reference to the contained value.
    // Does not call the `OPTION_VERIFY` macro.
    // No OPTION_LIFETIMEBOUND
    [[nodiscard]] OPTION_PURE constexpr T& get_unchecked() & noexcept { return base::get(); }
    [[nodiscard]] OPTION_PURE constexpr const T& get_unchecked() const& noexcept { return base::get(); }
    [[nodiscard]] OPTION_PURE constexpr T&& get_unchecked() && noexcept { return static_cast<T&&>(base::get()); }
    [[nodiscard]] OPTION_PURE constexpr const T&& get_unchecked() const&& noexcept { return static_cast<const T&&>(base::get()); }

    // Returns a reference to the contained value.
    // Throws a `opt::bad_access` if this `opt::option` does not contain the value.
    // More verbose version of `opt::option<T>::value`.
    [[nodiscard]] constexpr T& value_or_throw() & OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(*this); }
    [[nodiscard]] constexpr const T& value_or_throw() const& OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(*this); }
    [[nodiscard]] constexpr T&& value_or_throw() && OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(std::move(*this)); }
    [[nodiscard]] constexpr const T&& value_or_throw() const&& OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(std::move(*this)); }

    // Returns a reference to the contained value.
    // Throws a `opt::bad_access` if this `opt::option` does not contain the value.
    // Similar to `std::optional<T>::value`.
    [[nodiscard]] constexpr T& value() & OPTION_LIFETIMEBOUND { return value_or_throw(); }
    [[nodiscard]] constexpr const T& value() const& OPTION_LIFETIMEBOUND { return value_or_throw(); }
    [[nodiscard]] constexpr T&& value() && OPTION_LIFETIMEBOUND { return static_cast<T&&>(value_or_throw()); }
    [[nodiscard]] constexpr const T&& value() const&& OPTION_LIFETIMEBOUND { return static_cast<const T&&>(value_or_throw()); }

    // Returns the contained value if this `opt::option` contains the value;
    // otherwise returns a forwarded `default_value`.
    // Same as `std::optional<T>::value_or`
    template<class U>
    [[nodiscard]] constexpr T value_or(U&& default_value) const& {
        if (has_value()) {
            return *(*this);
        }
        return static_cast<T>(std::forward<U>(default_value));
    }
    template<class U>
    [[nodiscard]] constexpr T value_or(U&& default_value) && {
        if (has_value()) {
            return std::move(*(*this));
        }
        return static_cast<T>(std::forward<U>(default_value));
    }

    // Returns the contained value if `opt::option` contains the value;
    // otherwise return a default constructed `T` (expression `T{}`).
    // Similar to Rust's `std::option::Option<T>::value_or_default`
    [[nodiscard]] constexpr T value_or_default() const& {
        if (has_value()) {
            return get();
        }
        return T{};
    }
    [[nodiscard]] constexpr T value_or_default() && {
        if (has_value()) {
            return std::move(get());
        }
        return T{};
    }

    // Returns the provided `def` default value if this `opt::option` does not contain a value;
    // otherwise, return result of a `f` invocation with this `opt::option` contained value as an argument.
    // Use `opt::option<T>::map_or_else` to lazy evaluate the default value
    // Same as Rust's `std::option::Option<T>::map_or`
    template<class U, class F>
    [[nodiscard]] constexpr auto map_or(U&& def, F&& f) & { return impl::option::map_or<T>(*this, std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F>
    [[nodiscard]] constexpr auto map_or(U&& def, F&& f) const& { return impl::option::map_or<T>(*this, std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F>
    [[nodiscard]] constexpr auto map_or(U&& def, F&& f) && { return impl::option::map_or<T>(std::move(*this), std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F>
    [[nodiscard]] constexpr auto map_or(U&& def, F&& f) const&& { return impl::option::map_or<T>(std::move(*this), std::forward<U>(def), std::forward<F>(f)); }

    // Returns a `def` default function result if this `opt::option` does not contain a value;
    // otherwise, returns result of a 'f' invocation with this `opt::option` contained value as an argument
    // Same as Rust's `std::option::Option<T>::map_or_else`
    template<class D, class F>
    [[nodiscard]] constexpr auto map_or_else(D&& def, F&& f) & { return impl::option::map_or_else<T>(*this, std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F>
    [[nodiscard]] constexpr auto map_or_else(D&& def, F&& f) const& { return impl::option::map_or_else<T>(*this, std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F>
    [[nodiscard]] constexpr auto map_or_else(D&& def, F&& f) && { return impl::option::map_or_else<T>(std::move(*this), std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F>
    [[nodiscard]] constexpr auto map_or_else(D&& def, F&& f) const&& { return impl::option::map_or_else<T>(std::move(*this), std::forward<D>(def), std::forward<F>(f)); }

    // Returns a pointer to the contained value if this `opt::option` contains the value;
    // otherwise, return `nullptr`.
    [[nodiscard]] OPTION_PURE constexpr std::remove_reference_t<T>* ptr_or_null() noexcept OPTION_LIFETIMEBOUND {
        return has_value() ? std::addressof(get()) : nullptr;
    }
    [[nodiscard]] OPTION_PURE constexpr const std::remove_reference_t<T>* ptr_or_null() const noexcept OPTION_LIFETIMEBOUND {
        return has_value() ? std::addressof(get()) : nullptr;
    }

    // Returns an empty `opt::option` if this `opt::option` does not contain a value;
    // otherwise, invoke `f` with the contained value as an argument and return:
    //     `opt::option` that containes current contained value if `f` return `true`
    //     an empty `opt::option` if `f` returns `false`
    // Same as Rust's `std::option::Option<T>::filter`
    template<class F>
    [[nodiscard]] constexpr option<T> filter(F&& f) & { return impl::option::filter(*this, std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr option<T> filter(F&& f) const& { return impl::option::filter(*this, std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr option<T> filter(F&& f) && { return impl::option::filter(std::move(*this), std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr option<T> filter(F&& f) const&& { return impl::option::filter(std::move(*this), std::forward<F>(f)); }

    // Converts `opt::option<opt::option<U>>` to `opt::option<U>`.
    // If this `opt::option<T>` then return underlying `opt::option<U>`;
    // otherwise return an empty `opt::option<U>`.
    // Where `U` is `typename T::value_type`.
    // Example: opt::option<opt::option<int>> -> flatten() -> opt::option<int>
    // Same as Rust's `std::option::Option<T>::flatten`
    [[nodiscard]] constexpr auto flatten() const& { return impl::option::flatten<T>(*this); }
    [[nodiscard]] constexpr auto flatten() && { return impl::option::flatten<T>(std::move(*this)); }

    // Returns an empty `opt::option` if this `opt::option` does not contain a value;
    // otherwise invokes `f` with the contained value as an argument and returns the result.
    // This operation is also sometimes called flatmap.
    // Same as `std::optional<T>::and_then` or Rust's `std::option::Option<T>::and_then`
    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) & { return impl::option::and_then(*this, std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) const& { return impl::option::and_then(*this, std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) && { return impl::option::and_then(std::move(*this), std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) const&& { return impl::option::and_then(std::move(*this), std::forward<F>(f)); }

    // Maps an `opt::option<T>` to `opt::option<U>` by applying a function to a contained value, otherwise returns `opt::none`
    // If this `opt::option` contains a value, invokes `f` with the contained value as an argument,
    // then returns an `opt::option` that contains the result of invocation; otherwise, return an empty `opt::option`
    // Same as `std::optional<T>::transform` or Rust's `std::option::Option<T>::map`
    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) & { return impl::option::map<T>(*this, std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) const& { return impl::option::map<T>(*this, std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) && { return impl::option::map<T>(std::move(*this), std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) const&& { return impl::option::map<T>(std::move(*this), std::forward<F>(f)); }

    // Returns a contained value if this `opt::option` contains a value;
    // otherwise, return the result of `f`.
    // Same as `std::optional<T>::or_else`
    template<class F>
    [[nodiscard]] constexpr option<T> or_else(F&& f) const& { return impl::option::or_else<T>(*this, std::forward<F>(f)); }
    template<class F>
    [[nodiscard]] constexpr option<T> or_else(F&& f) && { return impl::option::or_else<T>(std::move(*this), std::forward<F>(f)); }

    // Unzips this `opt::option` containing a tuple like object to the tuple like object of `opt::option`s
    // If this `opt::option` contains a value, return tuple like object that contains
    // `opt::option`s of the current `opt::option` underlying values;
    // otherwise, return tuple like object that contains the empty `opt::option`s.
    // Where tuple like object is a std::array, std::pair or a std::tuple
    // Example: opt::option<std::tuple<int, float>> -> unzip() -> std::tuple<opt::option<int>, opt::option<float>>
    // Similar to Rust's `std::option::Option<T>::unzip` but for any number of values
    [[nodiscard]] constexpr auto unzip() & { return impl::option::unzip(*this); }
    [[nodiscard]] constexpr auto unzip() const& { return impl::option::unzip(*this); }
    [[nodiscard]] constexpr auto unzip() && { return impl::option::unzip(std::move(*this)); }
    [[nodiscard]] constexpr auto unzip() const&& { return impl::option::unzip(std::move(*this)); }

    // Replaces the value in this `opt::option` by a provided `val`
    // and returns the old `opt::option` value
    // Similar to Rust's `std::option::Option<T>::replace`
    template<class U>
    [[nodiscard]] constexpr option<T> replace(U&& val) {
        option<T> tmp{std::move(*this)};
        // should call the destructor after moving, because moving does not end lifetime
        base::reset();
        base::construct(std::forward<U>(val));
        return tmp;
    }

    template<class U>
    constexpr void swap(option<U>& other) noexcept(impl::option::nothrow_swap<T, U>::value) {
        using std::swap;
        if (!has_value() && !other.has_value()) {
            return;
        }
        if (has_value() && other.has_value()) {
            swap(get(), other.get());
            return;
        }
        if (has_value()) {
            other.base::construct(std::move(get()));
            base::reset();
            return;
        }
        base::construct(std::move(other.get()));
        other.base::reset();
    }
};

template<class T>
option(T) -> option<T>;

namespace impl {
    template<class T>
    struct internal_option_traits<opt::option<T>, option_strategy::avaliable_option> {
        using base = typename opt::option<T>::base;
        using traits = opt::option_traits<T>;

        static constexpr std::uintmax_t max_level = traits::max_level - 1;

        static std::uintmax_t get_level(const opt::option<T>* const value) noexcept {
            std::uintmax_t level = traits::get_level(std::addressof(static_cast<const base*>(value)->value));
            level -= 1;
            return level < std::uintmax_t(-2) ? level : std::uintmax_t(-1);
            // if (level == std::uintmax_t(-1) || level == 0) {
            //     return std::uintmax_t(-1);
            // } else {
            //     return level - 1;
            // }
        }

        static void set_level(opt::option<T>* const value, const std::uintmax_t level) noexcept {
            traits::set_level(std::addressof(static_cast<base*>(value)->value), level + 1);
        }
    };
    template<class T>
    struct internal_option_traits<opt::option<T>, option_strategy::unavaliable_option> {
        using base = typename opt::option<T>::base;
        using bool_traits = opt::option_traits<bool>;

        static constexpr std::uintmax_t max_level = bool_traits::max_level;

        static std::uintmax_t get_level(const opt::option<T>* const value) noexcept {
            return bool_traits::get_level(std::addressof(static_cast<const base*>(value)->has_value_flag));
        }
        static void set_level(opt::option<T>* const value, const std::uintmax_t level) noexcept {
            bool_traits::set_level(std::addressof(static_cast<base*>(value)->has_value_flag), level);
        }
    };
}

template<class T>
[[nodiscard]] constexpr option<std::decay_t<T>> make_option(T&& value) {
    return option<std::decay_t<T>>{std::forward<T>(value)};
}
template<class T, class... Args>
[[nodiscard]] constexpr option<T> make_option(Args&&... args) {
    return option<T>{std::in_place, std::forward<Args>(args)...};
}
template<class T, class U, class... Args>
[[nodiscard]] constexpr option<T> make_option(std::initializer_list<U> ilist, Args&&... args) {
    return option<T>{std::in_place, ilist, std::forward<Args>(args)...};
}

// Zips `options...` into `opt::option<std::tuple<VALS...>>`
// where `VALS...` are underlying values of `options...`
// if every `options...` contains a value; otherwise returns an empty `opt::option`
// Example: opt::zip(option1, option2)
// will return `opt::option<std::tuple<option1::value_type, option2::value_type>>`
// if `option1.has_value()` and `option2.has_value()`; otherwise will return an empty `opt::option`
template<class... Options, std::enable_if_t<std::conjunction_v<opt::is_option<impl::remove_cvref<Options>>...>, int> = 0>
[[nodiscard]] constexpr auto zip(Options&&... options)
    -> opt::option<std::tuple<typename impl::remove_cvref<Options>::value_type...>>
{
    if ((options.has_value() && ...)) {
        return opt::option{std::tuple{std::forward<Options>(options).get()...}};
    } else {
        return {};
    }
}

// If all passed `options...` contains a value, invokes `fn`
// with every contained `options...` value as arguments
// and returns `opt::option` with the result of this invocation;
// otherwise returns an empty `opt::option`
// Example: opt::zip_with(func, option1, option2)
// will return invocation of `func` with (`option1.get()`, `option2.get()`) as arguments
// if `option1.has_value()` and `option2.has_value()`; otherwise will return an empty `opt::option`
template<class Fn, class... Options, std::enable_if_t<std::conjunction_v<opt::is_option<impl::remove_cvref<Options>>...>, int> = 0>
[[nodiscard]] constexpr auto zip_with(Fn&& fn, Options&&... options)
{
    using fn_result = std::invoke_result_t<Fn, decltype(std::declval<Options>().get())...>;
    if constexpr (std::is_void_v<fn_result>) {
        if ((options.has_value() && ...)) {
            std::invoke(std::forward<Fn>(fn), std::forward<Options>(options).get()...);
        }
        return void();
    } else {
        if ((options.has_value() && ...)) {
            return opt::option<fn_result>{std::invoke(std::forward<Fn>(fn), std::forward<Options>(options).get()...)};
        } else {
            return opt::option<fn_result>{opt::none};
        }
    }
}

// Constructs 'opt::option<To>' using 'const From&', if passed 'opt::option<From>' has value
// else return 'opt::none'
template<class To, class From>
[[nodiscard]] constexpr opt::option<To> option_cast(const opt::option<From>& value) {
    return value.map([](const From& x) { return To(x); });
}
// Constructs 'opt::option<To>' using 'From&&', if passed 'opt::option<From>' has value
// else return 'opt::none'
template<class To, class From>
[[nodiscard]] constexpr opt::option<To> option_cast(opt::option<From>&& value) {
    return std::move(value).map([](From&& x) { return To(std::move(x)); });
}

// Constructs 'opt::option<T>' from dereferenced value of proveded pointer if it is not equal to 'nullptr';
// otherwise, returns 'opt::none'
template<class T>
[[nodiscard]] constexpr opt::option<T> from_nullable(T* const nullable_ptr) {
    if (nullable_ptr == nullptr) { return {}; }
    return {*nullable_ptr};
}

template<class T, class U>
constexpr impl::option::enable_swap<T, U> swap(option<T>& left, option<U>& right) noexcept(impl::option::nothrow_swap<T, U>::value) {
    left.swap(right);
}

namespace impl {
    template<std::size_t I, class Self>
    constexpr auto get_impl(Self&& self) noexcept {
        using std::get;
        using type = impl::copy_reference_t<decltype(get<I>(std::forward<Self>(self).get())), Self>;

        if (self.has_value()) {
            return opt::option<type>{static_cast<type>(get<I>(std::forward<Self>(self).get()))};
        }
        return opt::option<type>{opt::none};
    }
    template<class T, class Self>
    constexpr auto get_impl(Self&& self) noexcept {
        using std::get;
        using type = impl::copy_reference_t<decltype(get<T>(std::forward<Self>(self).get())), Self>;

        if (self.has_value()) {
            return opt::option<type>{static_cast<type>(get<T>(std::forward<Self>(self).get()))};
        }
        return opt::option<type>{opt::none};
    }
}

template<std::size_t I, class T>
[[nodiscard]] constexpr auto get(opt::option<T>& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<I>(x); }
template<std::size_t I, class T>
[[nodiscard]] constexpr auto get(const opt::option<T>& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<I>(x); }
template<std::size_t I, class T>
[[nodiscard]] constexpr auto get(opt::option<T>&& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<I>(std::move(x)); }
template<std::size_t I, class T>
[[nodiscard]] constexpr auto get(const opt::option<T>&& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<I>(std::move(x)); }

template<class T, class OptT>
[[nodiscard]] constexpr auto get(opt::option<OptT>& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<T>(x); }
template<class T, class OptT>
[[nodiscard]] constexpr auto get(const opt::option<OptT>& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<T>(x); }
template<class T, class OptT>
[[nodiscard]] constexpr auto get(opt::option<OptT>&& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<T>(std::move(x)); }
template<class T, class OptT>
[[nodiscard]] constexpr auto get(const opt::option<OptT>&& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<T>(std::move(x)); }

namespace impl {
    template<std::size_t I, class Variant>
    constexpr auto variant_get(Variant&& v) noexcept {
        auto* const ptr = std::get_if<I>(&v);
        using type = impl::copy_reference_t<decltype(*ptr), Variant>;
        return ptr == nullptr ? opt::none : opt::option<type>{static_cast<type>(*ptr)};
    }
    template<class T, class Variant>
    constexpr auto variant_get(Variant&& v) noexcept {
        auto* const ptr = std::get_if<T>(&v);
        using type = impl::copy_reference_t<decltype(*ptr), Variant>;
        return ptr == nullptr ? opt::none : opt::option<type>{static_cast<type>(*ptr)};
    }
}

template<std::size_t I, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(std::variant<Ts...>& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<I>(v); }
template<std::size_t I, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(const std::variant<Ts...>& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<I>(v); }
template<std::size_t I, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(std::variant<Ts...>&& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<I>(std::move(v)); }
template<std::size_t I, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(const std::variant<Ts...>&& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<I>(std::move(v)); }

template<class T, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(std::variant<Ts...>& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<T>(v); }
template<class T, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(const std::variant<Ts...>& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<T>(v); }
template<class T, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(std::variant<Ts...>&& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<T>(std::move(v)); }
template<class T, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(const std::variant<Ts...>&& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<T>(std::move(v)); }

namespace impl {
    template<class T>
    struct io_helper1 {
        T value;

        explicit constexpr io_helper1(T value_) noexcept
            : value{value_} {}
    };
    template<class Stream, class T>
    Stream& operator<<(Stream& ostream, io_helper1<T> x) {
        if (x.value.has_value()) {
            ostream << x.value.get();
        }
        return ostream;
    }

    template<class T, class NoneCase>
    struct io_helper2 {
        T value;
        NoneCase none_case;

        explicit constexpr io_helper2(T value_, NoneCase none_case_) noexcept
            : value{value_}, none_case{none_case_} {}
    };
    template<class Stream, class T, class NoneCase>
    Stream& operator<<(Stream& ostream, io_helper2<T, NoneCase> x) {
        if (x.value.has_value()) {
            ostream << x.value.get();
        } else {
            ostream << x.none_case;
        }
        return ostream;
    }
    template<class Stream, class T, class NoneCase>
    Stream& operator>>(Stream& istream, io_helper2<T, NoneCase> x) {
        if (x.value.has_value()) {
            istream >> x.value.get();
        } else {
            istream >> x.none_case;
        }
        return istream;
    }
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr auto io(const opt::option<T>& x OPTION_LIFETIMEBOUND) noexcept {
    return impl::io_helper1<const opt::option<T>&>{x};
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr auto io(opt::option<T>& x OPTION_LIFETIMEBOUND) noexcept {
    return impl::io_helper1<opt::option<T>&>{x};
}

template<class T, class NoneCase>
[[nodiscard]] OPTION_PURE constexpr auto io(const opt::option<T>& x OPTION_LIFETIMEBOUND, const NoneCase& none_case OPTION_LIFETIMEBOUND) noexcept {
    return impl::io_helper2<const opt::option<T>&, const NoneCase&>(x, none_case);
}
template<class T, class NoneCase>
[[nodiscard]] OPTION_PURE constexpr auto io(opt::option<T>& x OPTION_LIFETIMEBOUND, NoneCase& none_case OPTION_LIFETIMEBOUND) noexcept {
    return impl::io_helper2<opt::option<T>&, NoneCase&>(x, none_case);
}

template<class T>
[[nodiscard]] OPTION_PURE constexpr auto at(T&& container OPTION_LIFETIMEBOUND, const std::size_t index) noexcept {
    using type = impl::copy_reference_t<decltype(std::forward<T>(container)[index]), T>;

    if (index >= container.size()) { return opt::option<type>{opt::none}; }
    return opt::option<type>{static_cast<type>(std::forward<T>(container)[index])};
}

// Returns the `left` `opt::option` if it contains a value, otherwise return `right` value
// x = left option value
// y = right value
// N = empty option (none)
//| left | right | result |
//|   x  |   y   |    x   |
//|   N  |   y   |    y   |
// Same as `opt::option<T>::value_or`
template<class T>
[[nodiscard]] constexpr T operator|(const opt::option<T>& left, const T& right) {
    return left.value_or(right);
}
// Returns the `left` `opt::option` if it contains a value, otherwise return `right`
// x = left option value
// y = right option value
// N = empty option (none)
//| left | right | result |
//|   x  |   y   |    x   |
//|   x  |   N   |    x   |
//|   N  |   y   |    y   |
//|   N  |   N   |    N   |
template<class T>
[[nodiscard]] constexpr opt::option<T> operator|(const opt::option<T>& left, const opt::option<T>& right) {
    if (left.has_value()) {
        return left.get();
    }
    return right;
}
// Returns `left`
// x = left option value
// N = empty option (none)
//| left | right | result |
//|   x  |   N   |   x    |
//|   N  |   N   |   N    |
template<class T>
[[nodiscard]] constexpr opt::option<T> operator|(const opt::option<T>& left, none_t) {
    return left;
}
// Returns `right`
// y = right option value
// N = empty option (none)
//| left | right | result |
//|   N  |   y   |   y    |
//|   N  |   N   |   N    |
template<class T>
[[nodiscard]] constexpr opt::option<T> operator|(none_t, const opt::option<T>& right) {
    return right;
}

// Copy assigns the `right` `opt::option` to the `left` `opt::option`
// if the `left` `opt::option` does not contain a value.
// Returns a reference to the `left`
template<class T>
constexpr opt::option<T>& operator|=(opt::option<T>& left, const opt::option<T>& right) {
    if (!left.has_value()) {
        left = right;
    }
    return left;
}
// Move assigns the `right` `opt::option` to the `left` `opt::option`
// if the `left` `opt::option` does not contain a value.
// Returns a reference to the `left` `opt::option`
template<class T>
constexpr opt::option<T>& operator|=(opt::option<T>& left, opt::option<T>&& right) {
    if (!left.has_value()) {
        left = std::move(right);
    }
    return left;
}
// Copy assigns the `right` value to the `left` `opt::option`
// if the `left` `opt::option` does not containe a value.
// Returns a reference to the `left` `opt::option`
template<class T>
constexpr opt::option<T>& operator|=(opt::option<T>& left, const T& right) {
    if (!left.has_value()) {
        left = right;
    }
    return left;
}
// Move assigns the `right` value to the `left` `opt::option`
// if the `left` `opt::option` does not containe a value.
// Returns a reference to the `left` `opt::option`
template<class T>
constexpr opt::option<T>& operator|=(opt::option<T>& left, T&& right) {
    if (!left.has_value()) {
        left = std::move(right);
    }
    return left;
}

// Returns an empty `opt::option` if the `left` `opt::option` does not contain a value;
// otherwise return `right`
// x = left option value
// y = right option value
// N = empty option (none)
//| left | right | result |
//|   x  |   y   |   y    |
//|   x  |   N   |   N    |
//|   N  |   y   |   N    |
//|   N  |   N   |   N    |
template<class T, class U>
[[nodiscard]] constexpr opt::option<U> operator&(const opt::option<T>& left, const opt::option<U>& right) {
    if (left.has_value()) {
        return right;
    }
    return opt::none;
}

// Returns `opt::option` containing a value if exactly one of `left`, `right` contains a value;
// otherwise return an empty `opt::option`
// x = left option value
// y = right option value
// E = empty option (none)
//| left | right | result |
//|   x  |   y   |   E    |
//|   x  |   E   |   x    |
//|   E  |   y   |   y    |
//|   E  |   E   |   E    |
template<class T>
[[nodiscard]] constexpr opt::option<T> operator^(const opt::option<T>& left, const opt::option<T>& right) {
    if (left.has_value() && !right.has_value()) {
        return left;
    }
    if (!left.has_value() && right.has_value()) {
        return right;
    }
    return opt::none;
}

template<class T1, class T2>
[[nodiscard]] constexpr bool operator==(const option<T1>& left, const option<T2>& right) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() == right.get(); }
    return left_has_value == right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator!=(const option<T1>& left, const option<T2>& right) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() != right.get(); }
    return left_has_value != right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator<(const option<T1>& left, const option<T2>& right) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() < right.get(); }
    return left_has_value < right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator<=(const option<T1>& left, const option<T2>& right) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() <= right.get(); }
    return left_has_value <= right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator>(const option<T1>& left, const option<T2>& right) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() > right.get(); }
    return left_has_value > right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator>=(const option<T1>& left, const option<T2>& right) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() >= right.get(); }
    return left_has_value >= right_has_value;
}

template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator==(const option<T>& left, none_t) noexcept {
    return !left.has_value();
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator==(none_t, const opt::option<T>& right) noexcept {
    return !right.has_value();
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator!=(const option<T>& left, none_t) noexcept {
    return left.has_value();
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator!=(none_t, const opt::option<T>& right) noexcept {
    return right.has_value();
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator<([[maybe_unused]] const option<T>& left, none_t) noexcept {
    return false;
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator<(none_t, const opt::option<T>& right) noexcept {
    return right.has_value();
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator<=(const option<T>& left, none_t) noexcept {
    return !left.has_value();
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator<=(none_t, [[maybe_unused]] const opt::option<T>& right) noexcept {
    return true;
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator>(const option<T>& left, none_t) noexcept {
    return left.has_value();
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator>(none_t, [[maybe_unused]] const opt::option<T>& right) noexcept {
    return false;
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator>=([[maybe_unused]] const option<T>& left, none_t) noexcept {
    return true;
}
template<class T>
[[nodiscard]] OPTION_PURE constexpr bool operator>=(none_t, const opt::option<T>& right) noexcept {
    return !right.has_value();
}

template<class T1, class T2>
[[nodiscard]] constexpr bool operator==(const option<T1>& left, const T2& right) {
    return left.has_value() ? left.get() == right : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator==(const T1& left, const opt::option<T2>& right) {
    return right.has_value() ? left == right.get() : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator!=(const option<T1>& left, const T2& right) {
    return left.has_value() ? left.get() != right : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator!=(const T1& left, const opt::option<T2>& right) {
    return right.has_value() ? left != right.get() : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator<(const option<T1>& left, const T2& right) {
    return left.has_value() ? left.get() < right : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator<(const T1& left, const opt::option<T2>& right) {
    return right.has_value() ? left < right.get() : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator<=(const option<T1>& left, const T2& right) {
    return left.has_value() ? left.get() <= right : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator<=(const T1& left, const opt::option<T2>& right) {
    return right.has_value() ? left <= right.get() : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator>(const option<T1>& left, const T2& right) {
    return left.has_value() ? left.get() > right : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator>(const T1& left, const opt::option<T2>& right) {
    return right.has_value() ? left > right.get() : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator>=(const option<T1>& left, const T2& right) {
    return left.has_value() ? left.get() >= right : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr bool operator>=(const T1& left, const opt::option<T2>& right) {
    return right.has_value() ? left >= right.get() : true;
}

namespace impl {
    template<class T>
    struct type_wrapper {
        T m{};

        template<class... Args, std::enable_if_t<impl::is_direct_list_initializable_v<T, Args...>, int> = 0>
        constexpr type_wrapper(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : m{std::forward<Args>(args)...} {}

        type_wrapper() = default;
        type_wrapper(const type_wrapper&) = default;
        type_wrapper(type_wrapper&&) = default;
        type_wrapper& operator=(const type_wrapper&) = default;
        type_wrapper& operator=(type_wrapper&&) = default;

        constexpr type_wrapper(const T& x) noexcept(std::is_nothrow_copy_constructible_v<T>)
            : m{x} {}
        constexpr type_wrapper(T&& x) noexcept(std::is_nothrow_move_constructible_v<T>)
            : m{std::move(x)} {}

        type_wrapper& operator=(const T& x) noexcept(std::is_nothrow_copy_assignable_v<T>) {
            m = x;
            return *this;
        }
        type_wrapper& operator=(T&& x) noexcept(std::is_nothrow_move_assignable_v<T>) {
            m = std::move(x);
            return *this;
        }

        constexpr operator T&() & noexcept { return m; }
        constexpr operator const T&() const& noexcept { return m; }
        constexpr operator T&&() && noexcept { return std::move(m); }
        constexpr operator const T&&() const&& noexcept { return std::move(m); }
    };
}

template<class T, auto...>
struct sentinel : impl::type_wrapper<T> { using impl::type_wrapper<T>::type_wrapper; };

namespace impl {
    template<class T, std::uintmax_t I, auto Value, auto... Values>
    constexpr std::uintmax_t sentinel_get_level_impl(const T& value) noexcept {
        if (value == Value) { return I; }

        if constexpr (sizeof...(Values) > 0) {
            return sentinel_get_level_impl<T, I + 1, Values...>(value);
        } else {
            return std::uintmax_t(-1);
        }
    }
    template<class T, std::uintmax_t I, auto Value, auto... Values>
    constexpr void sentinel_set_level_impl(T& value, const std::uintmax_t level) noexcept {
        if (level == I) { value = Value; return; }

        if constexpr (sizeof...(Values) > 0) {
            sentinel_set_level_impl<T, I + 1, Values...>(value, level);
        } else {
            OPTION_VERIFY(false, "Level is out of range");
        }
    }
}

template<class T, auto... Values>
struct option_traits<sentinel<T, Values...>> {
private:
    using value_t = sentinel<T, Values...>;
public:
    static constexpr std::uintmax_t max_level = sizeof...(Values);

    static constexpr std::uintmax_t get_level(const value_t* const value) noexcept {
        return impl::sentinel_get_level_impl<T, 0, Values...>(*value);
    }
    static constexpr void set_level(value_t* const value, const std::uintmax_t level) noexcept {
        impl::sentinel_set_level_impl<T, 0, Values...>(*value, level);
    }
};

template<class T, class Compare, class Set, auto...>
struct sentinel_f : impl::type_wrapper<T> { using impl::type_wrapper<T>::type_wrapper; };

namespace impl {
    template<class T, class Compare, std::uintmax_t I, auto Value, auto... Values>
    constexpr std::uintmax_t sentinel_f_get_level_impl(const T& value) noexcept {
        if (Compare{}(value, Value)) { return I; }

        if constexpr (sizeof...(Values) > 0) {
            return sentinel_get_level_impl<T, Compare, I + 1, Values...>(value);
        } else {
            return std::uintmax_t(-1);
        }
    }
    template<class T, class Set, std::uintmax_t I, auto Value, auto... Values>
    constexpr void sentinel_f_set_level_impl(T& value, const std::uintmax_t level) noexcept {
        if (level == I) { Set{}(value, Value); return; }

        if constexpr (sizeof...(Values) > 0) {
            sentinel_set_level_impl<T, Set, I + 1, Values...>(value, level);
        } else {
            OPTION_VERIFY(false, "Level is out of range");
        }
    }
}

template<class T, class Compare, class Set, auto... Values>
struct option_traits<sentinel_f<T, Compare, Set, Values...>> {
private:
    using value_t = sentinel_f<T, Compare, Set, Values...>;
public:
    static constexpr std::uintmax_t max_level = sizeof...(Values);

    static constexpr std::uintmax_t get_level(const value_t* const value) noexcept {
        return impl::sentinel_f_get_level_impl<T, Compare, 0, Values...>(*value);
    }
    static constexpr void set_level(value_t* const value, const std::uintmax_t level) noexcept {
        impl::sentinel_f_set_level_impl<T, Set, 0, Values...>(*value, level);
    }
};

template<class T, auto>
struct member : impl::type_wrapper<T> { using impl::type_wrapper<T>::type_wrapper; };

template<class T, auto MemberPtr>
struct option_traits<member<T, MemberPtr>> {
private:
    using value_t = member<T, MemberPtr>;
    using member_t = typename impl::member_type<decltype(MemberPtr)>::type;
    using traits = opt::option_traits<member_t>;
public:
    static constexpr std::uintmax_t max_level = traits::max_level;

    static constexpr std::uintmax_t get_level(const value_t* const value) noexcept {
        return traits::get_level(std::addressof(static_cast<const T&>(*value).*MemberPtr));
    }
    static constexpr void set_level(value_t* const value, const std::uintmax_t level) noexcept {
        traits::set_level(std::addressof(static_cast<T&>(*value).*MemberPtr), level);
    }
};

template<class T>
struct enforce : impl::type_wrapper<T> { using impl::type_wrapper<T>::type_wrapper; };

template<class T>
struct option_traits<enforce<T>> {
private:
    using traits = opt::option_traits<T>;
public:
    static constexpr std::uintmax_t max_level = traits::max_level;

    static_assert(max_level > 0, "the 'max_level' is expected to be greater than 0 (opt::enforce<T>)");

    static constexpr std::uintmax_t get_level(const enforce<T>* const value) noexcept {
        return traits::get_level(std::addressof(static_cast<const T&>(*value)));
    }
    static constexpr void set_level(enforce<T>* const value, const std::uintmax_t level) noexcept {
        traits::set_level(std::addressof(static_cast<T&>(*value)), level);
    }
};


namespace impl {
    template<class T, class>
    using enable_hash_helper1 = T;

    template<class T, class Hash = std::hash<T>>
    using enable_hash_helper2 = std::enable_if_t<
        and_<
            std::is_default_constructible<Hash>,
            std::is_copy_constructible<Hash>,
            std::is_move_constructible<Hash>,
            std::is_destructible<Hash>,
            std::is_invocable_r<std::size_t, Hash, const T&>
        >::value
    >;
}

}

template<class T>
struct std::hash<::opt::impl::enable_hash_helper1<
    opt::option<T>, ::opt::impl::enable_hash_helper2<std::remove_const_t<T>>
>> {
    using val_hash = std::hash<std::remove_const_t<T>>;
public:
    constexpr std::size_t operator()(const opt::option<T>& val) const noexcept(noexcept(val_hash{}(*val))) {
        constexpr std::size_t disengaged_hash = 0;
        return val.has_value() ? val_hash{}(val.get()) : disengaged_hash;
    }
};
