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
