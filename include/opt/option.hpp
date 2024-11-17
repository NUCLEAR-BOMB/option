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
#include <initializer_list>

#include <opt/option_fwd.hpp>

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

#ifdef __has_cpp_attribute
    #define OPTION_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
    #define OPTION_HAS_CPP_ATTRIBUTE(x) (0)
#endif

#ifdef __has_builtin
    #define OPTION_HAS_BUILTIN(x) __has_builtin(x)
#else
    #define OPTION_HAS_BUILTIN(x) (0)
#endif

#if OPTION_HAS_CPP_ATTRIBUTE(msvc::lifetimebound)
    #define OPTION_LIFETIMEBOUND [[msvc::lifetimebound]]
#elif OPTION_HAS_CPP_ATTRIBUTE(clang::lifetimebound)
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

#if defined(__has_feature) && OPTION_CLANG
    #if __has_feature(undefined_behavior_sanitizer)
        #define OPTION_NO_SANITIZE_OBJECT_SIZE [[clang::no_sanitize("object-size")]]
        #define OPTION_HAS_NO_SANITIZE_OBJECT_SIZE 1
    #endif
#endif
#ifndef OPTION_NO_SANITIZE_OBJECT_SIZE
    #define OPTION_NO_SANITIZE_OBJECT_SIZE
    #define OPTION_HAS_NO_SANITIZE_OBJECT_SIZE 0
#endif

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

#if OPTION_MSVC
    #define OPTION_DECLSPEC_EMPTY_BASES __declspec(empty_bases)
#else
    #define OPTION_DECLSPEC_EMPTY_BASES
#endif

#ifdef OPTION_CURRENT_FUNCTION
    #define OPTION_CAN_REFLECT_ENUM 1
#else
    #define OPTION_CAN_REFLECT_ENUM 0
#endif

#ifndef OPTION_CONSUMED_ANNOTATION_CHECKING
    #define OPTION_CONSUMED_ANNOTATION_CHECKING 0
#endif

#if OPTION_CONSUMED_ANNOTATION_CHECKING && (OPTION_HAS_CPP_ATTRIBUTE(clang::consumable) && OPTION_HAS_CPP_ATTRIBUTE(clang::callable_when) && OPTION_HAS_CPP_ATTRIBUTE(clang::param_typestate) && OPTION_HAS_CPP_ATTRIBUTE(clang::return_typestate) && OPTION_HAS_CPP_ATTRIBUTE(clang::set_typestate) && OPTION_HAS_CPP_ATTRIBUTE(clang::test_typestate))
    #define OPTION_CONSUMABLE(x) [[clang::consumable(x)]]
    #define OPTION_CALLABLE_WHEN(x) [[clang::callable_when(x)]]
    #define OPTION_PARAM_TYPESTATE(x) [[clang::param_typestate(x)]]
    #define OPTION_RETURN_TYPESTATE(x) [[clang::return_typestate(x)]]
    #define OPTION_SET_TYPESTATE(x) [[clang::set_typestate(x)]]
    #define OPTION_TEST_TYPESTATE(x) [[clang::test_typestate(x)]]
#else
    #define OPTION_CONSUMABLE(x)
    #define OPTION_CALLABLE_WHEN(x)
    #define OPTION_PARAM_TYPESTATE(x)
    #define OPTION_RETURN_TYPESTATE(x)
    #define OPTION_SET_TYPESTATE(x)
    #define OPTION_TEST_TYPESTATE(x)
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
        #define OPTION_STD_NAMESPACE_BEGIN \
            namespace std _GLIBCXX_VISIBILITY(default) { _GLIBCXX_BEGIN_NAMESPACE_VERSION \
            _Pragma("GCC diagnostic push") \
            _Pragma("GCC diagnostic ignored \"-Wignored-attributes\"")
        #define OPTION_STD_NAMESPACE_END \
            _Pragma("GCC diagnostic pop") \
            _GLIBCXX_END_NAMESPACE_VERSION }
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

    template<class T, size_t N>
    struct array; // Defined in header <array>

    template<class... Types>
    class variant; // Defined in header <variant>

#if OPTION_MSVC
    _EXPORT_STD template<size_t I, class... Types>
    _NODISCARD constexpr auto get_if(variant<Types...>*) noexcept;
    _EXPORT_STD template<size_t I, class... Types>
    _NODISCARD constexpr auto get_if(const variant<Types...>*) noexcept;
    _EXPORT_STD template<class T, class... Types>
    _NODISCARD constexpr add_pointer_t<T> get_if(variant<Types...>*) noexcept;
    _EXPORT_STD template<class T, class... Types>
    _NODISCARD constexpr add_pointer_t<const T> get_if(const variant<Types...>*) noexcept;

    _EXPORT_STD template<size_t I, class... Types>
    _NODISCARD constexpr tuple_element_t<I, tuple<Types...>>& get(tuple<Types...>&) noexcept;
    _EXPORT_STD template<size_t I, class... Types>
    _NODISCARD constexpr const tuple_element_t<I, tuple<Types...>>& get(const tuple<Types...>&) noexcept;
    _EXPORT_STD template<size_t I, class... Types>
    _NODISCARD constexpr tuple_element_t<I, tuple<Types...>>&& get(tuple<Types...>&&) noexcept;
    _EXPORT_STD template<size_t I, class... Types>
    _NODISCARD constexpr const tuple_element_t<I, tuple<Types...>>&& get(const tuple<Types...>&&) noexcept;
    _EXPORT_STD template<class T, class... Types>
    _NODISCARD constexpr T& get(tuple<Types...>&) noexcept;
    _EXPORT_STD template<class T, class... Types>
    _NODISCARD constexpr const T& get(const tuple<Types...>&) noexcept;
    _EXPORT_STD template<class T, class... Types>
    _NODISCARD constexpr T&& get(tuple<Types...>&&) noexcept;
    _EXPORT_STD template<class T, class... Types>
    _NODISCARD constexpr const T&& get(const tuple<Types...>&&) noexcept;

    _EXPORT_STD template<size_t I, class T, size_t Size>
    _NODISCARD constexpr T& get(array<T, Size>&) noexcept;
    _EXPORT_STD template<size_t I, class T, size_t Size>
    _NODISCARD constexpr const T& get(const array<T, Size>&) noexcept;
    _EXPORT_STD template<size_t I, class T, size_t Size>
    _NODISCARD constexpr T&& get(array<T, Size>&&) noexcept;
    _EXPORT_STD template<size_t I, class T, size_t Size>
    _NODISCARD constexpr const T&& get(const array<T, Size>&&) noexcept;
#elif OPTION_LIBCPP
    template<std::size_t N, class T>
    struct variant_alternative;

    template<std::size_t I, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr add_pointer_t<typename variant_alternative<I, variant<Types...>>::type> get_if(variant<Types...>*) noexcept;
    template<std::size_t I, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr add_pointer_t<const typename variant_alternative<I, variant<Types...>>::type> get_if(const variant<Types...>*) noexcept;
    template<class T, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr add_pointer_t<T> get_if(variant<Types...>*) noexcept;
    template<class T, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr add_pointer_t<const T> get_if(const variant<Types...>*) noexcept;

    template<size_t I, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr typename tuple_element<I, tuple<Types...>>::type& get(tuple<Types...>&) noexcept;
    template<size_t I, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr const typename tuple_element<I, tuple<Types...>>::type& get(const tuple<Types...>&) noexcept;
    template<size_t I, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr typename tuple_element<I, tuple<Types...>>::type&& get(tuple<Types...>&&) noexcept;
    template<size_t I, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr const typename tuple_element<I, tuple<Types...>>::type&& get(const tuple<Types...>&&) noexcept;
    template<class T, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr T& get(tuple<Types...>&) noexcept;
    template<class T, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr const T& get(const tuple<Types...>&) noexcept;
    template<class T, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr T&& get(tuple<Types...>&&) noexcept;
    template<class T, class... Types>
    _LIBCPP_HIDE_FROM_ABI constexpr const T&& get(const tuple<Types...>&&) noexcept;

    template<size_t I, class T, size_t Size>
    _LIBCPP_HIDE_FROM_ABI constexpr T& get(array<T, Size>&) noexcept;
    template<size_t I, class T, size_t Size>
    _LIBCPP_HIDE_FROM_ABI constexpr const T& get(const array<T, Size>&) noexcept;
    template<size_t I, class T, size_t Size>
    _LIBCPP_HIDE_FROM_ABI constexpr T&& get(array<T, Size>&&) noexcept;
    template<size_t I, class T, size_t Size>
    _LIBCPP_HIDE_FROM_ABI constexpr const T&& get(const array<T, Size>&&) noexcept;
#else
    template<size_t N, class T>
    struct variant_alternative;

    template<size_t I, class... Types>
    constexpr add_pointer_t<typename variant_alternative<I, variant<Types...>>::type> get_if(variant<Types...>*) noexcept;
    template<size_t I, class... Types>
    constexpr add_pointer_t<const typename variant_alternative<I, variant<Types...>>::type> get_if(const variant<Types...>*) noexcept;
    template<class T, class... Types>
    constexpr add_pointer_t<T> get_if(variant<Types...>*) noexcept;
    template<class T, class... Types>
    constexpr add_pointer_t<const T> get_if(const variant<Types...>*) noexcept;

    template<size_t I, class... Types>
    constexpr __tuple_element_t<I, tuple<Types...>>& get(tuple<Types...>&) noexcept;
    template<size_t I, class... Types>
    constexpr const __tuple_element_t<I, tuple<Types...>>& get(const tuple<Types...>&) noexcept;
    template<size_t I, class... Types>
    constexpr __tuple_element_t<I, tuple<Types...>>&& get(tuple<Types...>&&) noexcept;
    template<size_t I, class... Types>
    constexpr const __tuple_element_t<I, tuple<Types...>>&& get(const tuple<Types...>&&) noexcept;
    template<class T, class... Types>
    constexpr T& get(tuple<Types...>&) noexcept;
    template<class T, class... Types>
    constexpr const T& get(const tuple<Types...>&) noexcept;
    template<class T, class... Types>
    constexpr T&& get(tuple<Types...>&&) noexcept;
    template<class T, class... Types>
    constexpr const T&& get(const tuple<Types...>&&) noexcept;

    template<size_t I, class T, size_t Size>
    [[nodiscard]] constexpr T& get(array<T, Size>&) noexcept;
    template<size_t I, class T, size_t Size>
    [[nodiscard]] constexpr const T& get(const array<T, Size>&) noexcept;
    template<size_t I, class T, size_t Size>
    [[nodiscard]] constexpr T&& get(array<T, Size>&&) noexcept;
    template<size_t I, class T, size_t Size>
    [[nodiscard]] constexpr const T&& get(const array<T, Size>&&) noexcept;
#endif
    struct input_iterator_tag; // Defined in header <iterator>
    struct output_iterator_tag; // Defined in header <iterator>
    struct forward_iterator_tag; // Defined in header <iterator>
    struct bidirectional_iterator_tag; // Defined in header <iterator>
    struct random_access_iterator_tag; // Defined in header <iterator>
#if OPTION_IS_CXX20
    struct contiguous_iterator_tag; // Defined in header <iterator>
#endif
    template<class T>
    struct hash; // Defined in header <functional>

    // Forward declared in <utility>
    // template<class... Types>
    // class tuple; // Defined in header <tuple>

    template<class T>
    class complex; // Defined in header <complex>
OPTION_STD_NAMESPACE_END
#else
    #include <string_view>
    #include <string>
    #include <vector>
    #include <memory>
    #include <array>
    #include <variant>
    #include <complex>
#endif

#if OPTION_IS_CXX20
    #include <memory> // for std::construct_at
#endif

#if OPTION_HAS_BUILTIN(__builtin_addressof) || OPTION_MSVC
    #define OPTION_ADDRESSOF(x) __builtin_addressof(x)
#else
    #include <memory>
    #define OPTION_ADDRESSOF(x) ::std::addressof(x)
#endif

namespace opt {

namespace impl {
#if OPTION_HAS_BUILTIN(__is_trivially_destructible) || OPTION_MSVC
    // Don't check for std::is_destructible because non-destructible types are anyway not allowed inside opt::option
    template<class T>
    using is_trivially_destructible = std::bool_constant<__is_trivially_destructible(T)>;
    template<class T>
    inline constexpr bool is_trivially_destructible_v = __is_trivially_destructible(T);
#elif OPTION_HAS_BUILTIN(__has_trivial_destructor)
    template<class T>
    using is_trivially_destructible = std::bool_constant<__has_trivial_destructor(T)>;
    template<class T>
    inline constexpr bool is_trivially_destructible_v = __has_trivial_destructor(T);
#else
    using std::is_trivially_destructible;
    using std::is_trivially_destructible_v;
#endif

#if OPTION_HAS_BUILTIN(__is_trivially_assignable) || OPTION_MSVC
    // Don't uses std::add_lvalue_reference and std::add_rvalue_reference because non-referenceable types are anyway not allowed inside opt::option
    template<class T>
    using is_trivially_copy_assignable = std::bool_constant<__is_trivially_assignable(T&, const T&)>;
    template<class T>
    inline constexpr bool is_trivially_copy_assignable_v = __is_trivially_assignable(T&, const T&);

    template<class T>
    using is_trivially_move_assignable = std::bool_constant<__is_trivially_assignable(T&, T&&)>;
    template<class T>
    inline constexpr bool is_trivially_move_assignable_v = __is_trivially_assignable(T&, T&&);
#else
    using std::is_trivially_copy_assignable;
    using std::is_trivially_copy_assignable_v;

