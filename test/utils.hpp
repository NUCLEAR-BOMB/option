#pragma once

#include <option.hpp>

namespace opt {
    template<class T>
    void PrintTo(const opt::option<T>& value, ::std::ostream* os) { // NOLINT(readability-identifier-naming)
        if (value) {
            *os << *value;
        } else {
            *os << "[empty]";
        }
    }
}

template<class T>
constexpr std::remove_reference_t<T>&& as_rvalue(T&& x) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(x);
}

// https://stackoverflow.com/a/67059296
template<class T>
constexpr T& as_lvalue(T&& x) {
    return static_cast<T&>(x);
}

template<class T>
constexpr const std::remove_reference_t<T>&& as_const_rvalue(T&& x) noexcept {
    return static_cast<const std::remove_reference_t<T>&&>(x);
}

using std::as_const;

template<class T>
std::size_t hash_fn(const T& x) { return std::hash<T>{}(x); }

// NOLINTBEGIN
struct nontrivial_struct {
    ~nontrivial_struct() {}
    nontrivial_struct(const nontrivial_struct&) {}
    nontrivial_struct(nontrivial_struct&&) {}
    nontrivial_struct& operator=(const nontrivial_struct&) { return *this; }
    nontrivial_struct& operator=(nontrivial_struct&&) { return *this; }
};
// NOLINTEND

template<class T>
inline constexpr bool is_trivial_compatible =
    std::is_trivially_destructible_v<T> &&
    std::is_trivially_copy_constructible_v<T> &&
    std::is_trivially_move_constructible_v<T> &&
    std::is_trivially_copy_assignable_v<T> &&
    std::is_trivially_move_assignable_v<T>;

template<class T>
inline constexpr bool is_not_trivial_compatible =
    !std::is_trivially_destructible_v<T> &&
    !std::is_trivially_copy_constructible_v<T> &&
    !std::is_trivially_move_constructible_v<T> &&
    !std::is_trivially_copy_assignable_v<T> &&
    !std::is_trivially_move_assignable_v<T>;
