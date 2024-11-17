#pragma once

// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <opt/option.hpp>
#include <doctest/doctest.h>
#include <cfenv>

#if __has_include(<libassert/assert.hpp>)
#include <libassert/assert.hpp>

inline void libassert_failure_handler(const libassert::assertion_info& info) {
    libassert::enable_virtual_terminal_processing_if_needed(); // for terminal colors on windows
    const std::string message = info.to_string(
        libassert::terminal_width(libassert::stderr_fileno),
        libassert::isatty(libassert::stderr_fileno) ? libassert::get_color_scheme() : libassert::color_scheme::blank
    );
    const std::string file_name{info.file_name};
    ADD_FAIL_CHECK_AT(file_name.c_str(), int(info.line), message);
}

inline const auto set_libassert_handle = []() {
    libassert::set_failure_handler(&libassert_failure_handler);
    return 1;
} ();
#endif

template<class T>
struct doctest::StringMaker<opt::option<T>> {
    static doctest::String convert(const opt::option<T>& value) {
        if (value.has_value()) {
            return doctest::toString(value.get());
        } else {
            unsigned char bytes[sizeof(value)]{};
            std::memcpy(&bytes, &value, sizeof(value));
            return "[empty] (" + doctest::toString(bytes) + ")";
        }
    }
};
template<>
struct doctest::StringMaker<opt::none_t> {
    static doctest::String convert(const opt::none_t&) {
        return "[empty]";
    }
};
template<class T, auto... Vals>
struct doctest::StringMaker<opt::sentinel<T, Vals...>> {
    static doctest::String convert(const opt::sentinel<T, Vals...>& value) {
        return doctest::toString(static_cast<const T&>(value));
    }
};

template<class T>
constexpr std::remove_reference_t<T>&& as_rvalue(T&& x) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(x);
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
    ~nontrivial_struct() noexcept(false) {}
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

struct fp_exception_checker {
    fp_exception_checker() {
        REQUIRE_EQ(std::feclearexcept(FE_ALL_EXCEPT), 0);
    }
    ~fp_exception_checker() {
        CHECK_EQ(std::fetestexcept(FE_DIVBYZERO), 0);
        CHECK_EQ(std::fetestexcept(FE_INEXACT), 0);
        CHECK_EQ(std::fetestexcept(FE_INVALID), 0);
        CHECK_EQ(std::fetestexcept(FE_OVERFLOW), 0);
        CHECK_EQ(std::fetestexcept(FE_UNDERFLOW), 0);

        REQUIRE_EQ(std::feclearexcept(FE_ALL_EXCEPT), 0);
    }
};

template<class F>
OPTION_NO_SANITIZE_OBJECT_SIZE
constexpr void no_sanitize_object_size_invoke(F&& f) {
    static_cast<F&&>(f)();
}

template<class T>
inline constexpr std::size_t true_sizeof = std::is_reference_v<T> ? sizeof(std::remove_reference_t<T>*) : sizeof(T);