    using std::is_trivially_move_assignable;
    using std::is_trivially_move_assignable_v;
#endif

#if OPTION_HAS_BUILTIN(__is_same)
    template<class T1, class T2>
    using is_not_same = std::bool_constant<!__is_same(T1, T2)>;

    template<class T1, class T2>
    inline constexpr bool is_not_same_v = !__is_same(T1, T2);
#else
    template<class T1, class T2>
    struct is_not_same : std::true_type {};
    template<class T>
    struct is_not_same<T, T> : std::false_type {};

    template<class T1, class T2>
    inline constexpr bool is_not_same_v = true;
    template<class T>
    inline constexpr bool is_not_same_v<T, T> = false;
#endif

#if OPTION_HAS_BUILTIN(__remove_cvref)
    #if OPTION_GCC
        template<class T>
        struct remove_cvref_gcc_workaround {
            using type = __remove_cvref(T);
        };
        template<class T>
        using remove_cvref = typename remove_cvref_gcc_workaround<T>::type;
    #else
        template<class T>
        using remove_cvref = __remove_cvref(T);
    #endif
#else
    template<class T>
    struct remove_cvref_impl { using type = T; };
    template<class T>
    struct remove_cvref_impl<T&> { using type = T; };
    template<class T>
    struct remove_cvref_impl<T&&> { using type = T; };

    template<class T>
    struct remove_cvref_impl<const T> { using type = T; };
    template<class T>
    struct remove_cvref_impl<const T&> { using type = T; };
    template<class T>
    struct remove_cvref_impl<const T&&> { using type = T; };

    template<class T>
    struct remove_cvref_impl<volatile T> { using type = T; };
    template<class T>
    struct remove_cvref_impl<volatile T&> { using type = T; };
    template<class T>
    struct remove_cvref_impl<volatile T&&> { using type = T; };

    template<class T>
    struct remove_cvref_impl<const volatile T> { using type = T; };
    template<class T>
    struct remove_cvref_impl<const volatile T&> { using type = T; };
    template<class T>
    struct remove_cvref_impl<const volatile T&&> { using type = T; };

    template<class T>
    using remove_cvref = typename remove_cvref_impl<T>::type;
#endif

    // Simplified std::invoke (without noexcept and SFINAE)
    template<class T, class... Args>
    constexpr decltype(auto) invoke(T&& fn, Args&&... args) {
        return static_cast<T&&>(fn)(static_cast<Args&&>(args)...);
    }
    template<class T, class C, class First, class... Args,
        std::enable_if_t<std::is_function_v<T> && (std::is_same_v<C, remove_cvref<First>> || std::is_base_of_v<C, remove_cvref<First>>), int> = 0>
    constexpr decltype(auto) invoke(T C::* obj, First&& first, Args&&... args) {
        return (static_cast<First&&>(first).*obj)(static_cast<Args&&>(args)...);
    }
    template<class T, class C, class First, class... Args,
        std::enable_if_t<std::is_function_v<T> && !(std::is_same_v<C, remove_cvref<First>> || std::is_base_of_v<C, remove_cvref<First>>), int> = 0>
    constexpr decltype(auto) invoke(T C::* obj, First&& first, Args&&... args) {
        return ((*static_cast<First&&>(first)).*obj)(static_cast<Args&&>(args)...);
    }
    template<class T, class C, class First,
        std::enable_if_t<!std::is_function_v<T> && (std::is_same_v<C, remove_cvref<First>> || std::is_base_of_v<C, remove_cvref<First>>), int> = 0>
    constexpr decltype(auto) invoke(T C::* obj, First&& first) {
        return static_cast<First&&>(first).*obj;
    }
    template<class T, class C, class First,
        std::enable_if_t<!std::is_function_v<T> && !(std::is_same_v<C, remove_cvref<First>> || std::is_base_of_v<C, remove_cvref<First>>), int> = 0>
    constexpr decltype(auto) invoke(T C::* obj, First&& first) {
        return (*static_cast<First&&>(first)).*obj;
    }
    template<class T, class C, class First, class... Args,
        std::enable_if_t<std::is_function_v<T>, int> = 0>
    constexpr decltype(auto) invoke(T C::* obj, std::reference_wrapper<First> first, Args&&... args) {
        return (first.get().*obj)(static_cast<Args&&>(args)...);
    }
    template<class T, class C, class First,
        std::enable_if_t<!std::is_function_v<T>, int> = 0>
    constexpr decltype(auto) invoke(T C::* obj, std::reference_wrapper<First> first) {
        return first.get().*obj;
    }

#if OPTION_IS_CXX20
    using std::construct_at;
#else
    template<class T, class... Args>
    constexpr void construct_at(T* ptr, Args&&... args) {
        if constexpr (impl::is_trivially_move_assignable_v<T>) {
            if constexpr (std::is_aggregate_v<T>) {
                *ptr = T{static_cast<Args&&>(args)...};
            } else {
                *ptr = T(static_cast<Args&&>(args)...);
            }
        } else {
            if constexpr (std::is_aggregate_v<T>) {
                ::new(static_cast<void*>(ptr)) T{static_cast<Args&&>(args)...};
            } else {
                ::new(static_cast<void*>(ptr)) T(static_cast<Args&&>(args)...);
            }
        }
    }
#endif

    template<class T, class Traits, class = std::uintmax_t>
    inline constexpr bool has_get_level_method = false;
    template<class T, class Traits>
    inline constexpr bool has_get_level_method<T, Traits, decltype(static_cast<std::uintmax_t>(Traits::get_level(std::declval<const T*>())))>
        = noexcept(static_cast<std::uintmax_t>(Traits::get_level(std::declval<const T*>())));

    template<class T, class Traits, class = void>
    inline constexpr bool has_set_level_method = false;
    template<class T, class Traits>
    inline constexpr bool has_set_level_method<T, Traits, decltype(Traits::set_level(std::declval<std::remove_const_t<T>*>(), std::declval<std::uintmax_t>()))>
        = noexcept(Traits::set_level(std::declval<std::remove_const_t<T>*>(), std::declval<std::uintmax_t>()));

    template<class T, class = void>
    inline constexpr bool has_padding_member = false;
    template<class T>
    inline constexpr bool has_padding_member<T, decltype(std::declval<T&>().PADDING, void())> = true;

    template<class T, class = void>
    inline constexpr bool has_sentinel_enumerator = false;
    template<class T>
    inline constexpr bool has_sentinel_enumerator<T, decltype(T::SENTINEL, void())> = true;

    template<class T, class = void>
    inline constexpr bool has_sentinel_start_enumerator = false;
    template<class T>
    inline constexpr bool has_sentinel_start_enumerator<T, decltype(T::SENTINEL_START, void())> = true;

    template<class T, class = void>
    inline constexpr bool has_sentinel_end_enumerator = false;
    template<class T>
    inline constexpr bool has_sentinel_end_enumerator<T, decltype(T::SENTINEL_END, void())> = true;

    template<class T, class = void>
    inline constexpr bool is_custom_tuple_like = false;
    template<class T>
    inline constexpr bool is_custom_tuple_like<T, decltype(std::tuple_size<T>::value, void())> = true;

#if !OPTION_MSVC
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
#else
    template<class... Pred>
    using and_ = std::conjunction<Pred...>;

    template<class... Pred>
    using or_ = std::disjunction<Pred...>;
#endif
    template<class Pred>
    using not_ = std::bool_constant<!Pred::value>;

    template<class... Pred>
    inline constexpr bool and_v = and_<Pred...>::value;
    template<class... Pred>
    inline constexpr bool or_v = or_<Pred...>::value;
    template<class Pred>
    inline constexpr bool not_v = not_<Pred>::value;

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

#if OPTION_USE_BUILTIN_TRAITS

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

    constexpr std::size_t index_of_max(const std::initializer_list<std::uintmax_t> levels) noexcept {
        std::size_t idx = 0;
        std::uintmax_t max_value = levels.begin()[0];

        std::size_t i = 1;
        for (const std::uintmax_t* it = levels.begin() + 1; it < levels.end(); ++it, ++i) {
            if (*it > max_value) {
                max_value = *it;
                idx = i;
            }
        }
        return idx;
    }

    template<class... Traits>
    struct find_max_level {
        static constexpr std::size_t index = index_of_max({Traits::max_level...});
#if OPTION_HAS_BUILTIN(__type_pack_element)
        using type = __type_pack_element<index, Traits...>;
#else
        using type = std::tuple_element_t<index, std::tuple<Traits...>>;
#endif
    };
    template<>
    struct find_max_level<> {
        static constexpr std::size_t index = std::size_t(-1);
        struct type {
            static constexpr std::uintmax_t max_level = 0;
        };
    };

#ifdef OPTION_HAS_PFR
    template<class Struct>
    inline constexpr bool pfr_is_option_reflectable =
        OPTION_PFR_NAMESPACE is_implicitly_reflectable_v<Struct, option_tag>;

    template<class Struct>
    struct pfr_find_max_level;
    template<class... Ts>
    struct pfr_find_max_level<std::tuple<Ts&...>>
        : find_max_level<opt::option_traits<Ts>...> {};
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
    struct tuple_like_find_max_level;

    template<class TupleLike, std::size_t... Index>
    struct tuple_like_find_max_level<TupleLike, std::integer_sequence<std::size_t, Index...>>
        : find_max_level<opt::option_traits<std::tuple_element_t<Index, TupleLike>>...> {};

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
        reference_option,
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
        padding_member,
        tuple_like,
        enumeration_sentinel,
        enumeration_sentinel_start,
        enumeration_sentinel_start_end,
#if OPTION_CAN_REFLECT_ENUM
        enumeration,
#endif
        complex,
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
    template<class T>
    struct dispatch_specializations<opt::option<T&>> {
        static constexpr option_strategy value = option_strategy::reference_option;
    };
    template<class T>
    struct dispatch_specializations<opt::option<T&&>> {
        static constexpr option_strategy value = option_strategy::reference_option;
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
    struct dispatch_specializations<std::complex<T>> {
        static constexpr option_strategy value = option_strategy::complex;
    };

    template<class T>
    constexpr option_strategy detemine_option_strategy() {
        using st = option_strategy;
        constexpr st dispatch_st = dispatch_specializations<T>::value;

        if constexpr (dispatch_st != st::other) {
            return dispatch_st;
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
        if constexpr (has_padding_member<T>) {
            return st::padding_member;
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
            return uint_bool(impl::ptr_bit_cast<uint_bool>(value) - 2);
        }
        static void set_level(bool* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, uint_bool(level + 2));
        }
    };

