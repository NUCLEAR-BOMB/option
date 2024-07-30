#pragma once

#include <type_traits>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <new>
#include <functional>
#include <exception>
#include <cstring>
#include <tuple>
#include <array>
#include <limits>
#include <string_view>
#include <string>
#include <vector>

#ifdef __has_builtin
    #if __has_builtin(__builtin_unreachable)
        #define OPTION_UNREACHABLE() __builtin_unreachable()
    #endif
#endif
#ifndef OPTION_UNREACHABLE
        #if defined(__GNUC__) || defined(__GNUG__)
            #define OPTION_UNREACHABLE() __builtin_unreachable()
        #elif defined(_MSC_VER)
            #define OPTION_UNREACHABLE() __assume(0)
        #else
            #define OPTION_UNREACHABLE() ((void)0)
        #endif
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

// Define to 1 to use only quiet NaN in `opt::option<T>`, where T is floating point type.
// Define to 0 to use signaling NaN if available, or use quiet NaN instead.
// Default is 0.
#ifndef OPTION_USE_QUIET_NAN
    #define OPTION_USE_QUIET_NAN 0
#endif

#ifndef OPTION_BOOST_PFR_FILE
    #define OPTION_BOOST_PFR_FILE <boost/pfr.hpp>
#endif

#ifdef OPTION_USE_BOOST_PFR
    #if OPTION_USE_BOOST_PFR
        #if __has_include(OPTION_BOOST_PFR_FILE)
            #include OPTION_BOOST_PFR_FILE
            #ifndef BOOST_PFR_NOT_SUPPORTED
                #define OPTION_HAS_BOOST_PFR
            #endif
        #else // !__has_include(OPTION_BOOST_PFR_FILE)
            #error "The 'boost.pfr' library was not found. Define the 'OPTION_BOOST_PFR_FILE' macro to specify a custom path to the 'boost.pfr' library header"
        #endif
    #endif
