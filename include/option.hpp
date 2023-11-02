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
    template<class T, class Int, Int unused_value>
    struct option_flag_for_unused_value {
        static constexpr Int empty_value = unused_value;
        static_assert(sizeof(T) == sizeof(Int));
        static void reset(T& value) noexcept {
            std::memcpy(&value, &empty_value, sizeof(T));
        }
        static bool has_value(const T& value) noexcept {
            return std::memcmp(&value, &empty_value, sizeof(T)) != 0;
        }
    };
}

template<> struct option_flag<bool> : impl::option_flag_for_unused_value<bool, std::uint8_t, 2> {};

namespace impl {
    template<class T>
    using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

    struct nontrivial_dummy_t {
        constexpr nontrivial_dummy_t() noexcept {}
    };

    template<class T, class = std::size_t>
    inline constexpr bool has_option_flag = false;
    template<class T>
    inline constexpr bool has_option_flag<T, decltype(sizeof(opt::option_flag<T>))> = true;


    template<class T, class... Args>
    constexpr void construct_at(T& obj, Args&&... args) {
        ::new(static_cast<void*>(std::addressof(obj))) T(std::forward<Args>(args)...);
    }

    template<class T, bool store_flag = !has_option_flag<T>, bool is_trivially_destructible = std::is_trivially_destructible_v<T>>
    struct option_base;

    template<class T>
    struct option_base<T, /*store_flag=*/true, /*is_trivially_destructible=*/true> {
        constexpr option_base() noexcept : dummy(), has_value_flag(false) {}

        template<class... Args>
        constexpr option_base(Args&&... args)
            noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : value(std::forward<Args>(args)...), has_value_flag(true) {}

        template<class... Args>
        constexpr void construct(Args&&... args) {
            impl::construct_at(value, std::forward<Args>(args)...);
            has_value_flag = true;
        }

        constexpr void reset() noexcept {
            has_value_flag = false;
        }
        constexpr bool has_value() const noexcept {
            return has_value_flag;
        }

        union {
            nontrivial_dummy_t dummy;
            T value;
        };
    private:
        bool has_value_flag;
    };
    template<class T>
    struct option_base<T, /*store_flag=*/true, /*is_trivially_destructible=*/false> {
        constexpr option_base() noexcept : dummy(), has_value_flag(false) {}

        template<class... Args>
        constexpr option_base(Args&&... args)
            : value(std::forward<Args>(args)...), has_value_flag(true) {}

        ~option_base() {
            if (has_value()) {
                value.~T();
            }
        }

        constexpr option_base(const option_base&) = default;
        constexpr option_base(option_base&&) = default;
        constexpr option_base& operator=(const option_base&) = default;
        constexpr option_base& operator=(option_base&&) = default;
        
        constexpr void reset() noexcept(std::is_nothrow_destructible_v<T>) {
            if (has_value()) {
                value.~T();
                has_value_flag = false;
            }
        }
        template<class... Args>
        constexpr void construct(Args&&... args) {
            impl::construct_at(value, std::forward<Args>(args)...);
            has_value_flag = true;
        }

        constexpr bool has_value() const noexcept {
            return has_value_flag;
        }

        union {
            nontrivial_dummy_t dummy;
            T value;
        };
    private:
        bool has_value_flag;
    };
    template<class T>
    struct option_base<T, /*store_flag=*/false, /*is_trivially_destructible=*/true> {
        using opt_flag = opt::option_flag<T>;

        constexpr option_base() noexcept {
            opt_flag::reset(value);
        }
        template<class... Args>
        constexpr option_base(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : value(std::forward<Args>(args)...) {}

        constexpr bool has_value() const noexcept {
            return opt_flag::has_value(value);
        }

        constexpr void reset() noexcept {
            opt_flag::reset(value);
        }

        template<class... Args>
        constexpr void construct(Args&&... args) {
            impl::construct_at(value, std::forward<Args>(args)...);
        }


        T value;
    };


    template<class T>
    inline constexpr bool is_option_specialization = false;
    template<class T>
    inline constexpr bool is_option_specialization<opt::option<T>> = true;
}

class bad_access : public std::exception {
public:
    bad_access() noexcept = default;
    bad_access(const bad_access&) noexcept = default;

    const char* what() const noexcept override {
        return "Bad opt::option access";
    }
};

template<class T>
class option : private impl::option_base<T> {
    using base = impl::option_base<T>;
public:

    constexpr option() noexcept : base() {}
    constexpr option(opt::none_t) noexcept : base() {}

    constexpr option(const option& other) noexcept(std::is_nothrow_constructible_v<T, const T&>) {
        if (other) {
            base::construct(*other);
        }
    }
    constexpr option(option&& other) noexcept(std::is_nothrow_constructible_v<T, T>) {
        if (other) {
            base::construct(std::move(*other));
        }
    }

    constexpr option& operator=(opt::none_t) noexcept {
        reset();
        return *this;
    }
    constexpr option& operator=(const option& other) noexcept(std::is_nothrow_constructible_v<T, const T&> && std::is_nothrow_assignable_v<T&, const T&>) {
        if (other) {
            _assign(*other);
        } else {
            reset();
        }
        return *this;
    }
    constexpr option& operator=(option&& other) noexcept(std::is_nothrow_constructible_v<T, T> && std::is_nothrow_assignable_v<T&, T>) {
        if (other) {
            _assign(std::move(*other));
        } else {
            reset();
        }
        return *this;
    }
    template<class U = T, std::enable_if_t<
        !std::is_same_v<option, impl::remove_cvref<U>>
        && (!std::is_scalar_v<T> || !std::is_same_v<T, std::decay_t<U>>)
        && std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>
    , int> = 0>
    constexpr option& operator=(U&& value) noexcept(std::is_nothrow_assignable_v<T&, U> && std::is_nothrow_constructible_v<T, U>) {
        _assign(std::forward<U>(value));
        return *this;
    }