    template<class T>
    struct internal_option_traits<std::reference_wrapper<T>, option_strategy::reference_wrapper> {
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const std::reference_wrapper<T>* const value) noexcept {
            const std::uintptr_t ptr = impl::ptr_bit_cast<std::uintptr_t>(value);
            return ptr;
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
            const std::uint64_t uint = impl::ptr_bit_cast<std::uint64_t>(value);
            return uint - ptr_offset;
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uint64_t(ptr_offset + level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::pointer_32> {
    private:
        static constexpr std::uint32_t ptr_offset = 0xFFFF'FFFFUL - 63;
    public:
        static constexpr std::uintmax_t max_level = 32;

        static std::uintmax_t get_level(const T* const value) noexcept {
            const std::uint32_t uint = impl::ptr_bit_cast<std::uint32_t>(value);
            return uint - ptr_offset;
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uint32_t(ptr_offset + level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::float64_sNaN> {
    private:
        static constexpr std::uint64_t nan_start = 0b1'11111111111'0110110001111001111101010101101100001000100110001111;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            const std::uint64_t uint = impl::ptr_bit_cast<std::uint64_t>(value);
            return uint - nan_start;
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uint64_t(nan_start + level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::float64_qNaN> {
    private:
        static constexpr std::uint64_t nan_start = 0b1'11111111111'1011111100100110010000110000101110110011010101010111;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            const std::uint64_t uint = impl::ptr_bit_cast<std::uint64_t>(value);
            return uint - nan_start;
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uint64_t(nan_start + level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::float32_sNaN> {
    private:
        static constexpr std::uint32_t nan_start = 0b1'11111111'01111110110100110101111u;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            const std::uint32_t uint = impl::ptr_bit_cast<std::uint32_t>(value);
            return uint - nan_start;
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uint32_t(nan_start + level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::float32_qNaN> {
    private:
        static constexpr std::uint32_t nan_start = 0b1'11111111'10000111110111110110101u;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            const std::uint32_t uint = impl::ptr_bit_cast<std::uint32_t>(value);
            return uint - nan_start;
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, std::uint32_t(nan_start + level));
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
                return traits::get_level(OPTION_ADDRESSOF(value->first));
            } else {
                return traits::get_level(OPTION_ADDRESSOF(value->second));
            }
        }
        static constexpr void set_level(std::pair<First, Second>* const value, const std::uintmax_t level) noexcept {
            if constexpr (first_is_max) {
                traits::set_level(OPTION_ADDRESSOF(value->first), level);
            } else {
                traits::set_level(OPTION_ADDRESSOF(value->second), level);
            }
        }
    };
    template<class... Ts>
    struct internal_option_traits<std::tuple<Ts...>, option_strategy::tuple> {
    private:
        using find_result = find_max_level<opt::option_traits<Ts>...>;

        static constexpr std::size_t tuple_index = find_result::index;
    public:
        static constexpr std::uintmax_t max_level = find_result::type::max_level;

        static constexpr std::uintmax_t get_level(const std::tuple<Ts...>* const value) noexcept {
            return find_result::type::get_level(OPTION_ADDRESSOF(std::get<tuple_index>(*value)));
        }
        static constexpr void set_level(std::tuple<Ts...>* const value, const std::uintmax_t level) noexcept {
            find_result::type::set_level(OPTION_ADDRESSOF(std::get<tuple_index>(*value)), level);
        }
    };

    template<class T, std::size_t N>
    struct internal_option_traits<std::array<T, N>, option_strategy::array> {
    private:
        using traits = opt::option_traits<T>;
    public:
        static constexpr std::uintmax_t max_level = traits::max_level;

        static constexpr std::uintmax_t get_level(const std::array<T, N>* const value) noexcept {
            return traits::get_level(OPTION_ADDRESSOF((*value)[0]));
        }
        static constexpr void set_level(std::array<T, N>* const value, const std::uintmax_t level) noexcept {
            traits::set_level(OPTION_ADDRESSOF((*value)[0]), level);
        }
    };

#ifdef OPTION_HAS_PFR
    template<class T>
    struct internal_option_traits<T, option_strategy::reflectable> {
    private:
        using find_result = pfr_find_max_level<
            decltype(OPTION_PFR_NAMESPACE structure_tie(std::declval<T&>()))
        >;
        static constexpr std::size_t index = find_result::index;
    public:
        static constexpr std::uintmax_t max_level = find_result::type::max_level;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            return find_result::type::get_level(OPTION_ADDRESSOF(OPTION_PFR_NAMESPACE get<index>(*value)));
        }
        static constexpr void set_level(T* const value, const std::uintmax_t level) noexcept {
            find_result::type::set_level(OPTION_ADDRESSOF(OPTION_PFR_NAMESPACE get<index>(*value)), level);
        }
    };
#endif // defined(OPTION_HAS_PFR)
    template<class T>
    struct internal_option_traits<T, option_strategy::polymorphic> {
    private:
        static constexpr std::uintptr_t vtable_ptr_start = std::uintptr_t(-1) - 89151;
    public:
        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const T* const value) noexcept {
            const std::uintptr_t vtable_ptr = impl::ptr_bit_cast_least<std::uintptr_t>(value);
            return vtable_ptr - vtable_ptr_start;
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
            const std::uintptr_t uint_ptr = reinterpret_cast<std::uintptr_t>(value->data());
            return uint_ptr - sentinel_ptr;
        }
        static void set_level(std::basic_string_view<Elem, Traits>* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            const Elem* const ptr = reinterpret_cast<const Elem*>(sentinel_ptr + level);
            impl::construct_at(value, ptr, std::size_t(0));
        }
    };
    template<class Elem>
    struct internal_option_traits<std::unique_ptr<Elem, std::default_delete<Elem>>, option_strategy::unique_ptr> {
    private:
        static constexpr std::uintptr_t sentinel_ptr = std::uintptr_t(-1) - 46508;
    public:
        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const std::unique_ptr<Elem, std::default_delete<Elem>>* const value) noexcept {
            const std::uintptr_t uint = reinterpret_cast<std::uintptr_t>(value->get());
            return uint - sentinel_ptr;
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
            const std::uint32_t uint = impl::ptr_bit_cast<std::uint32_t>(value);
            return uint - offset_start;
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
            const std::uint64_t uint = impl::ptr_bit_cast_least<std::uint64_t>(value);
            return uint - offset_start;
        }
        static void set_level(type* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy_least(value, std::uint64_t(level + offset_start));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::padding_member> {
    private:
        using member_type = decltype(std::declval<T&>().PADDING);
        static_assert(std::is_unsigned_v<member_type>, ".PADDING member must be an unsigned integer");
    public:
        static constexpr std::uintmax_t max_level = std::uintmax_t(~member_type(0));

        static std::uintmax_t get_level(const T* const value) noexcept {
            return std::uintmax_t(std::uintmax_t(value->PADDING) - 1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            value->PADDING = member_type(level + 1);
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
            const underlying uint = impl::ptr_bit_cast<underlying>(value);
            return uint - max_enumerator_value;
        }
        static constexpr void set_level(T* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, underlying(level + max_enumerator_value));
        }
    };
#endif // OPTION_CAN_REFLECT_ENUM
    template<class T>
    struct internal_option_traits<T, option_strategy::enumeration_sentinel> {
    private:
        using underlying = std::underlying_type_t<T>;

        static constexpr T sentinel_value = T::SENTINEL;
    public:
        static constexpr std::uintmax_t max_level = 1;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            return std::uintmax_t(underlying(*value) - underlying(sentinel_value));
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
            const underlying val = impl::ptr_bit_cast<underlying>(value);
            return std::uintmax_t(val - sentinel_range_start);
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
            const underlying val = impl::ptr_bit_cast<underlying>(value);
            return std::uintmax_t(val - sentinel_range_start);
        }
        static constexpr void set_level(T* const value, [[maybe_unused]] const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            impl::ptr_bit_copy(value, underlying(sentinel_range_start + level));
        }
    };

    template<class T>
    struct internal_option_traits<T, option_strategy::tuple_like> {
    private:
        using find_result = tuple_like_find_max_level<T>;

        static constexpr std::size_t index = find_result::index;
    public:
        static constexpr std::uintmax_t max_level = find_result::type::max_level;

        static constexpr std::uintmax_t get_level(const T* const value) noexcept {
            return find_result::type::get_level(OPTION_ADDRESSOF(tuple_like_get<index>(*value)));
        }
        static constexpr void set_level(T* const value, const std::uintmax_t level) noexcept {
            find_result::type::set_level(OPTION_ADDRESSOF(tuple_like_get<index>(*value)), level);
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
            if (capacity == 0) {
                return size;
            } else {
                return std::uintmax_t(-1);
            }
#elif OPTION_LIBCPP
            std::size_t cap_and_is_long{};
            std::memcpy(&cap_and_is_long, reinterpret_cast<const std::uint8_t*>(value) + capacity_offset, sizeof(std::size_t));
            std::size_t size{};
            std::memcpy(&size, reinterpret_cast<const std::uint8_t*>(value) + size_offset, sizeof(std::size_t));
            if (cap_and_is_long == magic_capacity) {
                return size;
            } else {
                return std::uintmax_t(-1);
            }
#elif OPTION_LIBSTDCPP
            const void* data{};
            std::memcpy(&data, reinterpret_cast<const std::uint8_t*>(value) + data_offset, sizeof(void*));
            std::size_t size{};
            std::memcpy(&size, reinterpret_cast<const std::uint8_t*>(value) + size_offset, sizeof(std::size_t));
            if (data == nullptr) {
                return size;
            } else {
                return std::uintmax_t(-1);
            }
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
        static constexpr std::uintptr_t ptr_offset = std::uintptr_t(-1) - 76326;
    public:
        static constexpr std::uintmax_t max_level = 255;

        static constexpr std::uintmax_t get_level(const std::vector<T, Allocator>* const value) noexcept {
            std::uintptr_t first{};
            std::memcpy(&first, reinterpret_cast<const std::uint8_t*>(value) + 0, ptr_size);
            return first - ptr_offset;
        }
        static constexpr void set_level(std::vector<T, Allocator>* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            const std::uintptr_t first = std::uintptr_t(ptr_offset + level);
            std::memcpy(reinterpret_cast<std::uint8_t*>(value) + 0, &first, ptr_size);
        }
    };
#endif // !OPTION_UNKNOWN_STD

    template<class T>
    struct internal_option_traits<std::complex<T>, option_strategy::complex> {
    private:
        using fp_traits = opt::option_traits<T>;
    public:
        static constexpr std::uintmax_t max_level = fp_traits::max_level;

        static constexpr std::uintmax_t get_level(const std::complex<T>* const value) noexcept {
            const T real_part = value->real();
            return fp_traits::get_level(OPTION_ADDRESSOF(real_part));
        }
        static constexpr void set_level(std::complex<T>* const value, const std::uintmax_t level) noexcept {
            OPTION_VERIFY(level < max_level, "Level is out of range");
            T real_part{};
            fp_traits::set_level(OPTION_ADDRESSOF(real_part), level);
            value->real(real_part);
        }
    };

#if OPTION_CLANG
    #pragma clang diagnostic pop
#elif OPTION_GCC
    #pragma GCC diagnostic pop
#endif

#endif // OPTION_USE_BUILTIN_TRAITS
}

#if OPTION_USE_BUILTIN_TRAITS
template<class T, class>
struct option_traits : impl::internal_option_traits<T> {};
#else
template<class T, class>
struct option_traits {
    static constexpr std::uintmax_t max_level = 0;
};
#endif

#if OPTION_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wcomma"
#endif

template<class T>
struct option_traits<T, decltype(sizeof(sentinel_option_traits<T>), void())> {
    static constexpr std::uintmax_t max_level = 1;

    static constexpr std::uintmax_t get_level(const T* const value) noexcept {
        return std::uintmax_t(!(*value == sentinel_option_traits<T>::sentinel_value));
    }
    static constexpr void set_level(T* const value, const std::uintmax_t level) noexcept {
        OPTION_VERIFY(level == 0, "Level is out of range");
        *value = sentinel_option_traits<T>::sentinel_value;
    }
};

#if OPTION_CLANG
    #pragma clang diagnostic pop
#endif

namespace impl {
    struct construct_from_invoke_tag {
        explicit construct_from_invoke_tag() = default;
    };

    template<class Tuple>
    struct tuple_like_of_options {
        static_assert(!sizeof(Tuple),
            "To unzip opt::option<T>, T must be tuple-like."
            "A type T that satisfies tuple-like must be a specialization of "
            "std::array, std::pair, std::tuple");
    };
    template<class... Ts>
    struct tuple_like_of_options<std::tuple<Ts...>> {
        using type = std::tuple<opt::option<Ts>...>;
    };
    template<class T1, class T2>
    struct tuple_like_of_options<std::pair<T1, T2>> {
        using type = std::pair<opt::option<T1>, opt::option<T2>>;
    };
    template<class T, std::size_t N>
    struct tuple_like_of_options<std::array<T, N>> {
        using type = std::array<opt::option<T>, N>;
    };
    template<class Tuple>
    using tuple_like_of_options_t = typename tuple_like_of_options<Tuple>::type;

#if !OPTION_IS_CXX20
    template<bool IsAggregate, bool IsConstructible, class, class T, class... Args>
    struct is_initializable_from_impl
        : std::bool_constant<IsConstructible> {};

    template<class T, class... Args>
    struct is_initializable_from_impl<true, false, decltype(T{std::declval<Args>()...}, void()), T, Args...>
        : std::true_type {};

#if !OPTION_MSVC
    template<class T, class... Args>
    using is_initializable_from = is_initializable_from_impl<std::is_aggregate_v<T>, std::is_constructible_v<T, Args...>, void, T, Args...>;
#else
    template<class T, class... Args>
    using is_initializable_from = is_initializable_from_impl<__is_aggregate(T), __is_constructible(T, Args...), void, T, Args...>;
#endif
#else
    template<class T, class... Args>
    using is_initializable_from = std::is_constructible<T, Args...>;
#endif
    template<class T, class... Args>
    inline constexpr bool is_initializable_from_v = is_initializable_from<T, Args...>::value;

    template<class T>
    struct member_type {
        static_assert(!sizeof(T), "Expected pointer to data member");
    };
    template<class C, class T>
    struct member_type<T C::*> { using type = T; };

    template<class T, class U>
    struct copy_reference {
        using type = T;
    };
    template<class T, class U>
    struct copy_reference<T&, U> {
        using type = T&&;
    };
    template<class T, class U>
    struct copy_reference<T&&, U> {
        using type = T&&;
    };
    template<class T, class U>
    struct copy_reference<T&, U&> {
        using type = T&;
    };
    template<class T, class U>
    using copy_reference_t = typename copy_reference<T, U>::type;

    template<class T, class U>
    struct copy_lvalue_reference {
        using type = T;
    };
    template<class T, class U>
    struct copy_lvalue_reference<T, U&> {
        using type = T&;
    };
    template<class T, class U>
    struct copy_lvalue_reference<T&&, U&> {
        using type = T&&;
    };
    template<class T, class U>
    using copy_lvalue_reference_t = typename copy_lvalue_reference<T, U>::type;

    template<class T>
    inline constexpr bool is_reference_wrapper_v = false;
    template<class T>
    inline constexpr bool is_reference_wrapper_v<std::reference_wrapper<T>> = true;

    // See https://github.com/microsoft/STL/pull/878#issuecomment-639696118
    struct nontrivial_dummy {
        constexpr nontrivial_dummy() noexcept {}
    };

#if OPTION_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4296) // 'operator' : expression is always false
#endif

    template<class T,
        bool TriviallyDestructible = impl::is_trivially_destructible_v<T>,
        bool HasTraits = (opt::option_traits<std::remove_cv_t<T>>::max_level > 0)
    >
    struct option_destruct_base;

#if OPTION_MSVC
    #pragma warning(pop)
#endif

    template<class T>
    struct option_destruct_base<T, /*TriviallyDestructible=*/true, /*HasTraits=*/false> {
        union {
            nontrivial_dummy dummy;
            std::remove_const_t<T> value;
        };
        bool has_value_flag;

        constexpr option_destruct_base() noexcept
            : dummy{}, has_value_flag(false) {}

        template<class... Args>
        constexpr option_destruct_base(std::in_place_t, std::true_type, Args&&... args)
            : value{static_cast<Args&&>(args)...}, has_value_flag(true) {}

        template<class... Args>
        constexpr option_destruct_base(std::in_place_t, std::false_type, Args&&... args)
            : value(static_cast<Args&&>(args)...), has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, std::true_type, F&& f, Arg&& arg)
            : value{impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg))}, has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, std::false_type, F&& f, Arg&& arg)
            : value(impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg))), has_value_flag(true) {}

        constexpr void reset() noexcept {
            has_value_flag = false;
        }
        OPTION_PURE constexpr bool has_value() const noexcept {
            return has_value_flag;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            impl::construct_at(OPTION_ADDRESSOF(value), static_cast<Args&&>(args)...);
            has_value_flag = true;
        }
    };
    template<class T>
    struct option_destruct_base<T, /*TriviallyDestructible=*/false, /*HasTraits=*/false> {
        union {
            nontrivial_dummy dummy;
            std::remove_const_t<T> value;
        };
        bool has_value_flag;

        constexpr option_destruct_base() noexcept
            : dummy{}, has_value_flag(false) {}

        template<class... Args>
        constexpr option_destruct_base(std::in_place_t, std::true_type, Args&&... args)
            : value{static_cast<Args&&>(args)...}, has_value_flag(true) {}

        template<class... Args>
        constexpr option_destruct_base(std::in_place_t, std::false_type, Args&&... args)
            : value(static_cast<Args&&>(args)...), has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, std::true_type, F&& f, Arg&& arg)
            : value{impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg))}, has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, std::false_type, F&& f, Arg&& arg)
            : value(impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg))), has_value_flag(true) {}

        OPTION_CONSTEXPR_CXX20 ~option_destruct_base() {
            if (has_value_flag) {
                value.~T();
            }
        }

        constexpr void reset() {
            if (has_value_flag) {
                value.~T();
                has_value_flag = false;
            }
        }
        OPTION_PURE constexpr bool has_value() const noexcept {
            return has_value_flag;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            impl::construct_at(OPTION_ADDRESSOF(value), static_cast<Args&&>(args)...);
            has_value_flag = true;
        }
    };
