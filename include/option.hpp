#pragma once

#include <type_traits>
#include <utility>
#include <cstdint>
#include <memory>
#include <new>
#include <functional>
#include <exception>
#include <cstring>
#include <tuple>
#include <array>

#ifdef __has_builtin
    #if __has_builtin(__builtin_assume)
        #define OPTION_ASSUME(expression) __builtin_assume(expression)
    #endif
#endif
#ifndef OPTION_ASSUME
        #if defined(__GNUC__) || defined(__GNUG__)
            #define OPTION_ASSUME(expression) ((expression) ? (void)0 : (void)__builtin_unreachable())
        #elif defined(_MSC_VER)
            #define OPTION_ASSUME(expression) __assume(expression)
        #else
            #define OPTION_ASSUME(expression) ((void)0)
        #endif
#endif

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
    #define OPTION_CONST_ATTR [[gnu::const]]
#elif defined(_MSC_VER)
    #define OPTION_CONST_ATTR __declspec(noalias)
#else
    #define OPTION_CONST_ATTR
#endif

#ifndef OPTION_VERIFY
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
        #include <cstdio>
        #define OPTION_VERIFY(expression, message) \
            ((expression) ? (void)0 : ( \
                (void)std::fprintf(stderr, "%s:%d: assertion '%s' failed: %s\n", __FILE__, __LINE__, #expression, message), \
                (void)OPTION_DEBUG_BREAK) \
            )
    #else
        //#define OPTION_VERIFY(expression, message) OPTION_ASSUME(expression)
        #define OPTION_VERIFY(expression, message)
    #endif
#endif

namespace opt {

namespace impl {
    struct none_tag_ctor {};
}

struct none_t {
    constexpr explicit none_t(impl::none_tag_ctor) {}
};
inline constexpr none_t none{impl::none_tag_ctor{}};

template<class T>
class option;

template<class T, class = void>
struct option_flag;

template<class T>
inline constexpr bool is_option = false;
template<class T>
inline constexpr bool is_option<opt::option<T>> = true;

namespace impl {
    template<class T, bool is_enum = std::is_enum_v<T>, class = void>
    struct has_exploit_unused_value : std::false_type {};
    template<class T>
    struct has_exploit_unused_value<T, true, std::void_t<
        decltype(T::OPTION_EXPLOIT_UNUSED_VALUE)
    >> : std::true_type {};

    template<class T, class...>
    using first_type = T;

    template<class T>
    inline constexpr bool is_bit_representable = std::is_unsigned_v<T> && !std::is_same_v<T, bool>;

    template<class Left, class Right>
    constexpr bool bit_equal(const Left& left, const Right& right) noexcept {
        static_assert(sizeof(Left) == sizeof(Right));
        if constexpr (impl::is_bit_representable<Left> && impl::is_bit_representable<Right>) {
            return left == right;
        } else {
            return std::memcmp(&left, &right, sizeof(Left)) == 0;
        }
    }
    template<class Dest, class Source>
    constexpr void bit_copy(Dest& dest, const Source& src) noexcept {
        static_assert(sizeof(Dest) == sizeof(Source));
        if constexpr (impl::is_bit_representable<Dest> && impl::is_bit_representable<Source>) {
            dest = src;
        } else {
            std::memcpy(&dest, &src, sizeof(Dest));
        }
    }
    template<class To, class From>
    constexpr To bit_cast(const From& from) noexcept {
        static_assert(sizeof(To) == sizeof(From));
        To result;
        impl::bit_copy(result, from);
        return result;
    }

    template<class T, auto sentinel>
    class sentinel_option_flag {
        static_assert(sizeof(T) == sizeof(sentinel));
        static constexpr auto empty_value = sentinel; // for IntelliSense natvis
    public:
        static bool is_empty(const T& value) noexcept {
            return impl::bit_equal(value, sentinel);
        }
        static void construct_empty_flag(T& value) noexcept {
            impl::bit_copy(value, sentinel);
        }
    };
}

template<>
struct option_flag<bool> {
    static constexpr std::uint_least8_t empty_value = 0b0010;

    static bool is_empty(const bool& value) noexcept {
        const auto uint_value = impl::bit_cast<std::uint_least8_t>(value);
        return uint_value & empty_value;
    }
    static void construct_empty_flag(bool& value) noexcept {
        auto uint_value = impl::bit_cast<std::uint_least8_t>(value);
        uint_value |= empty_value;
        impl::bit_copy(value, uint_value);
    }
};

template<class T>
struct option_flag<T, std::enable_if_t<std::is_pointer_v<T>>> {
    static constexpr std::uintptr_t empty_value = [] {
        if constexpr (sizeof(void*) == 8) {
            // Uses 48-bit virtual address space implementation
            // 0x00007FFFFFFFFFFF = Canonical lower half
            // 0xFFFF800000000000 = Canonical higher half
            // 0x7FFFFFFFFFFFFFFF = Noncanonical middle address
            // (0x00007FFFFFFFFFFF + 0xFFFF800000000000) / 2
            return 0x7FFFFFFFFFFFFFFFu;
        } else if constexpr (sizeof(void*) == 4) {
            // For x86 architecture
            return 0xFFFFFFF3u;
        } else if constexpr (sizeof(void*) == 2) {
            // For 16-bit architecture
            return 0xFFFFu;
        } else if constexpr (sizeof(void*) == 1) {
            // For 8-bit architecture
            return 0xFFu;
        } else {
            static_assert(!sizeof(T*), "Unknown architecture");
        }
    }();

    static bool is_empty(const T& value) noexcept {
        return impl::bit_equal(value, empty_value);
    }
    static void construct_empty_flag(T& value) noexcept {
        impl::bit_copy(value, empty_value);
    }
};

template<class T>
struct option_flag<T, std::enable_if_t<impl::has_exploit_unused_value<T>::value>>
    : impl::sentinel_option_flag<T, T::OPTION_EXPLOIT_UNUSED_VALUE> {};

namespace impl {
    template<class T>
    using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

    struct nontrivial_dummy_t {
        constexpr nontrivial_dummy_t() noexcept {}
    };

    struct construct_from_invoke_tag {
        explicit construct_from_invoke_tag() = default;
    };

    template<class>
    inline constexpr bool is_reference_wrapper = false;
    template<class T>
    inline constexpr bool is_reference_wrapper<std::reference_wrapper<T>> = true;

    template<class T>
    struct unwrap_reference_t { using type = T; };
    template<class T>
    struct unwrap_reference_t<std::reference_wrapper<T>> { using type = T&; };
    template<class T>
    using unwrap_reference = typename unwrap_reference_t<T>::type;

    template<class T, class U>
    struct copy_cvref_t {
        using U1 = std::conditional_t<std::is_const_v<T>, std::add_const_t<U>, U>;
        using U2 = std::conditional_t<std::is_volatile_v<T>, std::add_volatile_t<U1>, U1>;
        using U3 = std::conditional_t<std::is_lvalue_reference_v<T>, std::add_lvalue_reference_t<U2>, U2>;
        using type = std::conditional_t<std::is_rvalue_reference_v<T>, std::add_rvalue_reference_t<U3>, U3>;
    };
    template<class T, class U>
    using copy_cvref = typename copy_cvref_t<T, U>::type;

    template<class T, class = std::size_t>
    inline constexpr bool has_option_flag = false;
    template<class T>
    inline constexpr bool has_option_flag<T, decltype(sizeof(opt::option_flag<T>))> = true;

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

    template<class T, class... Args>
    constexpr void construct_at(T& obj, Args&&... args) {
        if constexpr (std::is_trivially_copy_assignable_v<T>) {
            obj = T(std::forward<Args>(args)...);
        } else {
            ::new(static_cast<void*>(std::addressof(obj))) T(std::forward<Args>(args)...);
        }
    }

    template<class T,
        bool store_flag = !has_option_flag<T>,
        bool is_trivially_destructible = std::is_trivially_destructible_v<T>
    >
    struct option_destruct_base;

    template<class T>
    struct option_destruct_base<T, /*store_flag=*/true, /*is_trivially_destructible=*/true> {
        union {
            nontrivial_dummy_t dummy;
            T value;
        };
    private:
        bool has_value_flag;
    public:

        constexpr option_destruct_base() noexcept
            : dummy(), has_value_flag(false) {}

        template<class... Args>
        constexpr option_destruct_base(Args&&... args)
            : value(std::forward<Args>(args)...), has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value(std::invoke(std::forward<F>(f), std::forward<Arg>(arg))), has_value_flag(true) {}

        constexpr void reset() noexcept {
            has_value_flag = false;
        }
        constexpr bool has_value() const noexcept {
            return has_value_flag;
        }
        // precondition: has_value() == false
        template<class... Args>
        constexpr void construct(Args&&... args) {
            // has_value() == false
            impl::construct_at(value, std::forward<Args>(args)...);
            has_value_flag = true;
        }
    };
    template<class T>
    struct option_destruct_base<T, /*store_flag=*/true, /*is_trivially_destructible=*/false> {
        union {
            nontrivial_dummy_t dummy;
            T value;
        };
    private:
        bool has_value_flag;
    public:

        constexpr option_destruct_base() noexcept
            : dummy(), has_value_flag(false) {}

        template<class... Args>
        constexpr option_destruct_base(Args&&... args)
            : value(std::forward<Args>(args)...), has_value_flag(true) {}

        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value(std::invoke(std::forward<F>(f), std::forward<Arg>(arg))), has_value_flag(true) {}

        ~option_destruct_base() noexcept(std::is_nothrow_destructible_v<T>) {
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
        constexpr bool has_value() const noexcept {
            return has_value_flag;
        }
        // precondition: has_value() == false
        template<class... Args>
        constexpr void construct(Args&&... args) {
            // has_value() == false
            impl::construct_at(value, std::forward<Args>(args)...);
            has_value_flag = true;
        }
    };
    template<class T>
    struct option_destruct_base<T, /*store_flag=*/false, /*is_trivially_destructible=*/true> {
        union {
            nontrivial_dummy_t dummy;
            T value;
        };
        using flag = opt::option_flag<T>;

        constexpr option_destruct_base() noexcept
            : dummy() {
            flag::construct_empty_flag(value);
            // has_value() == false
        }
        template<class... Args>
        constexpr option_destruct_base(Args&&... args)
            : value(std::forward<Args>(args)...) {
            // has_value() == true
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value(std::invoke(std::forward<F>(f), std::forward<Arg>(arg))) {
            // has_value() == true
        }

        constexpr void reset() noexcept {
            flag::construct_empty_flag(value);
            // has_value() == false
        }
        constexpr bool has_value() const noexcept {
            return !flag::is_empty(value);
        }
        // precondition: has_value() == false
        template<class... Args>
        constexpr void construct(Args&&... args) {
            // has_value() == false
            impl::construct_at(value, std::forward<Args>(args)...);
            // has_value() == true
        }
    };
    template<class T>
    struct option_destruct_base<T, /*store_flag=*/false, /*is_trivially_destructible=*/false> {
        union {
            nontrivial_dummy_t dummy;
            T value;
        };
        using flag = opt::option_flag<T>;

        constexpr option_destruct_base() noexcept
            : dummy() {
            flag::construct_empty_flag(value);
            // has_value() == false
        }
        template<class... Args>
        constexpr option_destruct_base(Args&&... args)
            : value(std::forward<Args>(args)...) {
            flag::destroy_empty_flag(value);
            // has_value() == true
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value(std::invoke(std::forward<F>(f), std::forward<Arg>(arg))) {
            flag::destroy_empty_flag(value);
            // has_value() == true
        }

        ~option_destruct_base() noexcept(std::is_nothrow_destructible_v<T>) {
            if (has_value()) {
                value.~T();
            }
        }

        constexpr void reset() noexcept {
            if (has_value()) {
                value.~T();
                flag::construct_empty_flag(value);
            }
            // has_value() == false
        }
        constexpr bool has_value() const noexcept {
            return !flag::is_empty(value);
        }
        // precondition: has_value() == false
        template<class... Args>
        constexpr void construct(Args&&... args) {
            // has_value() == false
            impl::construct_at(value, std::forward<Args>(args)...);
            flag::destroy_empty_flag(value);
            // has_value() == true
        }
    };

    template<class T, bool is_reference /*false*/ = std::is_reference_v<T>>
    class option_storage_base : private option_destruct_base<T> {
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
        // precondition: has_value() == false
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

        raw_type* value;

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
        option_storage_base() = default;

        template<class Arg>
        constexpr option_storage_base(Arg&& arg) noexcept
            : value(ref_to_ptr(std::forward<Arg>(arg))) {}

        template<class F, class Arg>
        constexpr option_storage_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value(ref_to_ptr(std::invoke(std::forward<F>(f), std::forward<Arg>(arg)))) {}

        constexpr bool has_value() const noexcept {
            return value != nullptr;
        }
        constexpr void reset() noexcept {
            value = nullptr;
        }

        constexpr T& get() const& noexcept { return *value; }
        constexpr T&& get() const&& noexcept { return std::move(*value); }

        // precondition: has_value() == false
        template<class Arg>
        constexpr void construct(Arg&& arg) noexcept {
            value = ref_to_ptr(std::forward<Arg>(arg));
        }
        // precondition: has_value() == false
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
    inline constexpr bool is_cv_bool = std::is_same_v<T, std::remove_cv_t<T>>;

    struct secret_type {};
}

class bad_access : public std::exception {
public:
    bad_access() noexcept = default;
    bad_access(const bad_access&) noexcept = default;

    const char* what() const noexcept override {
        return "Bad opt::option access";
    }
};

namespace impl::option {
    template<class T, class U, bool is_explicit>
    using enable_constructor_5 = std::enable_if_t<
           std::is_constructible_v<T, U&&> && !std::is_same_v<impl::remove_cvref<U>, opt::option<T>>
        && !(std::is_same_v<impl::remove_cvref<T>, bool> && opt::is_option<impl::remove_cvref<U>>)
        && (std::is_convertible_v<U&&, T> == !is_explicit) // explicit( condition )
    , int>;

    template<class T, class First, class... Args>
    using enable_constructor_6 = std::enable_if_t<
        std::is_constructible_v<T, First, Args...> && !std::is_same_v<impl::remove_cvref<First>, opt::option<T>>
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
        static_assert(opt::is_option<invoke_res>);
        if (self.has_value()) {
            return std::invoke(std::forward<F>(f), *std::forward<Self>(self));
        } else {
            return impl::remove_cvref<invoke_res>{opt::none};
        }
    }
    // implementation of opt::option<T>::map(F&&)
    // map(F&&) -> option<U> : F(T&&) -> U
    template<class T, class Self, class F>
    constexpr auto map(Self&& self, F&& f) {
        using f_result = std::remove_cv_t<std::invoke_result_t<F, impl::copy_cvref<Self, T>>>;
        if (self.has_value()) {
            return opt::option<f_result>{construct_from_invoke_tag{}, std::forward<F>(f), *std::forward<Self>(self)};
        }
        return opt::option<f_result>{opt::none};
    }
    // implementation of opt::option<T>::map_or(U&&, F&&)
    template<class T, class Self, class U, class F>
    constexpr impl::remove_cvref<U> map_or(Self&& self, U&& default_value, F&& f) {
        if (self.has_value()) {
            using raw_u = impl::remove_cvref<U>;
            return static_cast<raw_u>(std::invoke(std::forward<F>(f), std::forward<Self>(self).get()));
        }
        return std::forward<U>(default_value);
    }
    template<class T, class Self, class D, class F>
    constexpr auto map_or_else(Self&& self, D&& d, F&& f) {
        using d_result = std::invoke_result_t<D>;
        using f_result = std::invoke_result_t<F, impl::copy_cvref<Self, T>>;
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
        if (self.has_value()) {
            return std::forward<Self>(self);
        }
        return std::invoke(std::forward<F>(f));
    }
    // implementation of opt::option<T>::value_or_throw()
    template<class Self>
    constexpr auto&& value_or_throw(Self&& self) {
        if (!self.has_value()) {
            throw bad_access{};
        }
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
    template<class Self, class F>
    constexpr Self&& inspect(Self&& self, F&& f) {
        if (self.has_value()) {
            std::invoke(std::forward<F>(f), self.get());
        }
        return std::forward<Self>(self);
    }
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
    using raw_type = std::remove_reference_t<T>;

    using natvis_opt_flag = opt::option_flag<T>; // For IntelliSense Natvis visualizations
public:
    static_assert(!std::is_same_v<T, opt::none_t>,
        "In opt::option<T>, T cannot be opt::none_t."
        "If you using CTAD (Class template argument deduction),"
        "you should probably specify the type for an empty opt::option<T>");

    using value_type = T;

    // 1
    // postcondition: has_value() == false
    option() = default;
    // 2
    // postcondition: has_value() == false
    constexpr option(opt::none_t) noexcept : base() {}

    // 3
    // postcondition: has_value() == other.has_value()
    option(const option&) = default;
    // 4
    // postcondition: has_value() == other.has_value()
    option(option&&) = default;
    // 5
    // postcondition: has_value() == true
    template<class U = T, impl::option::enable_constructor_5<T, U, /*is_explicit=*/true> = 0>
    constexpr explicit option(U&& val) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : base(std::forward<U>(val)) {}
    template<class U = T, impl::option::enable_constructor_5<T, U, /*is_explicit=*/false> = 0>
    constexpr option(U&& val) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : base(std::forward<U>(val)) {}
    // 6
    // postcondition: has_value() == true
    template<class First, class... Args, impl::option::enable_constructor_6<T, First, Args...> = 0>
    constexpr option(First&& first, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, First, Args...>)
        : base(std::forward<First>(first), std::forward<Args>(args)...) {}

    // postcondition: has_value() == true
    // 7
    template<class F, class Arg>
    constexpr explicit option(impl::construct_from_invoke_tag, F&& f, Arg&& arg)
        : base(impl::construct_from_invoke_tag{}, std::forward<F>(f), std::forward<Arg>(arg)) {}

    // 8
    template<class U, impl::option::enable_constructor_8<T, U, /*is_explicit=*/false> = 0>
    constexpr option(const option<U>& other) {
        construct_from_option(other);
    }
    template<class U, impl::option::enable_constructor_8<T, U, /*is_explicit=*/true> = 0>
    constexpr explicit option(const option<U>& other) {
        construct_from_option(other);
    }
    // 9
    template<class U, impl::option::enable_constructor_9<T, U, /*is_explicit=*/false> = 0>
    constexpr option(option<U>&& other) {
        construct_from_option(std::move(other));
    }
    template<class U, impl::option::enable_constructor_9<T, U, /*is_explicit=*/true> = 0>
    constexpr explicit option(option<U>&& other) {
        construct_from_option(std::move(other));
    }

    // 1
    // postcondition: has_value() == false
    constexpr option& operator=(opt::none_t) noexcept(noexcept(reset())) {
        reset();
        return *this;
    }
    // 2
    // postcondition: has_value() == other.has_value()
    option& operator=(const option&) = default;
    // 3
    // postcondition: has_value() == other.has_value()
    option& operator=(option&&) = default;
    // 4
    // postcondition: has_value() == true
    template<class U = T, impl::option::enable_assigment_operator_4<T, U> = 0>
    constexpr option& operator=(U&& value) noexcept(impl::option::nothrow_assigment_operator_4<T, U>) {
        base::assign_from_value(std::forward<U>(value));
        return *this;
    }
    // 5
    template<class U, impl::option::enable_assigment_operator_5<T, U> = 0>
    constexpr option& operator=(const option<U>& other) {
        base::assign_from_option(other);
        return *this;
    }
    // 6
    template<class U, impl::option::enable_assigment_operator_6<T, U> = 0>
    constexpr option& operator=(option<U>&& other) {
        base::assign_from_option(std::move(other));
        return *this;
    }

    // postcondition: has_value() == false
    constexpr void reset() noexcept(std::is_nothrow_destructible_v<T>) {
        base::reset();
    }

    // postcondition: has_value() == true
    template<class... Args>
    constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...> && noexcept(reset())) {
        reset();
        base::construct(std::forward<Args>(args)...);
        return *(*this);
    }

    OPTION_CONST_ATTR
    constexpr bool has_value() const noexcept {
        return base::has_value();
    }
    OPTION_CONST_ATTR
    constexpr explicit operator bool() const noexcept { return has_value(); }

    template<class P>
    constexpr bool has_value_and(P&& predicate) & { return impl::option::has_value_and(*this, std::forward<P>(predicate)); }
    template<class P>
    constexpr bool has_value_and(P&& predicate) const& { return impl::option::has_value_and(*this, std::forward<P>(predicate)); }
    template<class P>
    constexpr bool has_value_and(P&& predicate) && { return impl::option::has_value_and(std::move(*this), std::forward<P>(predicate)); }
    template<class P>
    constexpr bool has_value_and(P&& predicate) const&& { return impl::option::has_value_and(std::move(*this), std::forward<P>(predicate)); }

    // postcondition: has_value() == false
    constexpr option<T> take() & noexcept(std::is_nothrow_copy_constructible_v<T> && noexcept(reset())) {
        auto tmp = *this;
        reset();
        return tmp;
    }
    constexpr option<T> take() && noexcept(std::is_nothrow_copy_constructible_v<T>) {
        return std::move(*this);
    }

    template<class P>
    constexpr option<T> take_if(P&& predicate) {
        if (has_value()) {
            if (std::invoke(std::forward<P>(predicate), get())) {
                return take();
            }
        }
        return opt::none;
    }

    constexpr T& insert(const raw_type& val) {
        this->emplace(val);
        return get();
    }
    constexpr T& insert(raw_type&& val) {
        this->emplace(std::move(val));
        return get();
    }

    template<class F>
    constexpr option& inspect(F&& f) & { return impl::option::inspect(*this, std::forward<F>(f)); }
    template<class F>
    constexpr const option& inspect(F&& f) const& { return impl::option::inspect(*this, std::forward<F>(f)); }
    template<class F>
    constexpr option&& inspect(F&& f) && { return impl::option::inspect(std::move(*this), std::forward<F>(f)); }
    template<class F>
    constexpr const option&& inspect(F&& f) const&& { return impl::option::inspect(std::move(*this), std::forward<F>(f)); }

    // precondition: has_value() == true
    constexpr T& get() & noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return base::get();
    }
    // precondition: has_value() == true
    constexpr const T& get() const& noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return base::get();
    }
    // precondition: has_value() == true
    constexpr raw_type&& get() && noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::move(base::get());
    }
    // precondition: has_value() == true
    constexpr const raw_type&& get() const&& noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::move(base::get());
    }
    // precondition: has_value() == true
    constexpr std::add_pointer_t<const T> operator->() const noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::addressof(get());
    }
    // precondition: has_value() == true
    constexpr std::add_pointer_t<T> operator->() noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::addressof(get());
    }
    // precondition: has_value() == true
    constexpr T& operator*() & noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return get();
    }
    // precondition: has_value() == true
    constexpr const T& operator*() const& noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return get();
    }
    // precondition: has_value() == true
    constexpr raw_type&& operator*() && noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::move(get());
    }
    // precondition: has_value() == true
    constexpr const raw_type&& operator*() const&& noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return std::move(get());
    }

    constexpr T& get_unchecked() & noexcept { return base::get(); }
    constexpr const T& get_unchecked() const& noexcept { return base::get(); }
    constexpr raw_type&& get_unchecked() && noexcept { return std::move(base::get()); }
    constexpr const raw_type&& get_unchecked() const&& noexcept { return std::move(base::get()); }

    // More verbose version of opt::option<T>::value() or std::option<T>::value()
    constexpr T& value_or_throw() & { return impl::option::value_or_throw(*this); }
    constexpr const T& value_or_throw() const& { return impl::option::value_or_throw(*this); }
    constexpr raw_type&& value_or_throw() && { return impl::option::value_or_throw(std::move(*this)); }
    constexpr const raw_type&& value_or_throw() const&& { return impl::option::value_or_throw(std::move(*this)); }

    // Same as std::optional<T>::value()
    constexpr T& value() & { return value_or_throw(); }
    constexpr const T& value() const& { return value_or_throw(); }
    constexpr raw_type&& value() && { return std::move(value_or_throw()); }
    constexpr const raw_type&& value() const&& { return std::move(value_or_throw()); }

    template<class U>
    constexpr T value_or(U&& default_value) const& noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_constructible_v<T, U&&>) {
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
        static_assert(std::is_convertible_v<U&&, T>, "U&& must be convertible to T");
        if (has_value()) {
            return *(*this);
        }
        return static_cast<T>(std::forward<U>(default_value));
    }
    template<class U>
    constexpr T value_or(U&& default_value) && noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_constructible_v<T, U&&>) {
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
        static_assert(std::is_convertible_v<U&&, T>, "U&& must be convertible to T");
        if (has_value()) {
            return std::move(*(*this));
        }
        return static_cast<T>(std::forward<U>(default_value));
    }

    constexpr T value_or_default() const& {
        static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
        static_assert(std::is_copy_constructible_v<T>, "T must be copy constructible");
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
        if (has_value()) {
            return get();
        }
        return T{};
    }
    constexpr T value_or_default() && {
        static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
        if (has_value()) {
            return std::move(get());
        }
        return T{};
    }

    template<class U, class F>
    constexpr auto map_or(U&& def, F&& f) & { return impl::option::map_or<T>(*this, std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F>
    constexpr auto map_or(U&& def, F&& f) const& { return impl::option::map_or<T>(*this, std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F>
    constexpr auto map_or(U&& def, F&& f) && { return impl::option::map_or<T>(std::move(*this), std::forward<U>(def), std::forward<F>(f)); }
    template<class U, class F>
    constexpr auto map_or(U&& def, F&& f) const&& { return impl::option::map_or<T>(std::move(*this), std::forward<U>(def), std::forward<F>(f)); }

    template<class D, class F>
    constexpr auto map_or_else(D&& def, F&& f) & { return impl::option::map_or_else<T>(*this, std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F>
    constexpr auto map_or_else(D&& def, F&& f) const& { return impl::option::map_or_else<T>(*this, std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F>
    constexpr auto map_or_else(D&& def, F&& f) && { return impl::option::map_or_else<T>(std::move(*this), std::forward<D>(def), std::forward<F>(f)); }
    template<class D, class F>
    constexpr auto map_or_else(D&& def, F&& f) const&& { return impl::option::map_or_else<T>(std::move(*this), std::forward<D>(def), std::forward<F>(f)); }

    constexpr std::remove_reference_t<T>* ptr_or_null() & noexcept {
        return has_value() ? std::addressof(get()) : nullptr;
    }
    constexpr const std::remove_reference_t<T>* ptr_or_null() const& noexcept {
        return has_value() ? std::addressof(get()) : nullptr;
    }
    constexpr void ptr_or_null() && = delete;
    constexpr void ptr_or_null() const&& = delete;

    // filter(F&&) -> option<T> : F(const T&) -> bool
    template<class F>
    constexpr option<T> filter(F&& f) const {
        if (has_value()) {
            // f(*this) can return an object that can be explicitly converted to bool
            if (std::invoke(std::forward<F>(f), get())) {
                return get();
            }
        }
        return {};
    }

    constexpr auto flatten() const& { return impl::option::flatten(*this); }
    constexpr auto flatten() && { return impl::option::flatten(std::move(*this)); }

    // and_then(F&&) -> option<U> : F(option<T>) -> option<U>
    template<class F>
    constexpr auto and_then(F&& f) & { return impl::option::and_then(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) const& { return impl::option::and_then(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) && { return impl::option::and_then(std::move(*this), std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) const&& { return impl::option::and_then(std::move(*this), std::forward<F>(f)); }

    // map(F&&) -> option<U> : F(option<T>) -> U
    template<class F>
    constexpr auto map(F&& f) & { return impl::option::map<T>(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) const& { return impl::option::map<T>(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) && { return impl::option::map<T>(std::move(*this), std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) const&& { return impl::option::map<T>(std::move(*this), std::forward<F>(f)); }

    // or_else(F&&) -> option<T> : F() -> option<T>
    template<class F>
    constexpr option or_else(F&& f) const& { return impl::option::or_else<T>(*this, std::forward<F>(f)); }
    template<class F>
    constexpr option or_else(F&& f) && { return impl::option::or_else<T>(std::move(*this), std::forward<F>(f)); }

    constexpr void assume_has_value() const noexcept {
        OPTION_VERIFY(has_value(), "Assumption 'has_value()' failed");
    }

    constexpr auto unzip() & { return impl::option::unzip(*this); }
    constexpr auto unzip() const& { return impl::option::unzip(*this); }
    constexpr auto unzip() && { return impl::option::unzip(std::move(*this)); }
    constexpr auto unzip() const&& { return impl::option::unzip(std::move(*this)); }

    template<class U>
    constexpr option<T> replace(U&& val) & {
        option<T> tmp{std::move(*this)};
        // should call the destructor after moving, because moving does not end lifetime
        base::reset();
        base::construct(std::forward<U>(val));
        return tmp; // copy elision
    }

private:
    template<class Option>
    constexpr void construct_from_option(Option&& other) {
        if (other.has_value()) {
            base::construct(std::forward<Option>(other).get());
        }
    }
};

template<class T>
option(T) -> option<T>;

template<class T>
option(option<T>) -> option<option<T>>;

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

template<class Fn, class... Options, std::enable_if_t<
    (opt::is_option<impl::remove_cvref<Options>> && ...)
, int> = 0>
constexpr auto zip_with(Fn&& fn, Options&&... options)
    -> opt::option<std::invoke_result_t<Fn, decltype(std::forward<Options>(options).get())...>>
{
    if ((options.has_value() && ...)) {
        return std::invoke(std::forward<Fn>(fn), std::forward<Options>(options).get()...);
    } else {
        return {};
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

// x = left option value
// y = right value
// N = empty option (none)
//| left | right | result |
//|   x  |   y   |    x   |
//|   N  |   y   |    y   |
template<class T>
constexpr T operator|(const opt::option<T>& left, const T& right) {
    return left.value_or(right);
}
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
// x = left option value
// N = empty option (none)
//| left | right | result |
//|   x  |   N   |   x    |
//|   N  |   N   |   N    |
template<class T>
constexpr opt::option<T> operator|(const opt::option<T>& left, none_t) {
    return left;
}
// y = right option value
// N = empty option (none)
//| left | right | result |
//|   N  |   y   |   y    |
//|   N  |   N   |   N    |
template<class T>
constexpr opt::option<T> operator|(none_t, const opt::option<T>& right) {
    return right;
}

template<class T>
constexpr opt::option<T>& operator|=(opt::option<T>& left, const opt::option<T>& right) {
    if (!left.has_value()) {
        left = right;
    }
    return left;
}
template<class T>
constexpr opt::option<T>& operator|=(opt::option<T>& left, opt::option<T>&& right) {
    if (!left.has_value()) {
        left = std::move(right);
    }
    return left;
}
template<class T>
constexpr opt::option<T>& operator|=(opt::option<T>& left, const T& right) {
    if (!left.has_value()) {
        left = right;
    }
    return left;
}
template<class T>
constexpr opt::option<T>& operator|=(opt::option<T>& left, T&& right) {
    if (!left.has_value()) {
        left = std::move(right);
    }
    return left;
}

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

template<>
struct option_flag<opt::option<bool>> {
    static constexpr std::uint_least8_t empty_value = 0b0100;

    static bool is_empty(const opt::option<bool>& value) noexcept {
        const auto uint_value = impl::bit_cast<std::uint_least8_t>(value.get_unchecked());
        return uint_value & empty_value;
    }
    static void construct_empty_flag(opt::option<bool>& value) noexcept {
        auto uint_value = impl::bit_cast<std::uint_least8_t>(value.get_unchecked());
        uint_value |= empty_value;
        impl::bit_copy(value.get_unchecked(), uint_value);
    }
};

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
struct std::hash<opt::impl::first_type<opt::option<T>,
    std::enable_if_t<std::is_default_constructible_v<std::hash<std::remove_const_t<T>>>>>> {
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
