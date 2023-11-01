#pragma once

#include <type_traits>
#include <utility>
#include <cstdint>
#include <memory>
#include <new>
#include <functional>
#include <exception>

namespace opt {

namespace impl {
    struct none_tag_ctor {};
}

struct none_t {
    constexpr explicit none_t(impl::none_tag_ctor) {}
};
inline constexpr none_t none{impl::none_tag_ctor{}};

namespace impl {
    enum class no_sentinel_t {};

    template<class T>
    struct sentinel_maker {
        static constexpr no_sentinel_t value{};
    };
    template<> struct sentinel_maker<bool> {
        static constexpr std::uint_least8_t value = 2;
    };

}

template<class T, auto sentinel = impl::sentinel_maker<T>::value>
class option;

namespace impl {
    template<class T>
    using remove_cvref = std::remove_cv_t<std::remove_reference_t<T>>;

    struct nontrivial_dummy_t {
        constexpr nontrivial_dummy_t() noexcept {}
    };

    template<class T, auto sentinel,
        bool store_state = std::is_same_v<decltype(sentinel), no_sentinel_t>,
        bool is_trivially_destructible = std::is_trivially_destructible_v<T>>
    struct option_base;

    template<class T, auto sentinel>
    struct option_base<T, sentinel, /*store_state=*/true, /*is_trivially_destructible=*/true> {
        constexpr option_base() noexcept : dummy(), has_value(false) {}

        template<class... Args>
        constexpr option_base(Args&&... args)
            noexcept(std::is_nothrow_constructible_v<T, Args...>)
            : value(std::forward<Args>(args)...), has_value(true) {}

        constexpr void reset() noexcept {
            has_value = false;
        }

        union {
            nontrivial_dummy_t dummy;
            T value;
        };
        bool has_value;
    };
    template<class T, auto sentinel>
    struct option_base<T, sentinel, /*store_state=*/true, /*is_trivially_destructible=*/false> {
        constexpr option_base() noexcept : dummy(), has_value(false) {}

        template<class... Args>
        constexpr option_base(Args&&... args)
            : value(std::forward<Args>(args)...), has_value(true) {}

        ~option_base() {
            if (has_value) {
                value.~T();
            }
        }

        constexpr option_base(const option_base&) = default;
        constexpr option_base(option_base&&) = default;
        constexpr option_base& operator=(const option_base&) = default;
        constexpr option_base& operator=(option_base&&) = default;
        
        constexpr void reset() noexcept(std::is_nothrow_destructible_v<T>) {
            if (has_value) {
                value.~T();
                has_value = false;
            }
        }

        union {
            nontrivial_dummy_t dummy;
            T value;
        };
        bool has_value;
    };

    template<class T, class... Args>
    constexpr void construct_at(T& obj, Args&&... args) {
        ::new(static_cast<void*>(std::addressof(obj))) T(std::forward<Args>(args)...);
    }

    template<class T>
    inline constexpr bool is_option_specialization = false;
    template<class T, auto Value>
    inline constexpr bool is_option_specialization<opt::option<T, Value>> = true;
}

class bad_access : public std::exception {
public:
    bad_access() noexcept = default;
    bad_access(const bad_access&) noexcept = default;

    const char* what() const noexcept override {
        return "Bad opt::option access";
    }
};

template<class T, auto sentinel>
class option : private impl::option_base<T, sentinel> {
    using base = impl::option_base<T, sentinel>;
public:

    constexpr option() noexcept : base() {}
    constexpr option(opt::none_t) noexcept : base() {}

    constexpr option(const option& other) noexcept(std::is_nothrow_constructible_v<T, const T&>) {
        if (other) {
            impl::construct_at(base::value, *other);
        }
    }
    constexpr option(option&& other) noexcept(std::is_nothrow_constructible_v<T, T&&>) {
        if (other) {
            impl::construct_at(base::value, std::move(*other));
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

    template<class First, class... Args, std::enable_if_t<std::is_constructible_v<T, First, Args...>, int> = 0>
    constexpr option(First&& first, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, First, Args...>)
        : base(std::forward<First>(first), std::forward<Args>(args)...) {}

    constexpr void reset() noexcept {
        base::reset();
    }

    template<class... Args>
    constexpr T& emplace(Args&&... args) {
        reset();
        impl::construct_at(base::value, std::forward<Args>(args)...);
        base::has_value = true;
        return *(*this);
    }

    constexpr bool has_value() const noexcept {
        return base::has_value;
    }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    constexpr const T* operator->() const noexcept {
        return std::launder(&base::value);
    }
    constexpr T* operator->() noexcept {
        return std::launder(&this->base::value);
    }
    constexpr const T& operator*() const& noexcept {
        return *std::launder(&this->base::value);
    }
    constexpr T& operator*() & noexcept {
        return *std::launder(&this->base::value);
    }
    constexpr const T&& operator*() const&& noexcept {
        return std::move(*std::launder(&this->base::value));
    }
    constexpr T&& operator*() && noexcept {
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
    constexpr auto and_then(F&& f) & { return and_then_impl(**this, std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) const& { return and_then_impl(**this, std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) && { return and_then_impl(std::move(**this), std::forward<F>(f)); }
    template<class F>
    constexpr auto and_then(F&& f) const&& { return and_then_impl(std::move(**this), std::forward<F>(f)); }

private:
    template<class Value, class F>
    constexpr auto and_then_impl(Value&& val, F&& f) {
        using invoke_res = impl::remove_cvref<std::invoke_result_t<F, Value>>;
        static_assert(impl::is_option_specialization<invoke_res>);
        if (has_value()) {
            return std::invoke(std::forward<F>(f), std::forward<Value>(val));
        } else {
            return impl::remove_cvref<invoke_res>{};
        }
    }
public:

    template<class F>
    constexpr auto map(F&& f) & { return map_impl(**this, std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) const& { return map_impl(**this, std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) && { return map_impl(std::move(**this), std::forward<F>(f)); }
    template<class F>
    constexpr auto map(F&& f) const&& { return map_impl(std::move(**this), std::forward<F>(f)); }

private:
    template<class V, class F>
    constexpr auto map_impl(V&& val, F&& f) {
        using invoke_res = impl::remove_cvref<std::invoke_result_t<F, V>>;
        if (has_value()) {
            return opt::option<invoke_res>{std::invoke(std::forward<F>(f), std::forward<V>(val))};
        } else {
            return opt::option<invoke_res>{opt::none};
        }
    }

private:

    template<class U>
    constexpr void _assign(U&& other) {
        if (has_value()) {
            this->base::value = std::forward<U>(other);
        } else {
            emplace(std::forward<U>(other));
        }
        this->base::has_value = true;
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
constexpr bool operator<(const option<T>& left, none_t) noexcept {
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
constexpr bool operator<=(none_t, const option<T>& right) noexcept {
    return true;
}
template<class T>
constexpr bool operator>(const option<T>& left, none_t) noexcept {
    return left.has_value();
}
template<class T>
constexpr bool operator>(none_t, const option<T>& right) noexcept {
    return false;
}
template<class T>
constexpr bool operator>=(const option<T>& left, none_t) noexcept {
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