#if OPTION_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    template<class T>
    struct option_destruct_base<T, /*TriviallyDestructible=*/true, /*HasTraits=*/true> {
        union {
            char dummy;
            std::remove_const_t<T> value;
        };
        using traits = opt::option_traits<std::remove_cv_t<T>>;

        static_assert(impl::has_get_level_method<T, traits>, "The static method 'get_level' in 'opt::option_traits' does not exist or has an invalid function signature");
        static_assert(impl::has_set_level_method<T, traits>, "The static method 'set_level' in 'opt::option_traits' does not exist or has an invalid function signature");
        
        constexpr option_destruct_base() noexcept
            : dummy{} {
            traits::set_level(OPTION_ADDRESSOF(value), 0);
            OPTION_VERIFY(!has_value(), "After the default construction, the value is in an empty state.");
        }
        template<class... Args>
        constexpr option_destruct_base(const std::in_place_t, std::true_type, Args&&... args)
            : value{static_cast<Args&&>(args)...} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class... Args>
        constexpr option_destruct_base(std::in_place_t, std::false_type, Args&&... args)
            : value(static_cast<Args&&>(args)...) {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, std::true_type, F&& f, Arg&& arg)
            : value{impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg))} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, std::false_type, F&& f, Arg&& arg)
            : value(impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg))) {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }

        constexpr void reset() noexcept {
            traits::set_level(OPTION_ADDRESSOF(value), 0);
            OPTION_VERIFY(!has_value(), "After resetting, the value is in an empty state.");
        }
        OPTION_PURE constexpr bool has_value() const noexcept {
            return traits::get_level(OPTION_ADDRESSOF(value)) != 0;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            impl::construct_at(OPTION_ADDRESSOF(value), static_cast<Args&&>(args)...);
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
    };
    template<class T>
    struct option_destruct_base<T, /*TriviallyDestructible=*/false, /*HasTraits=*/true> {
        union {
            char dummy;
            std::remove_const_t<T> value;
        };
        using traits = opt::option_traits<std::remove_cv_t<T>>;

        static_assert(impl::has_get_level_method<T, traits>, "The static method 'get_level' in 'opt::option_traits' does not exist or has an invalid function signature");
        static_assert(impl::has_set_level_method<T, traits>, "The static method 'set_level' in 'opt::option_traits' does not exist or has an invalid function signature");
        
        constexpr option_destruct_base() noexcept
            : dummy{} {
            traits::set_level(OPTION_ADDRESSOF(value), 0);
            OPTION_VERIFY(!has_value(), "After the default construction, the value is in an empty state.");
        }
        template<class... Args>
        constexpr option_destruct_base(const std::in_place_t, std::true_type, Args&&... args)
            : value{static_cast<Args&&>(args)...} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class... Args>
        constexpr option_destruct_base(std::in_place_t, std::false_type, Args&&... args)
            : value(static_cast<Args&&>(args)...) {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, std::true_type, F&& f, Arg&& arg)
            : value{impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg))} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, std::false_type, F&& f, Arg&& arg)
            : value(impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg))) {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        OPTION_CONSTEXPR_CXX20 ~option_destruct_base() {
            if (has_value()) {
                value.~T();
            }
        }

        constexpr void reset() noexcept {
            if (has_value()) {
                value.~T();
                traits::set_level(OPTION_ADDRESSOF(value), 0);
                OPTION_VERIFY(!has_value(), "After resetting, the value is in an empty state.");
            }
        }
        OPTION_PURE constexpr bool has_value() const noexcept {
            return traits::get_level(OPTION_ADDRESSOF(value)) != 0;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            impl::construct_at(OPTION_ADDRESSOF(value), static_cast<Args&&>(args)...);
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
    };
#if OPTION_GCC
    #pragma GCC diagnostic pop
#endif

    template<class Self, class Option>
    constexpr void option_assign_from_option(Self& self, Option&& other) {
        if (other.has_value()) {
            if (self.has_value()) {
                self.value = static_cast<Option&&>(other).value;
            } else {
                self.construct(static_cast<Option&&>(other).value);
            }
        } else {
            self.reset();
        }
    }
    template<class Self, class Option>
    constexpr void option_construct_from_option(Self& self, Option&& other) {
        if (other.has_value()) {
            self.construct(static_cast<Option&&>(other).value);
        }
    }

#if OPTION_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wextra" // A base class is not initialized in the copy constructor of a derived class
#endif
    template<class T,
        bool TrivialCopyCtor = std::is_trivially_copy_constructible_v<T>,
        bool TrivialCopyAssignment = impl::is_trivially_copy_assignable_v<T> && impl::is_trivially_destructible_v<T>,
        bool TrivialMoveCtor = std::is_trivially_move_constructible_v<T>,
        bool TrivialMoveAssignment = impl::is_trivially_move_assignable_v<T> && impl::is_trivially_destructible_v<T>,
        bool is_reference = std::is_reference_v<T>
    >
    struct option_base : public option_destruct_base<T> {
        using option_destruct_base<T>::option_destruct_base;
    };

    // If TrivialCopyCtor is false, then always do not enable the trivial copy assignment operator
    template<class T, bool TrivialCopyAssignment>
    struct option_base<T, false, TrivialCopyAssignment, true, true, false> : public option_destruct_base<T> {
        using option_destruct_base<T>::option_destruct_base;

        option_base() = default;
        constexpr option_base(const option_base& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            impl::option_construct_from_option(*this, other);
        }
        constexpr option_base& operator=(const option_base& other) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
            impl::option_assign_from_option(*this, other);
            return *this;
        }
        option_base(option_base&&) = default;
        option_base& operator=(option_base&&) = default;
    };
    template<class T>
    struct option_base<T, true, false, true, true, false> : public option_destruct_base<T> {
        using option_destruct_base<T>::option_destruct_base;

        option_base() = default;
        option_base(const option_base&) = default;
        constexpr option_base& operator=(const option_base& other) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
            impl::option_assign_from_option(*this, other);
            return *this;
        }
        option_base(option_base&&) = default;
        option_base& operator=(option_base&&) = default;
    };
    // If TrivialMoveAssignment is false, then always do not enable the trivial move assignment operator
    template<class T, bool TrivialMoveAssignment>
    struct option_base<T, true, true, false, TrivialMoveAssignment, false> : public option_destruct_base<T> {
        using option_destruct_base<T>::option_destruct_base;

        option_base() = default;
        option_base(const option_base&) = default;
        option_base& operator=(const option_base&) = default;
        constexpr option_base(option_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
            impl::option_construct_from_option(*this, static_cast<option_base&&>(other));
        }
        constexpr option_base& operator=(option_base&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
            impl::option_assign_from_option(*this, static_cast<option_base&&>(other));
            return *this;
        }
    };
    // If TrivialMoveAssignment is false, then always do not enable the trivial move assignment operator
    // If TrivialCopyCtor is false, then always do not enable the trivial copy assignment operator
    template<class T, bool TrivialCopyAssignment, bool TrivialMoveAssignment>
    struct option_base<T, false, TrivialCopyAssignment, false, TrivialMoveAssignment, false> : public option_destruct_base<T> {
        using option_destruct_base<T>::option_destruct_base;

        option_base() = default;
        constexpr option_base(const option_base& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            impl::option_construct_from_option(*this, other);
        }
        constexpr option_base& operator=(const option_base& other) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
            impl::option_assign_from_option(*this, other);
            return *this;
        }
        constexpr option_base(option_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
            impl::option_construct_from_option(*this, static_cast<option_base&&>(other));
        }
        constexpr option_base& operator=(option_base&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
            impl::option_assign_from_option(*this, static_cast<option_base&&>(other));
            return *this;
        }
    };
    // If TrivialMoveAssignment is false, then always do not enable the trivial move assignment operator
    template<class T, bool TrivialMoveAssignment>
    struct option_base<T, true, false, false, TrivialMoveAssignment, false> : public option_destruct_base<T>
    {
        using option_destruct_base<T>::option_destruct_base;

        option_base() = default;
        option_base(const option_base&) = default;
        constexpr option_base& operator=(const option_base& other) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
            impl::option_assign_from_option(*this, other);
            return *this;
        }
        constexpr option_base(option_base&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
            impl::option_construct_from_option(*this, static_cast<option_base&&>(other));
        }
        constexpr option_base& operator=(option_base&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
            impl::option_assign_from_option(*this, static_cast<option_base&&>(other));
            return *this;
        }
    };
    template<class T>
    struct option_base<T, true, true, true, false, false> : public option_destruct_base<T> {
        using option_destruct_base<T>::option_destruct_base;

        option_base() = default;
        option_base(const option_base&) = default;
        option_base& operator=(const option_base&) = default;
        option_base(option_base&&) = default;
        constexpr option_base& operator=(option_base&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
            impl::option_assign_from_option(*this, static_cast<option_base&&>(other));
            return *this;
        }
    };
    // If TrivialCopyCtor is false, then always do not enable the trivial copy assignment operator
    template<class T, bool TrivialCopyAssignment>
    struct option_base<T, false, TrivialCopyAssignment, true, false, false> : public option_destruct_base<T> {
        using option_destruct_base<T>::option_destruct_base;

        option_base() = default;
        constexpr option_base(const option_base& other) noexcept(std::is_nothrow_copy_constructible_v<T>) {
            impl::option_construct_from_option(*this, other);
        }
        constexpr option_base& operator=(const option_base& other) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
            impl::option_assign_from_option(*this, other);
            return *this;
        }
        option_base(option_base&&) = default;
        constexpr option_base& operator=(option_base&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
            impl::option_assign_from_option(*this, static_cast<option_base&&>(other));
            return *this;
        }
    };
    template<class T>
    struct option_base<T, true, false, true, false, false> : public option_destruct_base<T> {
        using option_destruct_base<T>::option_destruct_base;

        option_base() = default;
        option_base(const option_base&) = default;
        constexpr option_base& operator=(const option_base& other) noexcept(std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) {
            impl::option_assign_from_option(*this, other);
            return *this;
        }
        option_base(option_base&&) = default;
        constexpr option_base& operator=(option_base&& other) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
            impl::option_assign_from_option(*this, static_cast<option_base&&>(other));
            return *this;
        }
    };
#if OPTION_GCC
    #pragma GCC diagnostic pop