#else // !defined(OPTION_USE_BOOST_PFR)
    #if __has_include(OPTION_BOOST_PFR_FILE)
        #include OPTION_BOOST_PFR_FILE
        #ifndef BOOST_PFR_NOT_SUPPORTED
            #define OPTION_HAS_BOOST_PFR
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
    #ifdef OPTION_HAS_LIBASSERT
        #define OPTION_VERIFY(expression, message) LIBASSERT_ASSUME(expression, message)
    #else
        #ifdef __clang__
            #define OPTION_DEBUG_BREAK __builtin_debugtrap()
        #elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
            #define OPTION_DEBUG_BREAK __debugbreak()
        #elif defined(__GNUC__) || defined(__GNUG__)
            #define OPTION_DEBUG_BREAK __builtin_trap()
        #else
            #include <csignal>
            #if defined(SIGTRAP)
                #define OPTION_DEBUG_BREAK ::std::raise(SIGTRAP)
            #else
                #define OPTION_DEBUG_BREAK ::std::raise(SIGABRT)
            #endif
        #endif
        #ifndef NDEBUG
            // Print an error message and call a debug break if the expression is evaluated as false
            #include <cstdio>
            #define OPTION_VERIFY(expression, message) \
                ((expression) ? (void)0 : ( \
                    (void)std::fprintf(stderr, "%s:%d: assertion '%s' failed: %s\n", __FILE__, __LINE__, #expression, message), \
                    (void)OPTION_DEBUG_BREAK) \
                )
        #else
            // Disable assertation on 'Release' build config
            #define OPTION_VERIFY(expression, message) \
                if (expression) {} else { OPTION_UNREACHABLE(); } ((void)0)
        #endif
    #endif
#endif

namespace opt {

namespace impl {
    struct none_tag_ctor {};
}

struct none_t {
    constexpr explicit none_t(impl::none_tag_ctor) {}
};
// Used to indicate `opt::option` with an empty state.
// Same as `std::nullptr`
inline constexpr none_t none{impl::none_tag_ctor{}};

template<class T>
class option;

template<class T, class = void>
struct option_traits;

// Check if is a specialization of `opt::option`
template<class>
inline constexpr bool is_option = false;
template<class T>
inline constexpr bool is_option<opt::option<T>> = true;

namespace impl {
    template<class T, class... Args>
    constexpr void construct_at(T* ptr, Args&&... args) {
        if constexpr (std::is_trivially_copy_assignable_v<T>) {
            *ptr = T{std::forward<Args>(args)...};
        } else {
            ::new(static_cast<void*>(ptr)) T{std::forward<Args>(args)...};
        }
    }

    template<class T>
    constexpr void destroy_at(T* ptr) {
        ptr->~T();
    }

    template<class T, class = std::size_t>
    inline constexpr bool has_option_traits = false;
    template<class T>
    inline constexpr bool has_option_traits<T, decltype(sizeof(opt::option_traits<T>))> =
        opt::option_traits<T>::max_level >= 1;

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

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#elif defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wclass-memaccess"
    #pragma GCC diagnostic ignored "-Wuninitialized"
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        std::memcpy(dest, &src, sizeof(Dest));
#if defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
    }

    enum class option_strategy {
        none,
        other,
        bool_,
        avaliable_option,
        unavaliable_option,
        reference,
        pointer_64,
        pointer_32,
        pointer_16,
        float64_sNaN,
        float64_qNaN,
        float32_sNaN,
        float32_qNaN,
    };

    template<class T>
    constexpr option_strategy detemine_option_strategy() {
        using st = option_strategy;

        if constexpr (is_option<T>) {
            if constexpr (has_option_traits<typename T::value_type>) {
                return st::avaliable_option;
            } else {
                return st::unavaliable_option;
            }
        }
        if constexpr (std::is_reference_v<T>) {
            return st::reference;
        }
        if constexpr (std::is_pointer_v<T>) {
            if constexpr (sizeof(T) <= 1) {
                return st::none;
            }
            if constexpr (sizeof(T) <= 4) {
                return st::pointer_32;
            }
            if constexpr (sizeof(T) <= 2) {
                return st::pointer_16;
            }
            return st::pointer_64;
        }
        if constexpr (std::is_floating_point_v<T>) {
            using limits = std::numeric_limits<T>;
            if constexpr (!limits::is_iec559 || (!limits::has_signaling_NaN && !limits::has_quiet_NaN)) {
                return st::none;
            }
            if constexpr (sizeof(T) == 8) {
                return limits::has_signaling_NaN ? st::float64_sNaN : st::float64_qNaN;
            }
            if constexpr (sizeof(T) == 4) {
                return limits::has_signaling_NaN ? st::float32_sNaN : st::float32_qNaN;
            }
            return st::none;
        }

        return st::other;
    }

    template<class T, option_strategy strategy = detemine_option_strategy<T>()>
    struct internal_option_traits {
        static constexpr std::uintmax_t max_level = 0;
    };

    template<>
    struct internal_option_traits<bool, option_strategy::other> {
        static constexpr std::uintmax_t max_level = 254;
        using uint_bool = std::uint_least8_t;

        static std::uintmax_t get_level(const bool* const value) noexcept {
            const auto u8_value = impl::ptr_bit_cast<uint_bool>(value);
            return u8_value >= 2 ? u8_value - 2 : std::uintmax_t(-1);
        }
        static void set_level(bool* const value, const std::uintmax_t level) noexcept {
            impl::ptr_bit_copy(value, uint_bool(level + 2));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::reference> {
        using unref = std::remove_reference_t<T>;

        static constexpr std::uintmax_t max_level = 255;

        static std::uintmax_t get_level(const unref* const* const value) noexcept {
            const auto uptr = impl::ptr_bit_cast<std::uintptr_t>(value);
            return uptr <= max_level ? uptr : std::uintmax_t(-1);
        }
        static void set_level(unref** const value, const std::uintmax_t level) noexcept {
            impl::ptr_bit_copy(value, std::uintptr_t(level));
        }
    };

    template<class T>
    struct internal_option_traits<std::reference_wrapper<T>, option_strategy::other> {
        static_assert(sizeof(std::reference_wrapper<T>) == sizeof(T*), "Unsupported std::reference_wrapper implementation.");

        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const std::reference_wrapper<T>* const value) noexcept {
            const auto ptr = impl::ptr_bit_cast<std::uintptr_t>(value);
            return ptr <= max_level ? ptr : std::uintmax_t(-1);
        }
        static void set_level(std::reference_wrapper<T>* const value, const std::uintmax_t level) noexcept {
            impl::ptr_bit_copy(value, std::uintptr_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::pointer_64> {
    private:
        static constexpr std::uintptr_t ptr_offset = 0xF8E1B1825D5D6C67;
    public:
        static constexpr std::uintmax_t max_level = 512;

        static std::uintmax_t get_level(const T* const value) noexcept {
            auto uint = impl::ptr_bit_cast<std::uintptr_t>(value);
            uint -= ptr_offset;
            return uint <= max_level ? uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            impl::ptr_bit_copy(value, ptr_offset + std::uintptr_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::pointer_32> {
    private:
        static constexpr std::uintptr_t ptr_offset = 0xFFFF'FFFF - 31;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            auto uint = impl::ptr_bit_cast<std::uintptr_t>(value);
            uint -= (ptr_offset - max_level);
            return uint <= max_level ? max_level - uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            impl::ptr_bit_copy(value, ptr_offset - std::uintptr_t(level));
        }
    };
    template<class T>
    struct internal_option_traits<T, option_strategy::pointer_16> {
    private:
        static constexpr std::uintptr_t ptr_offset = 0xFFFF;
    public:
        static constexpr std::uintmax_t max_level = 256;

        static std::uintmax_t get_level(const T* const value) noexcept {
            auto uint = impl::ptr_bit_cast<std::uintptr_t>(value);
            uint -= (ptr_offset - max_level);
            return uint <= max_level ? max_level - uint : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
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
            const auto uint = impl::ptr_bit_cast<std::uint64_t>(value);
            return uint >= nan_start && uint <= nan_start + max_level ? uint - nan_start : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
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
            const auto uint = impl::ptr_bit_cast<std::uint64_t>(value);
            return uint >= nan_start && uint <= nan_start + max_level ? uint - nan_start : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
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
            const auto uint = impl::ptr_bit_cast<std::uint32_t>(value);
            return uint >= nan_start && uint <= nan_start + max_level ? uint - nan_start : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
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
            const auto uint = impl::ptr_bit_cast<std::uint32_t>(value);
            return uint >= nan_start && uint <= nan_start + max_level ? uint - nan_start : std::uintmax_t(-1);
        }
        static void set_level(T* const value, const std::uintmax_t level) noexcept {
            impl::ptr_bit_copy(value, nan_start + std::uint32_t(level));
        }
    };
}

template<class T, class>
struct option_traits : impl::internal_option_traits<T> {};

namespace impl {
    template<class T>
    using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

    // See https://github.com/microsoft/STL/pull/878#issuecomment-639696118
    struct nontrivial_dummy_t {
        constexpr nontrivial_dummy_t() noexcept {}
    };

    struct construct_from_invoke_tag {
        explicit construct_from_invoke_tag() = default;
    };

    template<class T, class Traits, class = std::uintmax_t>
    inline constexpr bool has_get_level_method = false;
    template<class T, class Traits>
    inline constexpr bool has_get_level_method<T, Traits, decltype(static_cast<std::uintmax_t>(Traits::get_level(std::declval<const T*>())))> = true;

    template<class T, class Traits, class = void>
    inline constexpr bool has_set_level_method = false;
    template<class T, class Traits>
    inline constexpr bool has_set_level_method<T, Traits, decltype(Traits::set_level(std::declval<T*>(), std::declval<std::uintmax_t>()))> = true;

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

    template<class T, class, class... Args>
    struct is_direct_list_initializable_impl : std::false_type {};
    template<class T, class... Args>
    struct is_direct_list_initializable_impl<T, std::void_t<decltype(T{std::declval<Args>()...})>, Args...> : std::true_type {};

    template<class T, class... Args>
    inline constexpr bool is_direct_list_initializable = is_direct_list_initializable_impl<T, void, Args...>::value;

    enum class base_strategy {
        has_traits             = 1,
        trivially_destructible = 2,
        empty_object           = 4,
    };

    constexpr base_strategy operator|(const base_strategy left, const base_strategy right) {
        using type = std::underlying_type_t<base_strategy>;
        return base_strategy(type(left) | type(right));
    }

    template<class T>
    constexpr base_strategy detemine_base_strategy() noexcept {
        using st = base_strategy;

        constexpr bool trivially_destructible = std::is_trivially_destructible_v<T>;
        constexpr bool is_empty = std::is_empty_v<T>;
        if constexpr (is_empty && trivially_destructible) {
            return st::empty_object | st::trivially_destructible;
        }
        if constexpr (is_empty && !trivially_destructible) {
            return st::empty_object;
        }
        constexpr bool has_traits = has_option_traits<T>;

        if constexpr (has_traits && trivially_destructible) {
            return st::has_traits | st::trivially_destructible;
        }
        if constexpr (has_traits && !trivially_destructible) {
            return st::has_traits;
        }
        if constexpr (trivially_destructible) {
            return st::trivially_destructible;
        }
        return st{};
    }

    template<class T,
        base_strategy strategy = detemine_base_strategy<T>()
    >
    struct option_destruct_base;

    template<class T>
    struct option_destruct_base<T, base_strategy::trivially_destructible> {
        union {
            nontrivial_dummy_t dummy;
            T value;
        };
        bool has_value_flag;

        constexpr option_destruct_base() noexcept
            : dummy{}, has_value_flag(false) {}

        template<class... Args>
        constexpr option_destruct_base(Args&&... args)
            : value{std::forward<Args>(args)...}, has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))}, has_value_flag(true) {}

        constexpr void reset() noexcept {
            has_value_flag = false;
        }
        constexpr bool has_value() const noexcept {
            return has_value_flag;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value(), "Value is non-empty");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            has_value_flag = true;
        }
    };
    template<class T>
    struct option_destruct_base<T, base_strategy{}> {
        union {
            nontrivial_dummy_t dummy;
            T value;
        };
        bool has_value_flag;

        constexpr option_destruct_base() noexcept
            : dummy(), has_value_flag(false) {}

        template<class... Args>
        constexpr option_destruct_base(Args&&... args)
            : value{std::forward<Args>(args)...}, has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))}, has_value_flag(true) {}

        ~option_destruct_base() noexcept(std::is_nothrow_destructible_v<T>) {
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
        constexpr bool has_value() const noexcept {
            return has_value_flag;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value(), "Value is non-empty");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            has_value_flag = true;
        }
    };
    template<class T>
    struct option_destruct_base<T, base_strategy::has_traits | base_strategy::trivially_destructible> {
        union {
            nontrivial_dummy_t dummy;
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
        constexpr option_destruct_base(Args&&... args)
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
        constexpr bool has_value() const noexcept {
            return traits::get_level(std::addressof(value)) == std::uintmax_t(-1);
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value(), "Value is non-empty");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
    };
    template<class T>
    struct option_destruct_base<T, base_strategy::has_traits> {
        union {
            nontrivial_dummy_t dummy;
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
        constexpr option_destruct_base(Args&&... args)
            : value{std::forward<Args>(args)...} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))} {
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }

        ~option_destruct_base() noexcept(std::is_nothrow_destructible_v<T>) {
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
        constexpr bool has_value() const noexcept {
            return traits::get_level(std::addressof(value)) == std::uintmax_t(-1);
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value(), "Value is non-empty");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            OPTION_VERIFY(has_value(), "After the construction, the value is in an empty state. Possibly because of the constructor arguments");
        }
    };
    template<class T>
    struct option_destruct_base<T, base_strategy::empty_object | base_strategy::trivially_destructible> {
        union {
            std::uint_least8_t dummy_flag;
            T value;
        };
        struct traits {
            static constexpr std::uint_least8_t empty_value = 0b1111'0101;
        };
        static constexpr std::uint_least8_t in_use_value = 0;
        
        constexpr option_destruct_base() noexcept
            : dummy_flag(traits::empty_value) {}

        template<class... Args>
        constexpr option_destruct_base(Args&&... args)
            : value{std::forward<Args>(args)...} {
            dummy_flag = in_use_value; // force compiler to not optimize away initialization
        }

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))} {
            dummy_flag = in_use_value; // force compiler to not optimize away initialization
        }

        constexpr void reset() noexcept {
            dummy_flag = traits::empty_value;
        }
        constexpr bool has_value() const noexcept {
            return dummy_flag != traits::empty_value;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value(), "Value is non-empty.");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            dummy_flag = in_use_value; // force compiler to not optimize away initialization
        }
    };
    template<class T>
    struct option_destruct_base<T, base_strategy::empty_object> {
        union {
            std::uint_least8_t dummy_flag;
            T value;
        };
        struct traits {
            static constexpr std::uint_least8_t empty_value = 0b1111'0101;
        };
        static constexpr std::uint_least8_t in_use_value = 0;
        
        constexpr option_destruct_base() noexcept
            : dummy_flag(traits::empty_value) {}

        template<class... Args>
        constexpr option_destruct_base(Args&&... args)
            : value{std::forward<Args>(args)...} {
            dummy_flag = in_use_value; // force compiler to not optimize away initialization
        }

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{std::invoke(std::forward<F>(f), std::forward<Arg>(arg))} {
            dummy_flag = in_use_value; // force compiler to not optimize away initialization
        }

        ~option_destruct_base() noexcept(std::is_nothrow_destructible_v<T>) {
            if (has_value()) {
                impl::destroy_at(std::addressof(value));
            }
        }

        constexpr void reset() noexcept {
            if (dummy_flag != traits::empty_value) {
                impl::destroy_at(std::addressof(value));
                dummy_flag = traits::empty_value;
            }
        }
        constexpr bool has_value() const noexcept {
            return dummy_flag != traits::empty_value;
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            OPTION_VERIFY(!has_value(), "Value is non-empty.");
            impl::construct_at(std::addressof(value), std::forward<Args>(args)...);
            dummy_flag = in_use_value; // force compiler to not optimize away initialization
        }
    };


    template<class T, bool is_reference /*false*/ = std::is_reference_v<T>>
    class option_storage_base : public option_destruct_base<T> {
        using base = option_destruct_base<T>;
    public:
        using base::base;
        using base::has_value;
        using base::reset;
        using base::construct;

        constexpr T& get() & noexcept { return base::value; }
        constexpr const T& get() const& noexcept { return base::value; }
        constexpr T&& get() && noexcept { return std::move(base::value); }
        constexpr const T&& get() const&& noexcept { return std::move(base::value); }

        // logic of assigning opt::option<T&> from a value
        template<class U>
        constexpr void assign_from_value(U&& other) {
            // TODO

            if (has_value()) {
                get() = std::forward<U>(other);
            } else {
                construct(std::forward<U>(other));
            }
        }
        // logic of assigning opt::option<T&> from another opt::option<U&>
        template<class Option>
        constexpr void assign_from_option(Option&& other) {
            if (other.has_value()) {
                if (has_value()) {
                    get() = std::forward<Option>(other).get();
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

        option_storage_base() = default;

        template<class Arg>
        constexpr option_storage_base(Arg&& arg) noexcept
            : value{ref_to_ptr(std::forward<Arg>(arg))} {}

        template<class F, class Arg>
        constexpr option_storage_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value{ref_to_ptr(std::invoke(std::forward<F>(f), std::forward<Arg>(arg)))} {}

        constexpr bool has_value() const noexcept {
            return value != nullptr;
        }
        constexpr void reset() noexcept {
            value = nullptr;
        }

        constexpr T& get() const& noexcept { return *value; }
        constexpr T&& get() const&& noexcept { return std::move(*value); }

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
        bool /*true*/ = (
            std::is_trivially_copy_assignable_v<T> &&
            std::is_trivially_copy_constructible_v<T> &&
            std::is_trivially_destructible_v<T>
            ) || std::is_reference_v<T>,
        bool = (std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>) || std::is_reference_v<T>
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
        bool /*true*/ = (
            std::is_trivially_move_assignable_v<T> &&
            std::is_trivially_move_constructible_v<T> &&
            std::is_trivially_destructible_v<T>
            ) || std::is_reference_v<T>,
        bool = (std::is_move_constructible_v<T> && std::is_move_assignable_v<T>) || std::is_reference_v<T>
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
    inline constexpr bool is_cv_bool = std::is_same_v<std::remove_cv_t<T>, bool>;
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
    template<class T, class U, bool is_explicit>
    using enable_constructor_5 = std::enable_if_t<
           std::is_constructible_v<T, U&&> && !std::is_same_v<impl::remove_cvref<U>, opt::option<T>>
        && !(std::is_same_v<impl::remove_cvref<T>, bool> && opt::is_option<impl::remove_cvref<U>>)
        && (std::is_convertible_v<U&&, T> == !is_explicit) // explicit( condition )
    , int>;

    template<class T, class First, class... Args>
    using enable_constructor_6 = std::enable_if_t<
        (std::is_constructible_v<T, First, Args...> || is_direct_list_initializable<T, First, Args...>)
        && !std::is_same_v<impl::remove_cvref<First>, opt::option<T>>
    , int>;

    template<class T, class U>
    using enable_assigment_operator_4 = std::enable_if_t<
           !opt::is_option<U>
        && (!std::is_scalar_v<T> || !std::is_same_v<T, std::decay_t<U>>)
        && std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>
    , int>;

    template<class T, class U, class UOpt = opt::option<U>>
    inline constexpr bool is_constructible_from_option =
        std::is_constructible_v<T, UOpt&> ||
        std::is_constructible_v<T, const UOpt&> ||
        std::is_constructible_v<T, UOpt&&> ||
        std::is_constructible_v<T, const UOpt&&> ||
        std::is_convertible_v<UOpt&, T> ||
        std::is_convertible_v<const UOpt&, T> ||
        std::is_convertible_v<UOpt&&, T> ||
        std::is_convertible_v<const UOpt&&, T>;

    template<class T, class U, class UOpt = opt::option<U>>
    inline constexpr bool is_assignable_from_option =
        std::is_assignable_v<T&, UOpt&> ||
        std::is_assignable_v<T&, const UOpt&> ||
        std::is_assignable_v<T&, UOpt&&> ||
        std::is_assignable_v<T&, const UOpt&&>;

    template<class T, class U, bool is_explicit>
    using enable_constructor_8 = std::enable_if_t<
           std::is_constructible_v<T, const U&>
        && (!is_cv_bool<T> && !is_constructible_from_option<T, U>)
        && (std::is_convertible_v<const U&, T> == !is_explicit)
    , int>;

    template<class T, class U, bool is_explicit>
    using enable_constructor_9 = std::enable_if_t<
           std::is_convertible_v<T, U&&>
        && (!is_cv_bool<T> && !is_constructible_from_option<T, U>)
        && (std::is_convertible_v<U&&, T> == !is_explicit)
    , int>;

    template<class T, class U>
    using enable_assigment_operator_5 = std::enable_if_t<
        (!is_constructible_from_option<T, U> && !is_assignable_from_option<T, U>)
        && ((std::is_constructible_v<T, const U&> && std::is_assignable_v<T&, const U&>)
            || std::is_reference_v<T>)
    , int>;

    template<class T, class U>
    using enable_assigment_operator_6 = std::enable_if_t<
        (!is_constructible_from_option<T, U> && !is_assignable_from_option<T, U>)
        && ((std::is_constructible_v<T, U&&> && std::is_assignable_v<T&, U&&>)
            || std::is_reference_v<T>)
    , int>;

    template<class T>
    inline constexpr bool nothrow_assigment_operator_2 =
        std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>
        && std::is_nothrow_destructible_v<T>;

    template<class T>
    inline constexpr bool nothrow_assigment_operator_3 =
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>
        && std::is_nothrow_destructible_v<T>;

    template<class T, class U>
    inline constexpr bool nothrow_assigment_operator_4 =
        std::is_nothrow_assignable_v<T&, U&&> && std::is_nothrow_constructible_v<T, U&&>
        && std::is_nothrow_destructible_v<T>;


    // implementation of opt::option<T>::and_then(F&&)
    template<class Self, class F>
    constexpr auto and_then(Self&& self, F&& f) {
        using invoke_res = impl::remove_cvref<std::invoke_result_t<F, decltype(*std::forward<Self>(self))>>;
        static_assert(opt::is_option<invoke_res>, "The return type of function F must be a specialization of opt::option");
        if (self.has_value()) {
            return std::invoke(std::forward<F>(f), *std::forward<Self>(self));
        } else {
            return impl::remove_cvref<invoke_res>{opt::none};
        }
    }
    template<class T, class Fn>
    using enable_and_then = std::enable_if_t<std::is_invocable_v<Fn, T>, int>;

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
    template<class T, class Fn>
    using enable_map = std::enable_if_t<std::is_invocable_v<Fn, T>, int>;

    // implementation of opt::option<T>::map_or(U&&, F&&)
    template<class T, class Self, class U, class F>
    constexpr impl::remove_cvref<U> map_or(Self&& self, U&& default_value, F&& f) {
        if (self.has_value()) {
            return std::invoke(std::forward<F>(f), std::forward<Self>(self).get());
        }
        return std::forward<U>(default_value);
    }
    template<class T, class U, class Fn>
    using enable_map_or = std::enable_if_t<std::is_invocable_r_v<impl::remove_cvref<U>, Fn, T>, int>;

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
    template<class T, class D, class Fn>
    using enable_map_or_else = std::enable_if_t<std::is_invocable_v<D> && std::is_invocable_v<Fn, T>, int>;

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
    template<class Fn>
    using enable_or_else = std::enable_if_t<std::is_invocable_v<Fn>, int>;

    // implementation of opt::option<T>::value_or_throw()
    template<class Self>
    constexpr auto&& value_or_throw(Self&& self) {
        if (!self.has_value()) { throw_bad_access(); }
        return *std::forward<Self>(self);
    }

    template<class Self>
    constexpr auto flatten(Self&& self) {
        using pure_self = impl::remove_cvref<Self>;
        // this is for a nice error message if Self is not an opt::option<opt::option<T>>
        constexpr bool is_option_option = opt::is_option<typename pure_self::value_type>;
        if constexpr (is_option_option) {
            if (self.has_value() && self->has_value()) {
                return typename pure_self::value_type{std::forward<Self>(self)->get()};
            }
            return typename pure_self::value_type{opt::none};
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
    template<class T, class P>
    using enable_has_value_and = std::enable_if_t<std::is_invocable_r_v<bool, P, T>, int>;

    template<class Self, class F>
    constexpr Self&& inspect(Self&& self, F&& f) {
        if (self.has_value()) {
            std::invoke(std::forward<F>(f), self.get());
        }
        return std::forward<Self>(self);
    }
    template<class T, class Fn>
    using enable_inspect = std::enable_if_t<std::is_invocable_v<Fn, T>, int>; // ignore return value

    template<class TupleOfOptions, class Tuple, std::size_t... Idx>
    constexpr auto unzip_impl(std::index_sequence<Idx...>) {
        return TupleOfOptions{opt::option<std::tuple_element_t<Idx, Tuple>>{}...};
    }

    template<class Self>
    constexpr auto unzip(Self&& self) {
        using tuple_type = impl::remove_cvref<typename impl::remove_cvref<Self>::value_type>;
        static_assert(is_tuple_like<tuple_type>,
            "To unzip opt::option<T>, T must be tuple-like."
            "A type T that satisfies tuple-like must be a specialization of "
            "std::array, std::pair, std::tuple");

        using tuple_of_options = impl::tuple_like_of_options<tuple_type>;
        if (self.has_value()) {
            return std::apply([&](auto&&... vals) {
                return tuple_of_options{opt::option{std::forward<decltype(vals)>(vals)}...};
            }, std::forward<Self>(self).get());
        } else {
            constexpr auto indexes = std::make_index_sequence<std::tuple_size_v<tuple_type>>{};
            return unzip_impl<tuple_of_options, tuple_type>(indexes);
        }
    }
}

template<class T>
class option : private impl::option_move_assign_base<T>
{
    using base = impl::option_move_assign_base<T>;

    template<class, impl::option_strategy> friend struct impl::internal_option_traits;

    using natvis_destruct_base = impl::option_destruct_base<T>; // For IntelliSense Natvis visualizations
public:
    static_assert(!std::is_same_v<T, opt::none_t>,
        "In opt::option<T>, T cannot be opt::none_t."
        "If you using CTAD (Class template argument deduction),"
        "you should probably specify the type for an empty opt::option<T>");
    static_assert(!std::is_void_v<T>,
        "T cannot be (possibly cv-qualified) `void`");
    static_assert(std::is_destructible_v<T>,
        "T must be destructible");

    using value_type = T;

    // Default constructor.
    // Constructors an object that does not contain a value.
    // Trivial only for `opt::option<T&>`.
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
    template<class U = T, impl::option::enable_constructor_5<T, U, /*is_explicit=*/true> = 0>
    constexpr explicit option(U&& val) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : base(std::forward<U>(val)) {}
    template<class U = T, impl::option::enable_constructor_5<T, U, /*is_explicit=*/false> = 0>
    constexpr option(U&& val) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : base(std::forward<U>(val)) {}

    // Constructs the `opt::option` that contains a value,
    // initialized from the arguments `first`, `args...`.
    // Postcondition: has_value() == true
    template<class First, class... Args, impl::option::enable_constructor_6<T, First, Args...> = 0>
    constexpr option(First&& first, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, First, Args...>)
        : base(std::forward<First>(first), std::forward<Args>(args)...) {}

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
    template<class U, impl::option::enable_constructor_8<T, U, /*is_explicit=*/false> = 0>
    constexpr option(const option<U>& other) noexcept(std::is_nothrow_constructible_v<T, const U&>) {
        base::construct_from_option(other);
    }
    template<class U, impl::option::enable_constructor_8<T, U, /*is_explicit=*/true> = 0>
    constexpr explicit option(const option<U>& other) noexcept(std::is_nothrow_constructible_v<T, const U&>) {
        base::construct_from_option(other);
    }
    // Converting move constructor.
    // If `other` containes a value, move constructs a contained value.
    // If `other` does not contain a value, constructs an empty `opt::option`.
    // Explicit if `!std::is_convertible_v<U&&, T>`.
    // Postcondition: has_value() == other.has_value()
    template<class U, impl::option::enable_constructor_9<T, U, /*is_explicit=*/false> = 0>
    constexpr option(option<U>&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
        base::construct_from_option(std::move(other));
    }
    template<class U, impl::option::enable_constructor_9<T, U, /*is_explicit=*/true> = 0>
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
    template<class... Args, std::enable_if_t<std::is_constructible_v<T, Args...>, int> = 0>
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
    constexpr bool has_value() const noexcept {
        return base::has_value();
    }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    // Returns `true` if the `opt::option` contains a value
    // and invocation of the `predicate` with the contained as an argument value evaluates to `true`;
    // otherwise return `false`
    // Same as Rust's `std::option::Option<T>::is_some_and`
    template<class P, impl::option::enable_has_value_and<T&, P> = 0>
    constexpr bool has_value_and(P&& predicate) & { return impl::option::has_value_and(*this, std::forward<P>(predicate)); }
    template<class P, impl::option::enable_has_value_and<const T&, P> = 0>
    constexpr bool has_value_and(P&& predicate) const& { return impl::option::has_value_and(*this, std::forward<P>(predicate)); }
    template<class P, impl::option::enable_has_value_and<T&&, P> = 0>
    constexpr bool has_value_and(P&& predicate) && { return impl::option::has_value_and(std::move(*this), std::forward<P>(predicate)); }
    template<class P, impl::option::enable_has_value_and<const T&&, P> = 0>
    constexpr bool has_value_and(P&& predicate) const&& { return impl::option::has_value_and(std::move(*this), std::forward<P>(predicate)); }

    // Takes the value out of the `opt::option`.
    // If this `opt::option` contains a value return it with move
    // and destroy which is left in `opt::option`;
    // otherwise return an empty `opt::option`
    // Same as Rust's `std::option::Option<T>::take`
    // Postcondition: has_value() == false
    constexpr option<T> take() & {
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
        option<T> tmp{std::move(*this)};
        reset();
        return tmp;
    }
    constexpr option<T> take() && {
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
        return std::move(*this);
    }

    // Takes the value out of the `opt::option`
    // if invocation of the `predicate` with the contained value as an argument evaluates to `true`.
    // This method similar to conditional `opt::option<T>::take`
    // Same as Rust's `std::option::Option<T>::take_if`
    template<class P, std::enable_if_t<std::is_invocable_r_v<bool, P, T&>, int> = 0>
    constexpr option<T> take_if(P&& predicate) {
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
        if (has_value() && bool(std::invoke(std::forward<P>(predicate), get()))) {
            return take();
        }
        return opt::none;
    }

    // Inserts `val` into the `opt::option` and returns a reference to it
    // If the `opt::option` contains a value, the contained value is destroyed
    // Same as Rust's `std::option::Option<T>::insert`
    constexpr T& insert(const std::remove_reference_t<T>& val) OPTION_LIFETIMEBOUND {
        this->emplace(val);
        return get();
    }
    constexpr T& insert(std::remove_reference_t<T>&& val) OPTION_LIFETIMEBOUND {
        this->emplace(std::move(val));
        return get();
    }

    // Invokes the `f` with a contained value
    // If this `opt::option` contains a value invoke `f` with the contained value as an argument;
    // otherwise do nothing.
    // Returns `*this`.
    // Does not modify the contained value inside.
    // Same as Rust's `std::option::Option<T>::inspect`
    template<class F, impl::option::enable_inspect<T&, F> = 0>
    constexpr option& inspect(F&& f) & { return impl::option::inspect(*this, std::forward<F>(f)); }
    template<class F, impl::option::enable_inspect<const T&, F> = 0>
    constexpr const option& inspect(F&& f) const& { return impl::option::inspect(*this, std::forward<F>(f)); }
    template<class F, impl::option::enable_inspect<T&, F> = 0>
    constexpr option&& inspect(F&& f) && { return impl::option::inspect(std::move(*this), std::forward<F>(f)); }
    template<class F, impl::option::enable_inspect<const T&, F> = 0>
    constexpr const option&& inspect(F&& f) const&& { return impl::option::inspect(std::move(*this), std::forward<F>(f)); }

    // Returns a reference to the contained value.
    // This method does not check whether the `opt::option` contains a value
    // on "Release" configuration.
    // Calls the `OPTION_VERIFY` macro if this `opt::option` does not contain the value.
    // Same as `std::optional<T>::operator*`.
    // Precondition: has_value() == true
    constexpr T& get() & noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return base::get();
    }
    constexpr const T& get() const& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return base::get();
    }
    constexpr T&& get() && noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::move(base::get());
    }
    constexpr const T&& get() const&& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::move(base::get());
    }
    // Returns a pointer to the contained value.
    // Calls the `OPTION_VERIFY` macro if this `opt::option` does not contain the value
    // Returns `std::addressof` of this `opt::option` contained value.
    // Same as `std::optional<T>::operator->`.
    // Precondition: has_value() == true
    constexpr std::add_pointer_t<const T> operator->() const noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::addressof(get());
    }
    constexpr std::add_pointer_t<T> operator->() noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::addressof(get());
    }
    // Returns a reference to the contained value.
    // Calls the `OPTION_VERIFY` macro if this `opt::option` does not contain the value
    // Same as `std::optional<T>::operator*` or `opt::option<T>::get`.
    // Precondition: has_value() == true
    constexpr T& operator*() & noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return get();
    }
    constexpr const T& operator*() const& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return get();
    }
    constexpr T&& operator*() && noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::move(get());
    }
    constexpr const T&& operator*() const&& noexcept OPTION_LIFETIMEBOUND {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::move(get());
    }

    // Returns a reference to the contained value.
    // Does not call the `OPTION_VERIFY` macro.
    // No OPTION_LIFETIMEBOUND
    constexpr T& get_unchecked() & noexcept { return base::get(); }
    constexpr const T& get_unchecked() const& noexcept { return base::get(); }
    constexpr T&& get_unchecked() && noexcept { return std::move(base::get()); }
    constexpr const T&& get_unchecked() const&& noexcept { return std::move(base::get()); }

    // Returns a reference to the contained value.
    // Throws a `opt::bad_access` if this `opt::option` does not contain the value.
    // More verbose version of `opt::option<T>::value`.
    constexpr T& value_or_throw() & OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(*this); }
    constexpr const T& value_or_throw() const& OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(*this); }
    constexpr T&& value_or_throw() && OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(std::move(*this)); }
    constexpr const T&& value_or_throw() const&& OPTION_LIFETIMEBOUND { return impl::option::value_or_throw(std::move(*this)); }

    // Returns a reference to the contained value.
    // Throws a `opt::bad_access` if this `opt::option` does not contain the value.
    // Similar to `std::optional<T>::value`.
    constexpr T& value() & OPTION_LIFETIMEBOUND { return value_or_throw(); }
    constexpr const T& value() const& OPTION_LIFETIMEBOUND { return value_or_throw(); }
    constexpr T&& value() && OPTION_LIFETIMEBOUND { return std::move(value_or_throw()); }
    constexpr const T&& value() const&& OPTION_LIFETIMEBOUND { return std::move(value_or_throw()); }

    // Returns the contained value if this `opt::option` contains the value;
    // otherwise returns a forwarded `default_value`.
    // Same as `std::optional<T>::value_or`
    template<class U>
    constexpr T value_or(U&& default_value) const& noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_constructible_v<T, U&&>)
        OPTION_LIFETIMEBOUND {
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
        static_assert(std::is_convertible_v<U&&, T>, "U&& must be convertible to T");
        if (has_value()) {
            return *(*this);
        }
        return static_cast<T>(std::forward<U>(default_value));
    }
    template<class U>
    constexpr T value_or(U&& default_value) && noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_constructible_v<T, U&&>)
        OPTION_LIFETIMEBOUND {
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
        static_assert(std::is_convertible_v<U&&, T>, "U&& must be convertible to T");
        if (has_value()) {
            return std::move(*(*this));
        }
        return static_cast<T>(std::forward<U>(default_value));
    }

    // Returns the contained value if `opt::option` contains the value;
    // otherwise return a default constructed `T` (expression `T{}`).
    // Similar to Rust's `std::option::Option<T>::value_or_default`
    constexpr T value_or_default() const& noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_default_constructible_v<T>) {
        static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
        if (has_value()) {
            return get();
        }
        return T{};
    }
    constexpr T value_or_default() && noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_default_constructible_v<T>) {
        static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
        if (has_value()) {
            return std::move(get());
        }
        return T{};
    }

    // Returns the provided `def` default value if this `opt::option` does not contain a value;
    // otherwise, return result of a `f` invocation with this `opt::option` contained value as an argument.
    // Use `opt::option<T>::map_or_else` to lazy evaluate the default value
    // Same as Rust's `std::option::Option<T>::map_or`
    template<class U, class F, impl::option::enable_map_or<T&, U, F> = 0>
    constexpr auto map_or(U&& def, F&& f) & { return impl::option::map_or<T>(*this, std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F, impl::option::enable_map_or<const T&, U, F> = 0>
    constexpr auto map_or(U&& def, F&& f) const& { return impl::option::map_or<T>(*this, std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F, impl::option::enable_map_or<T&&, U, F> = 0>
    constexpr auto map_or(U&& def, F&& f) && { return impl::option::map_or<T>(std::move(*this), std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F, impl::option::enable_map_or<const T&&, U, F> = 0>
    constexpr auto map_or(U&& def, F&& f) const&& { return impl::option::map_or<T>(std::move(*this), std::forward<U>(def), std::forward<F>(f)); }

    // Returns a `def` default function result if this `opt::option` does not contain a value;
    // otherwise, returns result of a 'f' invocation with this `opt::option` contained value as an argument
    // Same as Rust's `std::option::Option<T>::map_or_else`
    template<class D, class F, impl::option::enable_map_or_else<T&, D, F> = 0>
    constexpr auto map_or_else(D&& def, F&& f) & { return impl::option::map_or_else<T>(*this, std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F, impl::option::enable_map_or_else<const T&, D, F> = 0>
    constexpr auto map_or_else(D&& def, F&& f) const& { return impl::option::map_or_else<T>(*this, std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F, impl::option::enable_map_or_else<T&&, D, F> = 0>
    constexpr auto map_or_else(D&& def, F&& f) && { return impl::option::map_or_else<T>(std::move(*this), std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F, impl::option::enable_map_or_else<const T&&, D, F> = 0>
    constexpr auto map_or_else(D&& def, F&& f) const&& { return impl::option::map_or_else<T>(std::move(*this), std::forward<D>(def), std::forward<F>(f)); }

    // Returns a pointer to the contained value if this `opt::option` contains the value;
    // otherwise, return `nullptr`.
    constexpr std::remove_reference_t<T>* ptr_or_null() & noexcept OPTION_LIFETIMEBOUND {
        return has_value() ? std::addressof(get()) : nullptr;
    }
    constexpr const std::remove_reference_t<T>* ptr_or_null() const& noexcept OPTION_LIFETIMEBOUND {
        return has_value() ? std::addressof(get()) : nullptr;
    }
    constexpr void ptr_or_null() && = delete;
    constexpr void ptr_or_null() const&& = delete;

    // Returns an empty `opt::option` if this `opt::option` does not contain a value;
    // otherwise, invoke `f` with the contained value as an argument and return:
    //     `opt::option` that containes current contained value if `f` return `true`
    //     an empty `opt::option` if `f` returns `false`
    // Same as Rust's `std::option::Option<T>::filter`
    template<class F, std::enable_if_t<std::is_invocable_r_v<bool, F, const T&>, int> = 0>
    constexpr option<T> filter(F&& f) const {
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
        if (has_value() && bool(std::invoke(std::forward<F>(f), get()))) {
            return get();
        }
        return {};
    }

    // Converts `opt::option<opt::option<U>>` to `opt::option<U>`.
    // If this `opt::option<T>` then return underlying `opt::option<U>`;
    // otherwise return an empty `opt::option<U>`.
    // Where `U` is `typename T::value_type`.
    // Example: opt::option<opt::option<int>> -> flatten() -> opt::option<int>
    // Same as Rust's `std::option::Option<T>::flatten`
    constexpr auto flatten() const& { return impl::option::flatten(*this); }
    constexpr auto flatten() && { return impl::option::flatten(std::move(*this)); }

    // Returns an empty `opt::option` if this `opt::option` does not contain a value;
    // otherwise invokes `f` with the contained value as an argument and returns the result.
    // This operation is also sometimes called flatmap.
    // Same as `std::optional<T>::and_then` or Rust's `std::option::Option<T>::and_then`
    template<class F, impl::option::enable_and_then<T&, F> = 0>
    constexpr auto and_then(F&& f) & { return impl::option::and_then(*this, std::forward<F>(f)); }
    template<class F, impl::option::enable_and_then<const T&, F> = 0>
    constexpr auto and_then(F&& f) const& { return impl::option::and_then(*this, std::forward<F>(f)); }
    template<class F, impl::option::enable_and_then<T&&, F> = 0>
    constexpr auto and_then(F&& f) && { return impl::option::and_then(std::move(*this), std::forward<F>(f)); }
    template<class F, impl::option::enable_and_then<const T&&, F> = 0>
    constexpr auto and_then(F&& f) const&& { return impl::option::and_then(std::move(*this), std::forward<F>(f)); }

    // Maps an `opt::option<T>` to `opt::option<U>` by applying a function to a contained value, otherwise returns `opt::none`
    // If this `opt::option` contains a value, invokes `f` with the contained value as an argument,
    // then returns an `opt::option` that contains the result of invocation; otherwise, return an empty `opt::option`
    // Same as `std::optional<T>::transform` or Rust's `std::option::Option<T>::map`
    template<class F, impl::option::enable_map<T&, F> = 0>
    constexpr auto map(F&& f) & { return impl::option::map<T>(*this, std::forward<F>(f)); }
    template<class F, impl::option::enable_map<const T&, F> = 0>
    constexpr auto map(F&& f) const& { return impl::option::map<T>(*this, std::forward<F>(f)); }
    template<class F, impl::option::enable_map<T&&, F> = 0>
    constexpr auto map(F&& f) && { return impl::option::map<T>(std::move(*this), std::forward<F>(f)); }
    template<class F, impl::option::enable_map<const T&&, F> = 0>
    constexpr auto map(F&& f) const&& { return impl::option::map<T>(std::move(*this), std::forward<F>(f)); }

    // Returns a contained value if this `opt::option` contains a value;
    // otherwise, return the result of `f`.
    // Same as `std::optional<T>::or_else`
    template<class F, impl::option::enable_or_else<F> = 0>
    constexpr option<T> or_else(F&& f) const& { return impl::option::or_else<T>(*this, std::forward<F>(f)); }
    template<class F, impl::option::enable_or_else<F> = 0>
    constexpr option<T> or_else(F&& f) && { return impl::option::or_else<T>(std::move(*this), std::forward<F>(f)); }

    // Specifies that this `opt::option` will always contains value at a given point.
    // Will cause undefined behavior if this `opt::option` does not contain a value.
    constexpr void assume_has_value() const noexcept {
#ifdef NDEBUG
        if (!has_value()) {
            OPTION_UNREACHABLE();
        }
#else
        OPTION_VERIFY(has_value(), "Assumption 'has_value()' failed");
#endif
    }

    // Unzips this `opt::option` containing a tuple like object to the tuple like object of `opt::option`s
    // If this `opt::option` contains a value, return tuple like object that contains
    // `opt::option`s of the current `opt::option` underlying values;
    // otherwise, return tuple like object that contains the empty `opt::option`s.
    // Where tuple like object is a std::array, std::pair or a std::tuple
    // Example: opt::option<std::tuple<int, float>> -> unzip() -> std::tuple<opt::option<int>, opt::option<float>>
    // Similar to Rust's `std::option::Option<T>::unzip` but for any number of values
    constexpr auto unzip() & { return impl::option::unzip(*this); }
    constexpr auto unzip() const& { return impl::option::unzip(*this); }
    constexpr auto unzip() && { return impl::option::unzip(std::move(*this)); }
    constexpr auto unzip() const&& { return impl::option::unzip(std::move(*this)); }

    // Replaces the value in this `opt::option` by a provided `val`
    // and returns the old `opt::option` value
    // Similar to Rust's `std::option::Option<T>::replace`
    template<class U, std::enable_if_t<std::is_constructible_v<T, U&&>, int> = 0>
    constexpr option<T> replace(U&& val) & {
        option<T> tmp{std::move(*this)};
        // should call the destructor after moving, because moving does not end lifetime
        base::reset();
        base::construct(std::forward<U>(val));
        return tmp;
    }
};

template<class T>
option(T) -> option<T>;

template<class T>
option(option<T>) -> option<option<T>>;

namespace impl {
    template<class T>
    struct internal_option_traits<opt::option<T>, option_strategy::avaliable_option> {
        using base = impl::option_move_assign_base<T>;
        using traits = opt::option_traits<T>;

        static constexpr std::uintmax_t max_level = traits::max_level - 1;

        static std::uintmax_t get_level(const opt::option<T>* const value) noexcept {
            const std::uintmax_t level = traits::get_level(std::addressof(static_cast<const base*>(value)->value));
            if (level == std::uintmax_t(-1) || level == 0) {
                return std::uintmax_t(-1);
            } else {
                return level - 1;
            }
        }

        static void set_level(opt::option<T>* const value, const std::uintmax_t level) noexcept {
            traits::set_level(std::addressof(static_cast<base*>(value)->value), level + 1);
        }
    };
    template<class T>
    struct internal_option_traits<opt::option<T>, option_strategy::unavaliable_option> {
        using base = impl::option_move_assign_base<T>;
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

// Zips `options...` into `opt::option<std::tuple<VALS...>>`
// where `VALS...` are underlying values of `options...`
// if every `options...` contains a value; otherwise returns an empty `opt::option`
// Example: opt::zip(option1, option2)
// will return `opt::option<std::tuple<option1::value_type, option2::value_type>>`
// if `option1.has_value()` and `option2.has_value()`; otherwise will return an empty `opt::option`
template<class... Options, std::enable_if_t<
    (opt::is_option<impl::remove_cvref<Options>> && ...)
, int> = 0>
constexpr auto zip(Options&&... options)
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
template<class Fn, class... Options, std::enable_if_t<
    (opt::is_option<impl::remove_cvref<Options>> && ...)
, int> = 0>
constexpr auto zip_with(Fn&& fn, Options&&... options)
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
            return opt::option<fn_result>{};
        }
    }
}

namespace impl {
    template<class To, class From>
    struct static_cast_functor {
        constexpr To operator()(From&& from) noexcept(std::is_nothrow_constructible_v<To, From&&>) {
            return static_cast<To>(std::forward<From>(from));
        }
    };
}

// Constructs 'opt::option<To>' using 'const From&', if passed 'opt::option<From>' has value
// else return 'opt::none'
template<class To, class From>
constexpr opt::option<To> option_cast(const opt::option<From>& value) {
    return value.map(impl::static_cast_functor<To, const From&>{});
}
// Constructs 'opt::option<To>' using 'From&&', if passed 'opt::option<From>' has value
// else return 'opt::none'
template<class To, class From>
constexpr opt::option<To> option_cast(opt::option<From>&& value) {
    return std::move(value).map(impl::static_cast_functor<To, From&&>{});
}

// Constructs 'opt::option<T>' from dereferenced value of proveded pointer if it is not equal to 'nullptr';
// otherwise, returns 'opt::none'
template<class T>
constexpr opt::option<T> from_nullable(T* const nullable_ptr) {
    if (nullable_ptr == nullptr) { return {}; }
    return {*nullable_ptr};
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
constexpr T operator|(const opt::option<T>& left, const T& right) {
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
constexpr opt::option<T> operator|(const opt::option<T>& left, const opt::option<T>& right) {
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
constexpr opt::option<T> operator|(const opt::option<T>& left, none_t) {
    return left;
}
// Returns `right`
// y = right option value
// N = empty option (none)
//| left | right | result |
//|   N  |   y   |   y    |
//|   N  |   N   |   N    |
template<class T>
constexpr opt::option<T> operator|(none_t, const opt::option<T>& right) {
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
constexpr opt::option<U> operator&(const opt::option<T>& left, const opt::option<U>& right) {
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
constexpr opt::option<T> operator^(const opt::option<T>& left, const opt::option<T>& right) {
    if (left.has_value() && !right.has_value()) {
        return left;
    }
    if (!left.has_value() && right.has_value()) {
        return right;
    }
    return opt::none;
}

namespace impl {
    template<class Op, class T1, class T2>
    constexpr bool do_option_comparison(const opt::option<T1>& left, const opt::option<T2>& right) {
        if (left.has_value() && right.has_value()) {
            return Op{}(*left, *right);
        }
        return Op{}(left.has_value(), right.has_value());
    }
}

// Compare two opt::option
template<class T1, class T2>
constexpr bool operator==(const option<T1>& left, const option<T2>& right) noexcept(noexcept(*left == *right)) {
    return impl::do_option_comparison<std::equal_to<>>(left, right);
}
template<class T1, class T2>
constexpr bool operator!=(const option<T1>& left, const option<T2>& right) noexcept(noexcept(*left != *right)) {
    return impl::do_option_comparison<std::not_equal_to<>>(left, right);
}
template<class T1, class T2>
constexpr bool operator<(const option<T1>& left, const option<T2>& right) noexcept(noexcept(*left < *right)) {
    return impl::do_option_comparison<std::less<>>(left, right);
}
template<class T1, class T2>
constexpr bool operator<=(const option<T1>& left, const option<T2>& right) noexcept(noexcept(*left <= *right)) {
    return impl::do_option_comparison<std::less_equal<>>(left, right);
}
template<class T1, class T2>
constexpr bool operator>(const option<T1>& left, const option<T2>& right) noexcept(noexcept(*left > *right)) {
    return impl::do_option_comparison<std::greater<>>(left, right);
}
template<class T1, class T2>
constexpr bool operator>=(const option<T1>& left, const option<T2>& right) noexcept(noexcept(*left >= *right)) {
    return impl::do_option_comparison<std::greater_equal<>>(left, right);
}
// Compare an opt::option with a opt::none
template<class T>
constexpr bool operator==(const option<T>& left, none_t) noexcept {
    return !left.has_value();
}
template<class T>
constexpr bool operator==(none_t, const option<T>& right) noexcept {
    return !right.has_value();
}
template<class T>
constexpr bool operator!=(const option<T>& left, none_t) noexcept {
    return left.has_value();
}
template<class T>
constexpr bool operator!=(none_t, const option<T>& right) noexcept {
    return right.has_value();
}
template<class T>
constexpr bool operator<(const option<T>&, none_t) noexcept {
    return false;
}
template<class T>
constexpr bool operator<(none_t, const option<T>& right) noexcept {
    return right.has_value();
}
template<class T>
constexpr bool operator<=(const option<T>& left, none_t) noexcept {
    return !left.has_value();
}
template<class T>
constexpr bool operator<=(none_t, const option<T>&) noexcept {
    return true;
}
template<class T>
constexpr bool operator>(const option<T>& left, none_t) noexcept {
    return left.has_value();
}
template<class T>
constexpr bool operator>(none_t, const option<T>&) noexcept {
    return false;
}
template<class T>
constexpr bool operator>=(const option<T>&, none_t) noexcept {
    return true;
}
template<class T>
constexpr bool operator>=(none_t, const option<T>& right) noexcept {
    return !right.has_value();
}

namespace impl {
    template<class Op, bool if_hasnt_value, class T1, class T2>
    constexpr bool do_option_comparison_with_value(const opt::option<T1>& left, const T2& right) {
        if (left.has_value()) {
            return Op{}(*left, right);
        }
        return if_hasnt_value;
    }
    template<class Op, bool if_hasnt_value, class T1, class T2>
    constexpr bool do_option_comparison_with_value(const T1& left, const opt::option<T2>& right) {
        if (right.has_value()) {
            return Op{}(left, *right);
        }
        return if_hasnt_value;
    }
}

template<class T1, class T2>
constexpr bool operator==(const option<T1>& left, const T2& right) noexcept(noexcept(*left == right)) {
    return impl::do_option_comparison_with_value<std::equal_to<>, false>(left, right);
}
template<class T1, class T2>
constexpr bool operator==(const T1& left, const option<T2>& right) noexcept(noexcept(left == *right)) {
    return impl::do_option_comparison_with_value<std::equal_to<>, false>(left, right);
}
template<class T1, class T2>
constexpr bool operator!=(const option<T1>& left, const T2& right) noexcept(noexcept(*left != right)) {
    return impl::do_option_comparison_with_value<std::not_equal_to<>, true>(left, right);
}
template<class T1, class T2>
constexpr bool operator!=(const T1& left, const option<T2>& right) noexcept(noexcept(left != *right)) {
    return impl::do_option_comparison_with_value<std::not_equal_to<>, true>(left, right);
}
template<class T1, class T2>
constexpr bool operator<(const option<T1>& left, const T2& right) noexcept(noexcept(*left < right)) {
    return impl::do_option_comparison_with_value<std::less<>, true>(left, right);
}
template<class T1, class T2>
constexpr bool operator<(const T1& left, const option<T2>& right) noexcept(noexcept(left < *right)) {
    return impl::do_option_comparison_with_value<std::less<>, false>(left, right);
}
template<class T1, class T2>
constexpr bool operator<=(const option<T1>& left, const T2& right) noexcept(noexcept(*left <= right)) {
    return impl::do_option_comparison_with_value<std::less_equal<>, true>(left, right);
}
template<class T1, class T2>
constexpr bool operator<=(const T1& left, const option<T2>& right) noexcept(noexcept(left <= *right)) {
    return impl::do_option_comparison_with_value<std::less_equal<>, false>(left, right);
}
template<class T1, class T2>
constexpr bool operator>(const option<T1>& left, const T2& right) noexcept(noexcept(*left > right)) {
    return impl::do_option_comparison_with_value<std::greater<>, false>(left, right);
}
template<class T1, class T2>
constexpr bool operator>(const T1& left, const option<T2>& right) noexcept(noexcept(left > *right)) {
    return impl::do_option_comparison_with_value<std::greater<>, true>(left, right);
}
template<class T1, class T2>
constexpr bool operator>=(const option<T1>& left, const T2& right) noexcept(noexcept(*left >= right)) {
    return impl::do_option_comparison_with_value<std::greater_equal<>, false>(left, right);
}
template<class T1, class T2>
constexpr bool operator>=(const T1& left, const option<T2>& right) noexcept(noexcept(left >= *right)) {
    return impl::do_option_comparison_with_value<std::greater_equal<>, true>(left, right);
}

}

template<class T>
struct std::hash<opt::option<T>> {
private:
    using value_hash = std::hash<std::remove_const_t<T>>;
public:
    std::size_t operator()(const opt::option<T>& val) noexcept(noexcept(value_hash{}(*val))) {
        if (val.has_value()) {
            return value_hash{}(*val);
        }
        return static_cast<std::size_t>(-96391);
    }
};
