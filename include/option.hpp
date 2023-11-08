#pragma once

#include <type_traits>
#include <utility>
#include <cstdint>
#include <memory>
#include <new>
#include <functional>
#include <exception>
#include <cstring>

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
        #define OPTION_VERIFY(expression, message) \
            ((expression) ? (void)0 : ( \
                (void)std::fprintf(stderr, "%s:%d: assertion '%s' failed: %s\n", __FILE__, __LINE__, #expression, message), \
                (void)OPTION_DEBUG_BREAK) \
            )
    #else
        #define OPTION_VERIFY(expression, message) ((void)0)
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

template<class T>
struct option_flag;

namespace impl {
    template<class T>
    inline constexpr bool is_bit_representable = std::is_unsigned_v<T> && !std::is_same_v<T, bool>;

    template<class Left, class Right>
    constexpr int bit_cmp(const Left& left, const Right& right) noexcept {
        static_assert(sizeof(Left) == sizeof(Right));
        if constexpr (impl::is_bit_representable<Left> && impl::is_bit_representable<Right>) {
            return left == right ? 0 : left - right;
        } else {
            return std::memcmp(&left, &right, sizeof(Left));
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

    template<class T, auto sentinel>
    class sentinel_option_flag {
        static_assert(sizeof(T) == sizeof(sentinel));
        static constexpr auto empty_value = sentinel; // for IntelliSense natvis
    public:
        static bool is_empty(const T& value) noexcept {
            return impl::bit_cmp(value, sentinel) == 0;
        }
        static void construct_empty_flag(T& value) noexcept {
            impl::bit_copy(value, sentinel);
        }
        static constexpr void destroy_empty_flag(T&) noexcept {}
    };
}

template<> struct option_flag<bool> : impl::sentinel_option_flag<bool, std::uint8_t{2}> {};

namespace impl {
    template<class T>
    using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

    struct nontrivial_dummy_t {
        constexpr nontrivial_dummy_t() noexcept {}
    };

    struct construct_from_invoke_tag {
        explicit construct_from_invoke_tag() = default;
    };

    template<class T, class = std::size_t>
    inline constexpr bool has_option_flag = false;
    template<class T>
    inline constexpr bool has_option_flag<T, decltype(sizeof(opt::option_flag<T>))> = true;

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
            flag::destroy_empty_flag(value);
            // has_value() == true
        }
        template<class F, class Arg>
        constexpr option_destruct_base(construct_from_invoke_tag, F&& f, Arg&& arg)
            : value(std::invoke(std::forward<F>(f), std::forward<Arg>(arg))) {
            flag::destroy_empty_flag(value);
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
            flag::destroy_empty_flag(value);
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

    template<class T, bool is_reference = std::is_reference_v<T>> // is_reference=false
    class option_storage_base : private option_destruct_base<T> {
        using base = option_destruct_base<T>;
    public:
        using base::base;
        using base::has_value;
        using base::reset;
        using base::construct;

        constexpr T& get() & noexcept { return base::value; }
        constexpr const T& get() const& noexcept { return base::value; }
        constexpr T&& get() && noexcept { return base::value; }
        constexpr const T&& get() const&& noexcept { return base::value; }

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
        constexpr option_storage_base() noexcept
            : value(nullptr) {}

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

    template<class T, bool /*true*/ = std::is_trivially_copy_constructible_v<T>>
    struct option_copy_base : option_storage_base<T> {
        using option_storage_base<T>::option_storage_base;
    };
    template<class T>
    struct option_copy_base<T, false> : option_storage_base<T> {
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
    template<class T, bool /*true*/ = std::is_trivially_move_constructible_v<T>>
    struct option_move_base : option_copy_base<T> {
        using option_copy_base<T>::option_copy_base;
    };
    template<class T>
    struct option_move_base<T, false> : option_copy_base<T> {
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
    template<class T, bool /*true*/ =
        std::is_trivially_copy_assignable_v<T> || std::is_reference_v<T>>
    struct option_copy_assign_base : option_move_base<T> {
        using option_move_base<T>::option_move_base;
    };
    template<class T>
    struct option_copy_assign_base<T, false> : option_move_base<T> {
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
    template<class T, bool /*true*/ =
        std::is_trivially_move_assignable_v<T> || std::is_reference_v<T>>
    struct option_move_assign_base : option_copy_assign_base<T> {
        using option_copy_assign_base<T>::option_copy_assign_base;
    };
    template<class T>
    struct option_move_assign_base<T, false> : option_copy_assign_base<T> {
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



    template<class T>
    inline constexpr bool is_option_specialization = false;
    template<class T>
    inline constexpr bool is_option_specialization<opt::option<T>> = true;

    template<class T>
    inline constexpr bool is_cv_bool = std::is_same_v<T, std::remove_cv_t<T>>;
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
        && !(std::is_same_v<impl::remove_cvref<T>, bool> && impl::is_option_specialization<impl::remove_cvref<U>>)
        && (std::is_convertible_v<U&&, T> == !is_explicit) // explicit( condition )
    , int>;

    template<class T, class First, class... Args>
    using enable_constructor_6 = std::enable_if_t<
        std::is_constructible_v<T, First, Args...> && !std::is_same_v<impl::remove_cvref<First>, opt::option<T>>
    , int>;

    template<class T, class U>
    using enable_assigment_operator_4 = std::enable_if_t<
           !is_option_specialization<U>
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
        static_assert(impl::is_option_specialization<invoke_res>);
        if (self.has_value()) {
            return std::invoke(std::forward<F>(f), *std::forward<Self>(self));
        } else {
            return impl::remove_cvref<invoke_res>{opt::none};
        }
    }
    // implementation of opt::option<T>::map(F&&)
    // map(F&&) -> option<U> : F(T&&) -> U
    template<class Self, class F>
    constexpr auto map(Self&& self, F&& f) {
        using U = std::remove_cv_t<decltype(std::forward<F>(f)(*std::forward<Self>(self)))>;
        if (self.has_value()) {
            return opt::option<U>{construct_from_invoke_tag{}, std::forward<F>(f), *std::forward<Self>(self)};
        }
        return opt::option<U>{opt::none};
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
}

template<class T>
class option : private impl::option_move_assign_base<T> {
    using base = impl::option_move_assign_base<T>;
    using raw_type = std::remove_reference_t<T>;
public:
    using value_type = T;

    // 1
    // postcondition: has_value() == false
    constexpr option() noexcept : base() {}
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

    constexpr bool has_value() const noexcept {
        return base::has_value();
    }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    // postcondition: has_value() == false
    constexpr option take() & noexcept(std::is_nothrow_copy_constructible_v<T> && noexcept(reset())) {
        auto tmp = *this;
        reset();
        return tmp;
    }
    constexpr option take() && noexcept(std::is_nothrow_copy_constructible_v<T>) {
        return *this;
    }

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
        return &get();
    }
    // precondition: has_value() == true
    constexpr std::add_pointer_t<T> operator->() noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option<T>");
        return &get();
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
    constexpr auto map(F&& f) & { return impl::option::map(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) const& { return impl::option::map(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) && { return impl::option::map(std::move(*this), std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) const&& { return impl::option::map(std::move(*this), std::forward<F>(f)); }

    // or_else(F&&) -> option<T> : F() -> option<T>
    template<class F>
    constexpr option or_else(F&& f) const& { return impl::option::or_else<T>(*this, std::forward<F>(f)); }
    template<class F>
    constexpr option or_else(F&& f) && { return impl::option::or_else<T>(std::move(*this), std::forward<F>(f)); }
private:
    template<class Option>
    constexpr void construct_from_option(Option&& other) {
        if (other.has_value()) {
            base::construct(*std::forward<Option>(other));
        }
    }
};

template<class T>
option(T) -> option<T>;

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

namespace impl {
    template<class T, class...>
    using first_type = T;
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