#endif

    template<class T,
        bool TrivialCopyCtor,
        bool TrivialCopyAssignment,
        bool TrivialMoveCtor,
        bool TrivialMoveAssignment>
    struct option_base<T, TrivialCopyCtor, TrivialCopyAssignment, TrivialMoveCtor, TrivialMoveAssignment, /*is_reference=*/true> {
        using unref_type = std::remove_reference_t<T>;

        static constexpr unref_type* ref_to_ptr(T&& other) noexcept {
            return OPTION_ADDRESSOF(other);
        }
        template<class U>
        static constexpr unref_type* ref_to_ptr(std::reference_wrapper<U> other) noexcept {
            return OPTION_ADDRESSOF(other.get());
        }

        unref_type* value;

        constexpr option_base() noexcept
            : value{nullptr} {}

        template<class Arg, bool IsAggregate>
        constexpr option_base(const std::in_place_t, std::bool_constant<IsAggregate>, Arg&& arg) noexcept
            : value{ref_to_ptr(static_cast<Arg&&>(arg))} {}

        template<class F, class Arg, bool IsAggregate>
        constexpr option_base(construct_from_invoke_tag, std::bool_constant<IsAggregate>, F&& f, Arg&& arg)
            : value{ref_to_ptr(impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg)))} {
            using fn_result = decltype(impl::invoke(static_cast<F&&>(f), static_cast<Arg&&>(arg)));
            static_assert(std::is_reference_v<fn_result> || is_reference_wrapper_v<fn_result>,
                "function returned prvalue, expected lvalue or rvalue");
        }

        OPTION_PURE constexpr bool has_value() const noexcept {
            return value != nullptr;
        }
        constexpr void reset() noexcept {
            value = nullptr;
        }

        template<class Arg>
        constexpr void construct(Arg&& arg) noexcept {
            value = ref_to_ptr(static_cast<Arg&&>(arg));
        }
    };

    // Tag is to distinguish between derived `enable_copy_move` and derived `enable_copy_move` inside contained nested `opt::option`.
    // Without it, the empty base optimization will fail and it will take extra space to store `enable_copy_move`.
    template<class Tag, bool CopyCtor, bool CopyAssignment, bool MoveCtor, bool MoveAssignment, bool IsReference>
    struct enable_copy_move {};

    template<class Tag>
    struct enable_copy_move<Tag, true, false, true, true, false> {
        enable_copy_move() = default;
        enable_copy_move(const enable_copy_move&) = default;
        enable_copy_move& operator=(const enable_copy_move&) = delete;
        enable_copy_move(enable_copy_move&&) = default;
        enable_copy_move& operator=(enable_copy_move&&) = default;
    };
    template<class Tag, bool CopyAssignment>
    struct enable_copy_move<Tag, false, CopyAssignment, true, true, false> {
        enable_copy_move() = default;
        enable_copy_move(const enable_copy_move&) = delete;
        enable_copy_move& operator=(const enable_copy_move&) = delete;
        enable_copy_move(enable_copy_move&&) = default;
        enable_copy_move& operator=(enable_copy_move&&) = default;
    };
    template<class Tag, bool MoveAssignment>
    struct enable_copy_move<Tag, true, true, false, MoveAssignment, false> {
        enable_copy_move() = default;
        enable_copy_move(const enable_copy_move&) = default;
        enable_copy_move& operator=(const enable_copy_move&) = default;
        enable_copy_move(enable_copy_move&&) = delete;
        enable_copy_move& operator=(enable_copy_move&&) = delete;
    };
    template<class Tag, bool CopyAssignment, bool MoveAssignment>
    struct enable_copy_move<Tag, false, CopyAssignment, false, MoveAssignment, false> {
        enable_copy_move() = default;
        enable_copy_move(const enable_copy_move&) = delete;
        enable_copy_move& operator=(const enable_copy_move&) = delete;
        enable_copy_move(enable_copy_move&&) = delete;
        enable_copy_move& operator=(enable_copy_move&&) = delete;
    };
    template<class Tag, bool MoveAssignment>
    struct enable_copy_move<Tag, true, false, false, MoveAssignment, false> {
        enable_copy_move() = default;
        enable_copy_move(const enable_copy_move&) = default;
        enable_copy_move& operator=(const enable_copy_move&) = delete;
        enable_copy_move(enable_copy_move&&) = delete;
        enable_copy_move& operator=(enable_copy_move&&) = delete;
    };
    template<class Tag>
    struct enable_copy_move<Tag, true, true, true, false, false> {
        enable_copy_move() = default;
        enable_copy_move(const enable_copy_move&) = default;
        enable_copy_move& operator=(const enable_copy_move&) = default;
        enable_copy_move(enable_copy_move&&) = default;
        enable_copy_move& operator=(enable_copy_move&&) = delete;
    };
    template<class Tag, bool CopyAssignment>
    struct enable_copy_move<Tag, false, CopyAssignment, true, false, false> {
        enable_copy_move() = default;
        enable_copy_move(const enable_copy_move&) = delete;
        enable_copy_move& operator=(const enable_copy_move&) = delete;
        enable_copy_move(enable_copy_move&&) = default;
        enable_copy_move& operator=(enable_copy_move&&) = delete;
    };
    template<class Tag>
    struct enable_copy_move<Tag, true, false, true, false, false> {
        enable_copy_move() = default;
        enable_copy_move(const enable_copy_move&) = default;
        enable_copy_move& operator=(const enable_copy_move&) = delete;
        enable_copy_move(enable_copy_move&&) = default;
        enable_copy_move& operator=(enable_copy_move&&) = delete;
    };

#if OPTION_HAS_NO_SANITIZE_OBJECT_SIZE
    #pragma clang attribute push(OPTION_NO_SANITIZE_OBJECT_SIZE, apply_to = function)