    template<class First, class... Args, std::enable_if_t<
        std::is_constructible_v<T, First, Args...> && !std::is_same_v<impl::remove_cvref<First>, option>
    , int> = 0>
    constexpr option(First&& first, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, First, Args...>)
        : base(std::forward<First>(first), std::forward<Args>(args)...) {}

    constexpr void reset() noexcept {
        base::reset();
    }

    template<class... Args>
    constexpr T& emplace(Args&&... args) {
        reset();
        base::construct(std::forward<Args>(args)...);
        return *(*this);
    }

    constexpr bool has_value() const noexcept {
        return base::has_value();
    }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    constexpr option take() & noexcept(std::is_nothrow_constructible_v<T, const T&>) {
        auto tmp = *this;
        reset();
        return tmp;
    }
    constexpr option take() && noexcept(std::is_nothrow_constructible_v<T, const T&>) {
        return *this;
    }

    constexpr const T* operator->() const noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option");
        return std::launder(&base::value);
    }
    constexpr T* operator->() noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option");
        return std::launder(&this->base::value);
    }
    constexpr const T& operator*() const& noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option");
        return *std::launder(&this->base::value);
    }
    constexpr T& operator*() & noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option");
        return *std::launder(&this->base::value);
    }
    constexpr const T&& operator*() const&& noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option");
        return std::move(*std::launder(&this->base::value));
    }
    constexpr T&& operator*() && noexcept {
        OPTION_VERIFY(has_value(), "Accessing the value of an empty opt::option");
        return std::move(*std::launder(&this->base::value));
    }

    constexpr T& value_or_throw() & {
        if (!has_value()) { throw bad_access{}; }
        return *(*this);
    }
    constexpr const T& value_or_throw() const& {
        if (!has_value()) { throw bad_access{}; }
        return *(*this);
    }
    constexpr T&& value_or_throw() && {
        if (!has_value()) { throw bad_access{}; }
        return std::move(*(*this));
    }
    constexpr const T&& value_or_throw() const&& {
        if (!has_value()) { throw bad_access{}; }
        return std::move(*(*this));
    }

    template<class U>
    constexpr T value_or(U&& default_value) const& {
        if (has_value()) {
            return *(*this);
        }
        return static_cast<T>(std::forward<U>(default_value));
    }
    template<class U>
    constexpr T value_or(U&& default_value) && {
        if (has_value()) {
            return std::move(*(*this));
        }
        return static_cast<T>(std::forward<U>(default_value));
    }

    template<class F>
    constexpr auto and_then(F&& f) & { return and_then_impl(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) const& { return and_then_impl(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) && { return and_then_impl(std::move(*this), std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) const&& { return and_then_impl(std::move(*this), std::forward<F>(f)); }

private:
    template<class Self, class F>
    constexpr auto and_then_impl(Self&& self, F&& f) {
        using invoke_res = impl::remove_cvref<std::invoke_result_t<F, decltype(*std::forward<Self>(self))>>;
        static_assert(impl::is_option_specialization<invoke_res>);
        if (has_value()) {
            return std::invoke(std::forward<F>(f), *std::forward<Self>(self));
        } else {
            return impl::remove_cvref<invoke_res>{};
        }
    }
public:

    template<class F>
    constexpr auto map(F&& f) & { return map_impl(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) const& { return map_impl(*this, std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) && { return map_impl(std::move(*this), std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) const&& { return map_impl(std::move(*this), std::forward<F>(f)); }

private:
    template<class Self, class F>
    constexpr auto map_impl(Self&& self, F&& f) {
        using invoke_res = impl::remove_cvref<std::invoke_result_t<F, decltype(*std::forward<Self>(self))>>;
        if (has_value()) {
            return opt::option<invoke_res>{std::invoke(std::forward<F>(f), *std::forward<Self>(self))};
        } else {
            return opt::option<invoke_res>{opt::none};
        }
    }
public:


    template<class F>
    constexpr option or_else(F&& f) const& noexcept(noexcept(std::is_nothrow_invocable_v<F>)) {
        return has_value() ? *this : std::invoke(std::forward<F>(f));
    }
    template<class F>
    constexpr option or_else(F&& f) && noexcept(noexcept(std::is_nothrow_invocable_v<F>)) {
        return has_value() ? std::move(*this) : std::invoke(std::forward<F>(f));
    }

private:

    template<class U>
    constexpr void _assign(U&& other) {
        if (has_value()) {
            this->base::value = std::forward<U>(other);
        } else {
            base::construct(std::forward<U>(other));
        }
    }
};

namespace impl {
    template<class Op, class T1, class T2>
    constexpr bool do_option_comparison(const option<T1>& left, const option<T2>& right) {
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
    constexpr bool do_option_comparison_with_value(const option<T1>& left, const T2& right) {
        if (left.has_value()) {
            return Op{}(*left, right);
        }
        return if_hasnt_value;
    }
    template<class Op, bool if_hasnt_value, class T1, class T2>
    constexpr bool do_option_comparison_with_value(const T1& left, const option<T2>& right) {
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