#endif
    template<class T>
    class option_iterator {
    public:
#if OPTION_IS_CXX20
        using iterator_concept = std::contiguous_iterator_tag;
#endif
        using iterator_category = std::random_access_iterator_tag;
        using value_type = std::remove_reference_t<T>;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
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
#if OPTION_HAS_NO_SANITIZE_OBJECT_SIZE
    #pragma clang attribute pop
#endif

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

#if OPTION_CLANG && OPTION_CONSUMED_ANNOTATION_CHECKING
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wconsumed"
#endif

namespace impl::option {
    template<class T, class Self, class... Args>
    constexpr T value_or_construct(Self&& self, Args&&... args) {
        if (self.has_value()) {
            return static_cast<Self&&>(self).get();
        } else {
            if constexpr (std::is_aggregate_v<T>) {
                return T{static_cast<Args&&>(args)...};
            } else {
                return T(static_cast<Args&&>(args)...);
            }
        }
    }

    template<class Self, class F>
    constexpr auto and_then(Self&& self, F&& f) {
        using invoke_res = impl::remove_cvref<decltype(impl::invoke(static_cast<F&&>(f), *static_cast<Self&&>(self)))>;
        static_assert(opt::is_option_v<invoke_res>, "The return type of function F must be a specialization of opt::option");
        if (self.has_value()) {
            return impl::invoke(static_cast<F&&>(f), *static_cast<Self&&>(self));
        } else {
            return invoke_res{opt::none};
        }
    }

    template<class T, class Self, class F>
    constexpr auto map(Self&& self, F&& f) {
        using f_result = std::remove_cv_t<decltype(impl::invoke(static_cast<F&&>(f), *static_cast<Self&&>(self)))>;
        if (self.has_value()) {
            return opt::option<f_result>{construct_from_invoke_tag{}, static_cast<F&&>(f), *static_cast<Self&&>(self)};
        }
        return opt::option<f_result>{opt::none};
    }

    template<class T, class Self, class U, class F>
    constexpr impl::remove_cvref<U> map_or(Self&& self, U&& default_value, F&& f) {
        if (self.has_value()) {
            return impl::invoke(static_cast<F&&>(f), static_cast<Self&&>(self).get());
        }
        return static_cast<U&&>(default_value);
    }

    template<class T, class Self, class D, class F>
    constexpr auto map_or_else(Self&& self, D&& d, F&& f) {
        using d_result = decltype(static_cast<D&&>(d)());
        using f_result = decltype(impl::invoke(static_cast<F&&>(f), *static_cast<Self&&>(self)));
        static_assert(std::is_same_v<d_result, f_result>,
            "The type of the invoke result functions D and F must be the same");
        if (self.has_value()) {
            return impl::invoke(static_cast<F&&>(f), static_cast<Self&&>(self).get());
        }
        return static_cast<D&&>(d)();
    }

    template<class T, class Self, class F>
    constexpr opt::option<T> or_else(Self&& self, F&& f) {
        using f_result = decltype(static_cast<F&&>(f)());
        static_assert(std::is_same_v<impl::remove_cvref<f_result>, opt::option<T>>,
            "The function F must return an opt::option<T>");
        if (self.has_value()) {
            return static_cast<Self&&>(self);
        }
        return static_cast<F&&>(f)();
    }

    template<class Self>
    constexpr auto&& value_or_throw(Self&& self) {
        if (!self.has_value()) { throw_bad_access(); }
        return *static_cast<Self&&>(self);
    }

    template<class Self, class P>
    constexpr bool has_value_and(Self&& self, P&& predicate) {
        if (self.has_value()) {
            return impl::invoke(static_cast<P&&>(predicate), static_cast<Self&&>(self).get());
        }
        return false;
    }

    template<class Self, class F>
    constexpr Self& inspect(Self&& self, F&& f) {
        if (self.has_value()) {
            impl::invoke(static_cast<F&&>(f), static_cast<Self&&>(self).get());
        }
        return self;
    }

    template<class Self, class F>
    constexpr impl::remove_cvref<Self> filter(Self&& self, F&& f) {
        if (self.has_value() && bool(impl::invoke(static_cast<F&&>(f), self.get()))) {
            return static_cast<Self&&>(self).get();
        }
        return opt::none;
    }

    template<class T, class U>
    using enable_swap = std::enable_if_t<
        and_v<
            std::is_move_constructible<T>,
            std::is_move_constructible<U>,
            std::is_swappable_with<T, U>
        >
    >;
    template<class T, class U>
    inline constexpr bool nothrow_swap =
        std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_move_constructible_v<U>
        && std::is_nothrow_swappable_with_v<T, U>;
}

namespace impl {
    struct always_non_explicit {
        template<bool Condition>
        using is_explicit = std::enable_if<!Condition, int>;
    };
    template<class T, class U>
    struct check_from_value_ctor {
        template<bool Condition>
        using is_explicit = std::enable_if<
            (std::is_convertible_v<U, T> != Condition) && is_initializable_from_v<T, U>
        , int>;
    };
    template<class T, class U, class QualU>
    struct check_option_like_ctor {
        using UOpt = opt::option<U>;

        using is_constructible_from_option =
            or_<std::is_constructible<T, UOpt&>,
                std::is_constructible<T, const UOpt&>,
                std::is_constructible<T, UOpt&&>,
                std::is_constructible<T, const UOpt&&>,
                std::is_convertible<UOpt&, T>,
                std::is_convertible<const UOpt&, T>,
                std::is_convertible<UOpt&&, T>,
                std::is_convertible<const UOpt&&, T>>;

        using is_assignable_from_option =
            or_<std::is_assignable<T&, UOpt&>,
                std::is_assignable<T&, const UOpt&>,
                std::is_assignable<T&, UOpt&&>,
                std::is_assignable<T&, const UOpt&&>>;

        template<bool Condition>
        using constructor_is_explicit = std::enable_if<
            (std::is_convertible_v<QualU, T> != Condition)
            && (std::is_same_v<std::remove_cv_t<T>, bool> || !is_constructible_from_option::value)
        , int>;

        using assignment = std::enable_if<
            !is_constructible_from_option::value && !is_assignable_from_option::value>;
    };
    struct always_assign {
        using assignment = std::enable_if<true>;
    };

    struct option_check_fail {};

    template<bool IsReference/*=false*/>
    struct option_checks_base {
        template<class T, class U>
        using from_value_ctor = if_<
            is_not_same_v<remove_cvref<U>, opt::option<T>>
            && is_not_same_v<remove_cvref<U>, std::in_place_t>
            && (!std::is_same_v<std::remove_cv_t<T>, bool> || !opt::is_option_v<remove_cvref<U>>),
            check_from_value_ctor<T, U>, option_check_fail>;

        template<class T, class First, class... Args>
        using from_args_ctor = std::enable_if<
            and_v<
                is_not_same<remove_cvref<First>, opt::option<T>>,
                is_not_same<remove_cvref<First>, std::in_place_t>,
                is_initializable_from<T, First, Args...>
            >
        >;
        
        template<class T, class InPlaceT, class... Args>
        using from_in_place_args_ctor = std::enable_if<
            and_v<
                std::is_same<InPlaceT, std::in_place_t>,
                is_initializable_from<T, Args...>
            >
        >;

        template<class T, class U, class QualU>
        using from_option_like_ctor = if_<
            is_not_same_v<U, T>
            && is_initializable_from_v<T, QualU>,
            check_option_like_ctor<T, U, QualU>, option_check_fail
        >;
        template<class T, class U, class QualU>
        using from_option_like_assign = if_<
            is_not_same_v<U, T>
            && is_initializable_from_v<T, QualU>
            && std::is_assignable_v<T&, QualU>,
            check_option_like_ctor<T, U, QualU>, option_check_fail
        >;

        template<class T, class U>
        using from_value_assign = std::enable_if<
            is_not_same_v<remove_cvref<U>, opt::option<T>>
            && is_initializable_from_v<T, U>
            && std::is_assignable_v<T&, U>
        >;
    };

    template<class T, class U>
    inline constexpr bool is_same_reference_wrapper_v = false;
    template<class T>
    inline constexpr bool is_same_reference_wrapper_v<const T, std::reference_wrapper<T>> = true;
    template<class T>
    inline constexpr bool is_same_reference_wrapper_v<T, std::reference_wrapper<T>> = true;

    template<>
    struct option_checks_base</*IsReference=*/true> {
        template<class T, class U>
        using from_value_ctor = if_<
            is_not_same_v<remove_cvref<U>, std::in_place_t>
            && !opt::is_option_v<remove_cvref<U>>
            && (is_same_reference_wrapper_v<std::remove_reference_t<T>, std::remove_reference_t<U>>
                || (std::is_convertible_v<std::remove_reference_t<U>*, std::remove_reference_t<T>*>
                && (std::is_lvalue_reference_v<T> == std::is_lvalue_reference_v<U>))),
            always_non_explicit, option_check_fail>;

        template<class T, class First, class... Args>
        using from_args_ctor = std::enable_if<sizeof(First) == 0>;

        template<class T, class InPlaceT, class... Args>
        using from_in_place_args_ctor = std::enable_if<sizeof...(Args) == 1>;

        template<class T, class U, class QualU>
        using from_option_like_ctor = if_<
            is_not_same_v<U, T>
            && (is_same_reference_wrapper_v<std::remove_reference_t<T>, std::remove_reference_t<U>>
                || (std::is_convertible_v<std::remove_reference_t<U>*, std::remove_reference_t<T>*>
                && (std::is_lvalue_reference_v<T> == std::is_lvalue_reference_v<U>))),
            always_non_explicit, option_check_fail
        >;
        template<class T, class U, class QualU>
        using from_option_like_assign = if_<
            is_not_same_v<U, T>
            && (is_same_reference_wrapper_v<std::remove_reference_t<T>, std::remove_reference_t<U>>
                || (std::is_convertible_v<std::remove_reference_t<U>*, std::remove_reference_t<T>*>
                && (std::is_lvalue_reference_v<T> == std::is_lvalue_reference_v<U>))),
            always_assign, option_check_fail
        >;

        template<class T, class U>
        using from_value_assign = std::enable_if<
            is_not_same_v<remove_cvref<U>, opt::option<T>>
            && (is_same_reference_wrapper_v<std::remove_reference_t<T>, std::remove_reference_t<U>>
                || (std::is_convertible_v<std::remove_reference_t<U>*, std::remove_reference_t<T>*>
                && (std::is_lvalue_reference_v<T> == std::is_lvalue_reference_v<U>)))
        >;
    };
}

template<class T>
class OPTION_DECLSPEC_EMPTY_BASES OPTION_CONSUMABLE(unconsumed) option
    : private impl::option_base<T>
    , private impl::enable_copy_move<
        /*Tag=*/T,
        std::is_copy_constructible_v<T>,
        std::is_copy_assignable_v<T>,
        std::is_move_constructible_v<T>,
        std::is_move_assignable_v<T>,
        std::is_reference_v<T>
    >
{
    using base = impl::option_base<T>;

#if OPTION_USE_BUILTIN_TRAITS
    template<class, impl::option_strategy> friend struct impl::internal_option_traits;
#endif
    template<class> friend class option;

    using checks = impl::option_checks_base<std::is_reference_v<T>>;
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
    // static_assert(std::is_destructible_v<T>,
    //     "T must be destructible");

    using value_type = T;
    using iterator = impl::option_iterator<T>;
    using const_iterator = impl::option_iterator<const T>;

    OPTION_RETURN_TYPESTATE(unconsumed)
    constexpr option() noexcept {}

    OPTION_RETURN_TYPESTATE(unconsumed)
    constexpr option(opt::none_t) noexcept {}

    option(const option&) = default;

    option(option&&) = default;

    template<class U = std::remove_cv_t<T>, typename checks::template from_value_ctor<T, U>::template is_explicit<true>::type = 0>
    OPTION_RETURN_TYPESTATE(consumed)
    constexpr explicit option(U&& val)
        : base(std::in_place, std::bool_constant<std::is_aggregate_v<T>>{}, static_cast<U&&>(val)) {}
    template<class U = std::remove_cv_t<T>, typename checks::template from_value_ctor<T, U>::template is_explicit<false>::type = 0>
    OPTION_RETURN_TYPESTATE(consumed)
    constexpr option(U&& val)
        : base(std::in_place, std::bool_constant<std::is_aggregate_v<T>>{}, static_cast<U&&>(val)) {}

    template<class First, class Second, class... Args,
        class = typename checks::template from_args_ctor<T, First, Second, Args...>::type>
    OPTION_RETURN_TYPESTATE(consumed)
    constexpr explicit option(First&& first, Second&& second, Args&&... args)
        : base(std::in_place, std::bool_constant<std::is_aggregate_v<T>>{}, static_cast<First&&>(first), static_cast<Second&&>(second), static_cast<Args&&>(args)...) {}

    template<class InPlaceT, class... Args,
        class = typename checks::template from_in_place_args_ctor<T, InPlaceT, Args...>::type>
    OPTION_RETURN_TYPESTATE(consumed)
    constexpr explicit option(const InPlaceT, Args&&... args)
        : base(std::in_place, std::bool_constant<std::is_aggregate_v<T>>{}, static_cast<Args&&>(args)...) {}

    template<class InPlaceT, class U, class... Args,
        class = typename checks::template from_in_place_args_ctor<T, InPlaceT, std::initializer_list<U>&, Args...>::type>
    OPTION_RETURN_TYPESTATE(consumed)
    constexpr explicit option(const InPlaceT, std::initializer_list<U> ilist, Args&&... args)
        : base(std::in_place, std::bool_constant<std::is_aggregate_v<T>>{}, ilist, static_cast<Args&&>(args)...) {}

    template<class F, class Arg>
    OPTION_RETURN_TYPESTATE(consumed)
    constexpr explicit option(const impl::construct_from_invoke_tag, F&& f, Arg&& arg)
        : base(impl::construct_from_invoke_tag{}, std::bool_constant<std::is_aggregate_v<T>>{}, static_cast<F&&>(f), static_cast<Arg&&>(arg)) {}

    template<class U,
        typename checks::template from_option_like_ctor<T, U, const U&>::template constructor_is_explicit<false>::type = 0>
    constexpr option(const option<U>& other) {
        if (other.has_value()) {
            base::construct(other.get());
        }
    }
    template<class U,
        typename checks::template from_option_like_ctor<T, U, const U&>::template constructor_is_explicit<true>::type = 0>
    constexpr explicit option(const option<U>& other) {
        if (other.has_value()) {
            base::construct(other.get());
        }
    }
    template<class U,
        typename checks::template from_option_like_ctor<T, U, U&&>::template constructor_is_explicit<false>::type = 0>
    constexpr option(option<U>&& other) {
        if (other.has_value()) {
            base::construct(static_cast<option<U>&&>(other).get());
        }
    }
    template<class U,
        typename checks::template from_option_like_ctor<T, U, U&&>::template constructor_is_explicit<true>::type = 0>
    constexpr explicit option(option<U>&& other) {
        if (other.has_value()) {
            base::construct(static_cast<option<U>&&>(other).get());
        }
    }

    OPTION_SET_TYPESTATE(unconsumed)
    constexpr option& operator=(opt::none_t) noexcept {
        reset();
        return *this;
    }

    option& operator=(const option&) = default;
    
    option& operator=(option&&) = default;

    template<class U = std::remove_cv_t<T>,
        class = typename checks::template from_value_assign<T, U>::type>
    OPTION_SET_TYPESTATE(consumed)
    constexpr option& operator=(U&& val) {
        if constexpr (std::is_reference_v<T>) {
            base::value = base::ref_to_ptr(static_cast<U&&>(val));
        } else if constexpr (std::is_scalar_v<T>) {
            base::construct(static_cast<U&&>(val));
        } else {
            if (has_value()) {
                base::value = static_cast<U&&>(val);
                OPTION_VERIFY(has_value(), "After assignment, the value is in an empty state");
            } else {
                base::construct(static_cast<U&&>(val));
            }
        }
        return *this;
    }

    template<class U,
        class = typename checks::template from_option_like_assign<T, U, const U&>::assignment::type>
    constexpr option& operator=(const option<U>& other) {
        if constexpr (std::is_reference_v<T>) {
            if (other.has_value()) {
                base::value = base::ref_to_ptr(static_cast<option<U>&&>(other.get()));
            } else {
                reset();
            }
        } else {
            if (other.has_value()) {
                if (has_value()) {
                    base::value = other.get();
                    OPTION_VERIFY(has_value(), "After assignment, the value is in an empty state");
                } else {
                    base::construct(other.get());
                }
            } else {
                reset();
            }
        }
        return *this;
    }
    template<class U,
        class = typename checks::template from_option_like_assign<T, U, U&&>::assignment::type>
    constexpr option& operator=(option<U>&& other) {
        if constexpr (std::is_reference_v<T>) {
            if (other.has_value()) {
                base::value = base::ref_to_ptr(static_cast<option<U>&&>(other).get());
            } else {
                reset();
            }
        } else {
            if (other.has_value()) {
                if (has_value()) {
                    base::value = static_cast<option<U>&&>(other).get();
                    OPTION_VERIFY(has_value(), "After assignment, the value is in an empty state");
                } else {
                    base::construct(static_cast<option<U>&&>(other).get());
                }
            } else {
                reset();
            }
        }
        return *this;
    }

    OPTION_NO_SANITIZE_OBJECT_SIZE
    [[nodiscard]] constexpr iterator begin() noexcept OPTION_LIFETIMEBOUND {
        return iterator{has_value() ? OPTION_ADDRESSOF(get()) : nullptr};
    }
    OPTION_NO_SANITIZE_OBJECT_SIZE
    [[nodiscard]] constexpr const_iterator begin() const noexcept OPTION_LIFETIMEBOUND {
        return const_iterator{has_value() ? OPTION_ADDRESSOF(get()) : nullptr};
    }
    OPTION_NO_SANITIZE_OBJECT_SIZE
    [[nodiscard]] constexpr iterator end() noexcept OPTION_LIFETIMEBOUND {
        return iterator{has_value() ? (OPTION_ADDRESSOF(get()) + 1) : nullptr};
    }
    OPTION_NO_SANITIZE_OBJECT_SIZE
    [[nodiscard]] constexpr const_iterator end() const noexcept OPTION_LIFETIMEBOUND {
        return const_iterator{has_value() ? (OPTION_ADDRESSOF(get()) + 1) : nullptr};
    }

    constexpr void reset() noexcept {
        base::reset();
    }

    template<class... Args>
    OPTION_SET_TYPESTATE(consumed)
    constexpr T& emplace(Args&&... args) OPTION_LIFETIMEBOUND {
        reset();
        base::construct(static_cast<Args&&>(args)...);
        return *(*this);
    }

    template<class... Args>
    OPTION_SET_TYPESTATE(consumed)
    constexpr T& try_emplace(Args&&... args) OPTION_LIFETIMEBOUND {
        if (!has_value()) {
            base::construct(static_cast<Args&&>(args)...);
        }
        return *(*this);
    }

    OPTION_TEST_TYPESTATE(consumed)
    [[nodiscard]] OPTION_PURE constexpr bool has_value() const noexcept {
        return base::has_value();
    }
    OPTION_TEST_TYPESTATE(consumed)
    [[nodiscard]] OPTION_PURE constexpr explicit operator bool() const noexcept {
        return base::has_value();
    }

    template<class P>
    [[nodiscard]] constexpr bool has_value_and(P&& predicate) & { return impl::option::has_value_and(*this, static_cast<P&&>(predicate)); }
    template<class P>
    [[nodiscard]] constexpr bool has_value_and(P&& predicate) const& { return impl::option::has_value_and(*this, static_cast<P&&>(predicate)); }
    template<class P>
    [[nodiscard]] constexpr bool has_value_and(P&& predicate) && { return impl::option::has_value_and(static_cast<option&&>(*this), static_cast<P&&>(predicate)); }
    template<class P>
    [[nodiscard]] constexpr bool has_value_and(P&& predicate) const&& { return impl::option::has_value_and(static_cast<const option&&>(*this), static_cast<P&&>(predicate)); }

    OPTION_SET_TYPESTATE(unconsumed) OPTION_RETURN_TYPESTATE(unknown)
    [[nodiscard]] constexpr option take() {
        option tmp{static_cast<option&&>(*this)};
        reset();
        return tmp;
    }

    template<class P>
    [[nodiscard]] constexpr option<T> take_if(P&& predicate) {
        if (has_value() && bool(impl::invoke(static_cast<P&&>(predicate), get()))) {
            return take();
        }
        return opt::none;
    }

    template<class F>
    constexpr option& inspect(F&& f) & { return impl::option::inspect(*this, static_cast<F&&>(f)); }
    template<class F>
    constexpr const option& inspect(F&& f) const& { return impl::option::inspect(*this, static_cast<F&&>(f)); }
    template<class F>
    constexpr option& inspect(F&& f) && { return impl::option::inspect(static_cast<option&&>(*this), static_cast<F&&>(f)); }
    template<class F>
    constexpr const option& inspect(F&& f) const&& { return impl::option::inspect(static_cast<const option&&>(*this), static_cast<F&&>(f)); }

    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr T& get() & noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        if constexpr (std::is_reference_v<T>) {
            return *base::value;
        } else {
            return base::value;
        }
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr const T& get() const& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        if constexpr (std::is_reference_v<T>) {
            return *base::value;
        } else {
            return base::value;
        }
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr T&& get() && noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        if constexpr (std::is_reference_v<T>) {
            return static_cast<T&&>(*base::value);
        } else {
            return static_cast<T&&>(base::value);
        }
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr const T&& get() const&& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        if constexpr (std::is_reference_v<T>) {
            return static_cast<const T&&>(*base::value);
        } else {
            return static_cast<const T&&>(base::value);
        }
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr std::add_pointer_t<const T> operator->() const noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return OPTION_ADDRESSOF(get());
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr std::add_pointer_t<T> operator->() noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return OPTION_ADDRESSOF(get());
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr T& operator*() & noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return get();
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr const T& operator*() const& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return get();
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr T&& operator*() && noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return static_cast<T&&>(get());
    }
    OPTION_CALLABLE_WHEN(consumed)
    [[nodiscard]] OPTION_PURE constexpr const T&& operator*() const&& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return static_cast<const T&&>(get());
    }

    [[nodiscard]] OPTION_PURE constexpr T& get_unchecked() & noexcept {
        if constexpr (std::is_reference_v<T>) {
            OPTION_VERIFY(base::value != nullptr, "Dereferencing null pointer");
            return *base::value;
        } else {
            return base::value;
        }
    }
    [[nodiscard]] OPTION_PURE constexpr const T& get_unchecked() const& noexcept {
        if constexpr (std::is_reference_v<T>) {
            OPTION_VERIFY(base::value != nullptr, "Dereferencing null pointer");
            return *base::value;
        } else {
            return base::value;
        }
    }
    [[nodiscard]] OPTION_PURE constexpr T&& get_unchecked() && noexcept {
        if constexpr (std::is_reference_v<T>) {
            OPTION_VERIFY(base::value != nullptr, "Dereferencing null pointer");
            return static_cast<T&&>(*base::value);
        } else {
            return static_cast<T&&>(base::value);
        }
    }
    [[nodiscard]] OPTION_PURE constexpr const T&& get_unchecked() const&& noexcept {
        if constexpr (std::is_reference_v<T>) {
            OPTION_VERIFY(base::value != nullptr, "Dereferencing null pointer");
            return static_cast<const T&&>(*base::value);
        } else {
            return static_cast<const T&&>(base::value);
        }
    }

    [[nodiscard]] constexpr T& value_or_throw() & OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(*this); }
    [[nodiscard]] constexpr const T& value_or_throw() const& OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(*this); }
    [[nodiscard]] constexpr T&& value_or_throw() && OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(static_cast<option&&>(*this)); }
    [[nodiscard]] constexpr const T&& value_or_throw() const&& OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(static_cast<const option&&>(*this)); }

    [[nodiscard]] constexpr T& value() & OPTION_LIFETIMEBOUND { return value_or_throw(); }
    [[nodiscard]] constexpr const T& value() const& OPTION_LIFETIMEBOUND { return value_or_throw(); }
    [[nodiscard]] constexpr T&& value() && OPTION_LIFETIMEBOUND { return static_cast<T&&>(value_or_throw()); }
    [[nodiscard]] constexpr const T&& value() const&& OPTION_LIFETIMEBOUND { return static_cast<const T&&>(value_or_throw()); }

    template<class U = std::remove_cv_t<T>>
    [[nodiscard]] constexpr std::remove_cv_t<T> value_or(U&& default_value) const& {
        if (has_value()) {
            return get();
        } else {
            return static_cast<std::remove_cv_t<T>>(static_cast<U&&>(default_value));
        }
    }
    template<class U = std::remove_cv_t<T>>
    [[nodiscard]] constexpr std::remove_cv_t<T> value_or(U&& default_value) && {
        if (has_value()) {
            return static_cast<std::remove_cv_t<T>&&>(get());
        } else {
            return static_cast<std::remove_cv_t<T>>(static_cast<U&&>(default_value));
        }
    }

    template<class... Args>
    [[nodiscard]] constexpr std::remove_cv_t<T> value_or_construct(Args&&... args) const& { return impl::option::value_or_construct<std::remove_cv_t<T>>(*this, static_cast<Args&&>(args)...); }
    template<class... Args>
    [[nodiscard]] constexpr std::remove_cv_t<T> value_or_construct(Args&&... args) && { return impl::option::value_or_construct<std::remove_cv_t<T>>(static_cast<option&&>(*this), static_cast<Args&&>(args)...); }
    template<class U, class... Args>
    [[nodiscard]] constexpr std::remove_cv_t<T> value_or_construct(std::initializer_list<U> ilist, Args&&... args) const& { return impl::option::value_or_construct<std::remove_cv_t<T>>(*this, ilist, static_cast<Args&&>(args)...); }
    template<class U, class... Args>
    [[nodiscard]] constexpr std::remove_cv_t<T> value_or_construct(std::initializer_list<U> ilist, Args&&... args) && { return impl::option::value_or_construct<std::remove_cv_t<T>>(static_cast<option&&>(*this), ilist, static_cast<Args&&>(args)...); }

    template<class U, class F>
    [[nodiscard]] constexpr auto map_or(U&& def, F&& f) & { return impl::option::map_or<T>(*this, static_cast<U&&>(def), static_cast<F&&>(f)); }
    template<class U, class F>
    [[nodiscard]] constexpr auto map_or(U&& def, F&& f) const& { return impl::option::map_or<T>(*this, static_cast<U&&>(def), static_cast<F&&>(f)); }
    template<class U, class F>
    [[nodiscard]] constexpr auto map_or(U&& def, F&& f) && { return impl::option::map_or<T>(static_cast<option&&>(*this), static_cast<U&&>(def), static_cast<F&&>(f)); }
    template<class U, class F>
    [[nodiscard]] constexpr auto map_or(U&& def, F&& f) const&& { return impl::option::map_or<T>(static_cast<const option&&>(*this), static_cast<U&&>(def), static_cast<F&&>(f)); }

    template<class D, class F>
    [[nodiscard]] constexpr auto map_or_else(D&& def, F&& f) & { return impl::option::map_or_else<T>(*this, static_cast<D&&>(def), static_cast<F&&>(f)); }
    template<class D, class F>
    [[nodiscard]] constexpr auto map_or_else(D&& def, F&& f) const& { return impl::option::map_or_else<T>(*this, static_cast<D&&>(def), static_cast<F&&>(f)); }
    template<class D, class F>
    [[nodiscard]] constexpr auto map_or_else(D&& def, F&& f) && { return impl::option::map_or_else<T>(static_cast<option&&>(*this), static_cast<D&&>(def), static_cast<F&&>(f)); }
    template<class D, class F>
    [[nodiscard]] constexpr auto map_or_else(D&& def, F&& f) const&& { return impl::option::map_or_else<T>(static_cast<const option&&>(*this), static_cast<D&&>(def), static_cast<F&&>(f)); }

    [[nodiscard]] OPTION_PURE constexpr std::remove_reference_t<T>* ptr_or_null() noexcept OPTION_LIFETIMEBOUND {
        return has_value() ? OPTION_ADDRESSOF(get()) : nullptr;
    }
    [[nodiscard]] OPTION_PURE constexpr const std::remove_reference_t<T>* ptr_or_null() const noexcept OPTION_LIFETIMEBOUND {
        return has_value() ? OPTION_ADDRESSOF(get()) : nullptr;
    }

    template<class F>
    [[nodiscard]] constexpr option filter(F&& f) & { return impl::option::filter(*this, static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr option filter(F&& f) const& { return impl::option::filter(*this, static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr option filter(F&& f) && { return impl::option::filter(static_cast<option&&>(*this), static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr option filter(F&& f) const&& { return impl::option::filter(static_cast<const option&&>(*this), static_cast<F&&>(f)); }

    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) & { return impl::option::and_then(*this, static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) const& { return impl::option::and_then(*this, static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) && { return impl::option::and_then(static_cast<option&&>(*this), static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) const&& { return impl::option::and_then(static_cast<const option&&>(*this), static_cast<F&&>(f)); }

    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) & { return impl::option::map<T>(*this, static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) const& { return impl::option::map<T>(*this, static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) && { return impl::option::map<T>(static_cast<option&&>(*this), static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) const&& { return impl::option::map<T>(static_cast<const option&&>(*this), static_cast<F&&>(f)); }

    template<class F>
    [[nodiscard]] constexpr option or_else(F&& f) const& { return impl::option::or_else<T>(*this, static_cast<F&&>(f)); }
    template<class F>
    [[nodiscard]] constexpr option or_else(F&& f) && { return impl::option::or_else<T>(static_cast<option&&>(*this), static_cast<F&&>(f)); }

    template<class U>
    constexpr void swap(option<U>& other) noexcept(impl::option::nothrow_swap<T, U>) {
        using std::swap;
        if (!has_value() && !other.has_value()) {
            return;
        }
        if (has_value() && other.has_value()) {
            swap(base::value, other.base::value);
            return;
        }
        if (has_value()) {
            other.base::construct(static_cast<T&&>(get()));
            base::reset();
            return;
        }
        base::construct(static_cast<U&&>(other.get()));
        other.base::reset();
    }
};

template<class T>
option(T) -> option<T>;

namespace impl {
#if OPTION_USE_BUILTIN_TRAITS
    template<class T>
    struct internal_option_traits<opt::option<T>, option_strategy::avaliable_option> {
        using base = typename opt::option<T>::base;
        using traits = opt::option_traits<T>;

        static constexpr std::uintmax_t max_level = traits::max_level - 1;

        static std::uintmax_t get_level(const opt::option<T>* const value) noexcept {
            return traits::get_level(OPTION_ADDRESSOF(static_cast<const base*>(value)->value)) - 1;
        }

        static void set_level(opt::option<T>* const value, const std::uintmax_t level) noexcept {
            traits::set_level(OPTION_ADDRESSOF(static_cast<base*>(value)->value), level + 1);
        }
    };
    template<class T>
    struct internal_option_traits<opt::option<T>, option_strategy::unavaliable_option> {
        using base = typename opt::option<T>::base;
        using bool_traits = opt::option_traits<bool>;

        static constexpr std::uintmax_t max_level = bool_traits::max_level;

        static std::uintmax_t get_level(const opt::option<T>* const value) noexcept {
            return bool_traits::get_level(OPTION_ADDRESSOF(static_cast<const base*>(value)->has_value_flag));
        }
        static void set_level(opt::option<T>* const value, const std::uintmax_t level) noexcept {
            bool_traits::set_level(OPTION_ADDRESSOF(static_cast<base*>(value)->has_value_flag), level);
        }
    };
    template<class T>
    struct internal_option_traits<opt::option<T>, option_strategy::reference_option> {
        using base = typename opt::option<T>::base;

        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const opt::option<T>* const value) noexcept {
            const auto uint = reinterpret_cast<std::uintptr_t>(static_cast<const base*>(value)->value);
            return std::uintmax_t(uint) - 1;
        }
        static void set_level(opt::option<T>* const value, const std::uintmax_t level) noexcept {
            const auto uint = reinterpret_cast<std::remove_reference_t<T>*>(std::uintptr_t(level + 1));
            static_cast<base*>(value)->value = uint;
        }
    };
#endif // OPTION_USE_BUILTIN_TRAITS
}

template<class T>
OPTION_RETURN_TYPESTATE(consumed)
[[nodiscard]] constexpr option<std::decay_t<T>> make_option(T&& value) {
    return option<std::decay_t<T>>{static_cast<T&&>(value)};
}
template<class T, class... Args>
OPTION_RETURN_TYPESTATE(consumed)
[[nodiscard]] constexpr option<T> make_option(Args&&... args) {
    return option<T>{std::in_place, static_cast<Args&&>(args)...};
}
template<class T, class U, class... Args>
OPTION_RETURN_TYPESTATE(consumed)
[[nodiscard]] constexpr option<T> make_option(std::initializer_list<U> ilist, Args&&... args) {
    return option<T>{std::in_place, ilist, static_cast<Args&&>(args)...};
}

template<class... Options, std::enable_if_t<std::conjunction_v<opt::is_option<impl::remove_cvref<Options>>...>, int> = 0>
[[nodiscard]] constexpr auto zip(Options&&... options)
    -> opt::option<std::tuple<typename impl::remove_cvref<Options>::value_type...>>
{
    using result_tuple = std::tuple<typename impl::remove_cvref<Options>::value_type...>;
    if ((options.has_value() && ...)) {
        return opt::option{result_tuple{static_cast<Options&&>(options).get()...}};
    } else {
        return {};
    }
}

template<class Fn, class... Options, std::enable_if_t<std::conjunction_v<opt::is_option<impl::remove_cvref<Options>>...>, int> = 0>
[[nodiscard]] constexpr auto zip_with(Fn&& fn, Options&&... options)
{
    using fn_result = decltype(impl::invoke(static_cast<Fn&&>(fn), static_cast<Options&&>(options).get()...));
    if constexpr (std::is_void_v<fn_result>) {
        if ((options.has_value() && ...)) {
            impl::invoke(static_cast<Fn&&>(fn), static_cast<Options&&>(options).get()...);
        }
        return void();
    } else {
        if ((options.has_value() && ...)) {
            return opt::option<fn_result>{impl::invoke(static_cast<Fn&&>(fn), static_cast<Options&&>(options).get()...)};
        } else {
            return opt::option<fn_result>{opt::none};
        }
    }
}

template<class To, class From>
[[nodiscard]] constexpr opt::option<To> option_cast(const opt::option<From>& value) {
    return value.map([](const From& x) -> To { return To(x); });
}
template<class To, class From>
[[nodiscard]] constexpr opt::option<To> option_cast(opt::option<From>&& value) {
    return static_cast<opt::option<From>&&>(value).map([](From&& x) -> To { return To(static_cast<From&&>(x)); });
}

template<class T>
[[nodiscard]] constexpr opt::option<T&> from_nullable(T* const nullable_ptr) noexcept {
    if (nullable_ptr == nullptr) {
        return {};
    } else {
        return {*nullable_ptr};
    }
}

template<class T>
[[nodiscard]] constexpr opt::option<T> as_option(const T& value) noexcept {
    static_assert(std::is_trivially_copyable_v<T> && opt::option_traits<T>::max_level > 0);

    opt::option<T> result;
    result.get_unchecked() = value;
    return result;
}

template<class T, class U>
constexpr impl::option::enable_swap<T, U> swap(option<T>& left, option<U>& right) noexcept(impl::option::nothrow_swap<T, U>) {
    left.swap(right);
}

template<class Option>
[[nodiscard]] constexpr auto flatten(Option&& opt) {
    if constexpr (!opt::is_option_v<typename impl::remove_cvref<Option>::value_type>) {
        using result_type = impl::copy_lvalue_reference_t<typename impl::remove_cvref<Option>::value_type, Option>;
        return opt.has_value() ? opt::option<result_type>{static_cast<Option&&>(opt).get()} : opt::none;
    } else {
        return opt.has_value() ? flatten(static_cast<Option&&>(opt).get()) : opt::none;
    }
}

namespace impl {
    template<class TupleLikeType, class Self, std::size_t... Idx>
    constexpr auto unzip_impl(Self&& self, std::index_sequence<Idx...>) {
        using tuple_like_of_options = impl::tuple_like_of_options_t<TupleLikeType>;
        if (self.has_value()) {
            return tuple_like_of_options{
                opt::option<std::tuple_element_t<Idx, TupleLikeType>>{std::get<Idx>(static_cast<Self&&>(self).get())}...
            };
        } else {
            return tuple_like_of_options{
                opt::option<std::tuple_element_t<Idx, TupleLikeType>>{}...
            };
        }
    }
}

template<class Self>
[[nodiscard]] constexpr auto unzip(Self&& self) {
    using tuple_like_type = impl::remove_cvref<typename impl::remove_cvref<Self>::value_type>;
    return impl::unzip_impl<tuple_like_type>(
        static_cast<Self&&>(self),
        std::make_index_sequence<std::tuple_size<tuple_like_type>::value>{}
    );
}

namespace impl {
    template<std::size_t I, class Self>
    constexpr auto get_impl(Self&& self) noexcept {
        using std::get;
        using type = impl::copy_reference_t<decltype(get<I>(static_cast<Self&&>(self).get())), Self>;

        if (self.has_value()) {
            return opt::option<type>{static_cast<type>(get<I>(static_cast<Self&&>(self).get()))};
        }
        return opt::option<type>{opt::none};
    }
    template<class T, class Self>
    constexpr auto get_impl(Self&& self) noexcept {
        using std::get;
        using type = impl::copy_reference_t<decltype(get<T>(static_cast<Self&&>(self).get())), Self>;

        if (self.has_value()) {
            return opt::option<type>{static_cast<type>(get<T>(static_cast<Self&&>(self).get()))};
        }
        return opt::option<type>{opt::none};
    }
}

template<std::size_t I, class T>
[[nodiscard]] constexpr auto get(opt::option<T>& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<I>(x); }
template<std::size_t I, class T>
[[nodiscard]] constexpr auto get(const opt::option<T>& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<I>(x); }
template<std::size_t I, class T>
[[nodiscard]] constexpr auto get(opt::option<T>&& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<I>(static_cast<opt::option<T>&&>(x)); }
template<std::size_t I, class T>
[[nodiscard]] constexpr auto get(const opt::option<T>&& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<I>(static_cast<const opt::option<T>&&>(x)); }

template<class T, class OptT>
[[nodiscard]] constexpr auto get(opt::option<OptT>& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<T>(x); }
template<class T, class OptT>
[[nodiscard]] constexpr auto get(const opt::option<OptT>& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<T>(x); }
template<class T, class OptT>
[[nodiscard]] constexpr auto get(opt::option<OptT>&& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<T>(static_cast<opt::option<T>&&>(x)); }
template<class T, class OptT>
[[nodiscard]] constexpr auto get(const opt::option<OptT>&& x OPTION_LIFETIMEBOUND) noexcept { return impl::get_impl<T>(static_cast<const opt::option<T>&&>(x)); }

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
[[nodiscard]] OPTION_PURE constexpr auto get(std::variant<Ts...>&& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<I>(static_cast<std::variant<Ts...>&&>(v)); }
template<std::size_t I, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(const std::variant<Ts...>&& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<I>(static_cast<const std::variant<Ts...>&&>(v)); }

template<class T, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(std::variant<Ts...>& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<T>(v); }
template<class T, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(const std::variant<Ts...>& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<T>(v); }
template<class T, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(std::variant<Ts...>&& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<T>(static_cast<std::variant<Ts...>&&>(v)); }
template<class T, class... Ts>
[[nodiscard]] OPTION_PURE constexpr auto get(const std::variant<Ts...>&& v OPTION_LIFETIMEBOUND) noexcept { return impl::variant_get<T>(static_cast<const std::variant<Ts...>&&>(v)); }

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
    using type = impl::copy_reference_t<decltype(static_cast<T&&>(container)[index]), T>;

    if (index >= container.size()) { return opt::option<type>{opt::none}; }
    return opt::option<type>{static_cast<type>(static_cast<T&&>(container)[index])};
}

template<class T>
[[nodiscard]] OPTION_PURE constexpr auto at_front(T&& container OPTION_LIFETIMEBOUND) noexcept {
    using type = impl::copy_reference_t<decltype(static_cast<T&&>(container).front()), T>;
    if (container.size() == 0) {
        return opt::option<type>{};
    }
    return opt::option<type>{static_cast<type>(static_cast<T&&>(container).front())};
}

template<class T>
[[nodiscard]] OPTION_PURE constexpr auto at_back(T&& container OPTION_LIFETIMEBOUND) noexcept {
    using type = impl::copy_reference_t<decltype(static_cast<T&&>(container).back()), T>;
    if (container.size() == 0) {
        return opt::option<type>{};
    }
    return opt::option<type>{static_cast<type>(static_cast<T&&>(container).back())};
}

namespace impl {
    template<class T, class = void>
    inline constexpr bool has_mapped_type = false;
    template<class T>
    inline constexpr bool has_mapped_type<T, std::void_t<typename T::mapped_type>> = true;
}

template<class T, class K>
[[nodiscard]] constexpr auto lookup(T& associative_container OPTION_LIFETIMEBOUND, K&& key) {
    auto it = associative_container.find(static_cast<K&&>(key));
    if constexpr (impl::has_mapped_type<impl::remove_cvref<T>>) {
        using type = decltype((it->second));
        if (it == associative_container.end()) {
            return opt::option<type>{};
        }
        return opt::option<type>{it->second};
    } else {
        using type = decltype(*it);
        if (it == associative_container.end()) {
            return opt::option<type>{};
        }
        return opt::option<type>{*it};
    }
}

template<class T, class U, std::enable_if_t<!opt::is_option_v<impl::remove_cvref<U>>, int> = 0>
[[nodiscard]] constexpr std::remove_cv_t<T> operator|(const opt::option<T>& left, U&& right) {
    return left.value_or(static_cast<U&&>(right));
}
template<class T, class U, std::enable_if_t<!opt::is_option_v<impl::remove_cvref<U>>, int> = 0>
[[nodiscard]] constexpr std::remove_cv_t<T> operator|(opt::option<T>&& left, U&& right) {
    return static_cast<opt::option<T>&&>(left).value_or(static_cast<U&&>(right));
}

template<class T>
[[nodiscard]] constexpr opt::option<T> operator|(const opt::option<T>& left, const opt::option<T>& right) {
    if (left.has_value()) {
        return left;
    } else {
        return right;
    }
}
template<class T>
[[nodiscard]] constexpr opt::option<T> operator|(opt::option<T>&& left, const opt::option<T>& right) {
    if (left.has_value()) {
        return static_cast<opt::option<T>&&>(left);
    } else {
        return right;
    }
}
template<class T>
[[nodiscard]] constexpr opt::option<T> operator|(const opt::option<T>& left, opt::option<T>&& right) {
    if (left.has_value()) {
        return left;
    } else {
        return static_cast<opt::option<T>&&>(right);
    }
}
template<class T>
[[nodiscard]] constexpr opt::option<T> operator|(opt::option<T>&& left, opt::option<T>&& right) {
    if (left.has_value()) {
        return static_cast<opt::option<T>&&>(left);
    } else {
        return static_cast<opt::option<T>&&>(right);
    }
}

template<class T, class U>
constexpr opt::option<T>& operator|=(opt::option<T>& left, U&& right) {
    if (!left.has_value()) {
        left = static_cast<U&&>(right);
    }
    return left;
}

template<class T, class U>
[[nodiscard]] constexpr opt::option<U> operator&(const opt::option<T>& left, const opt::option<U>& right) {
    if (left.has_value()) {
        return right;
    }
    return opt::none;
}
template<class T, class U>
[[nodiscard]] constexpr opt::option<U> operator&(const opt::option<T>& left, opt::option<U>&& right) {
    if (left.has_value()) {
        return static_cast<opt::option<U>&&>(right);
    }
    return opt::none;
}

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
template<class T>
[[nodiscard]] constexpr opt::option<T> operator^(opt::option<T>&& left, const opt::option<T>& right) {
    if (left.has_value() && !right.has_value()) {
        return static_cast<opt::option<T>&&>(left);
    }
    if (!left.has_value() && right.has_value()) {
        return right;
    }
    return opt::none;
}
template<class T>
[[nodiscard]] constexpr opt::option<T> operator^(const opt::option<T>& left, opt::option<T>&& right) {
    if (left.has_value() && !right.has_value()) {
        return left;
    }
    if (!left.has_value() && right.has_value()) {
        return static_cast<opt::option<T>&&>(right);
    }
    return opt::none;
}
template<class T>
[[nodiscard]] constexpr opt::option<T> operator^(opt::option<T>&& left, opt::option<T>&& right) {
    if (left.has_value() && !right.has_value()) {
        return static_cast<opt::option<T>&&>(left);
    }
    if (!left.has_value() && right.has_value()) {
        return static_cast<opt::option<T>&&>(right);
    }
    return opt::none;
}

namespace impl {
    template<class T>
    [[nodiscard]] T fake_copy(T) noexcept;
}

template<class T1, class T2>
[[nodiscard]] constexpr auto operator==(const option<T1>& left, const option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left.get() == right.get())) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() == right.get(); }
    return left_has_value == right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator!=(const option<T1>& left, const option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left.get() != right.get())) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() != right.get(); }
    return left_has_value != right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator<(const option<T1>& left, const option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left.get() < right.get())) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() < right.get(); }
    return left_has_value < right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator<=(const option<T1>& left, const option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left.get() <= right.get())) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() <= right.get(); }
    return left_has_value <= right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator>(const option<T1>& left, const option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left.get() > right.get())) {
    const bool left_has_value = left.has_value();
    const bool right_has_value = right.has_value();
    if (left_has_value && right_has_value) { return left.get() > right.get(); }
    return left_has_value > right_has_value;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator>=(const option<T1>& left, const option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left.get() >= right.get())) {
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
[[nodiscard]] constexpr auto operator==(const option<T1>& left, const T2& right)
    -> decltype(impl::fake_copy<bool>(left.get() == right)) {
    return left.has_value() ? left.get() == right : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator==(const T1& left, const opt::option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left == right.get())) {
    return right.has_value() ? left == right.get() : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator!=(const option<T1>& left, const T2& right)
    -> decltype(impl::fake_copy<bool>(left.get() != right)) {
    return left.has_value() ? left.get() != right : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator!=(const T1& left, const opt::option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left != right.get())) {
    return right.has_value() ? left != right.get() : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator<(const option<T1>& left, const T2& right)
    -> decltype(impl::fake_copy<bool>(left.get() < right)) {
    return left.has_value() ? left.get() < right : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator<(const T1& left, const opt::option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left < right.get())) {
    return right.has_value() ? left < right.get() : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator<=(const option<T1>& left, const T2& right)
    -> decltype(impl::fake_copy<bool>(left.get() <= right)) {
    return left.has_value() ? left.get() <= right : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator<=(const T1& left, const opt::option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left <= right.get())) {
    return right.has_value() ? left <= right.get() : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator>(const option<T1>& left, const T2& right)
    -> decltype(impl::fake_copy<bool>(left.get() > right)) {
    return left.has_value() ? left.get() > right : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator>(const T1& left, const opt::option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left > right.get())) {
    return right.has_value() ? left > right.get() : true;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator>=(const option<T1>& left, const T2& right)
    -> decltype(impl::fake_copy<bool>(left.get() >= right)) {
    return left.has_value() ? left.get() >= right : false;
}
template<class T1, class T2>
[[nodiscard]] constexpr auto operator>=(const T1& left, const opt::option<T2>& right)
    -> decltype(impl::fake_copy<bool>(left >= right.get())) {
    return right.has_value() ? left >= right.get() : true;
}

namespace impl {
    template<class T>
    struct type_wrapper {
        T m{};

        template<class... Args, std::enable_if_t<impl::is_initializable_from_v<T, Args...>, int> = 0>
        constexpr explicit type_wrapper(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : m{static_cast<Args&&>(args)...} {}

        type_wrapper() = default;
        type_wrapper(const type_wrapper&) = default;
        type_wrapper(type_wrapper&&) = default;
        type_wrapper& operator=(const type_wrapper&) = default;
        type_wrapper& operator=(type_wrapper&&) = default;

        constexpr type_wrapper(const T& x) noexcept(std::is_nothrow_copy_constructible_v<T>)
            : m{x} {}
        constexpr type_wrapper(T&& x) noexcept(std::is_nothrow_move_constructible_v<T>)
            : m{static_cast<T&&>(x)} {}

        template<class U = T>
        constexpr type_wrapper& operator=(U&& x) {
            m = static_cast<U&&>(x);
            return *this;
        }

        constexpr operator T&() & noexcept { return m; }
        constexpr operator const T&() const& noexcept { return m; }
        constexpr operator T&&() && noexcept { return static_cast<T&&>(m); }
        constexpr operator const T&&() const&& noexcept { return static_cast<const T&&>(m); }
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
        return traits::get_level(OPTION_ADDRESSOF(static_cast<const T&>(*value).*MemberPtr));
    }
    static constexpr void set_level(value_t* const value, const std::uintmax_t level) noexcept {
        traits::set_level(OPTION_ADDRESSOF(static_cast<T&>(*value).*MemberPtr), level);
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
        return traits::get_level(OPTION_ADDRESSOF(static_cast<const T&>(*value)));
    }
    static constexpr void set_level(enforce<T>* const value, const std::uintmax_t level) noexcept {
        traits::set_level(OPTION_ADDRESSOF(static_cast<T&>(*value)), level);
    }
};

namespace impl {
    template<class T, class>
    using enable_hash_helper1 = T;

    template<class T, class Hash = std::hash<T>>
    using enable_hash_helper2 = std::enable_if_t<
        and_v<
            std::is_default_constructible<Hash>,
            std::is_copy_constructible<Hash>,
            std::is_move_constructible<Hash>,
            std::is_destructible<Hash>,
            std::is_invocable_r<std::size_t, Hash, const T&>
        >
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

#if OPTION_CLANG && OPTION_CONSUMED_ANNOTATION_CHECKING
    #pragma clang diagnostic pop
#endif
