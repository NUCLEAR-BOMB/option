
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>
#include <opt/option.hpp>
#include <type_traits>

namespace {

TEST_SUITE_BEGIN("meta");

// NOLINTBEGIN(modernize-use-equals-default,performance-noexcept-move-constructor,cert-oop54-cpp)

struct non_default_constructible {
    non_default_constructible() = delete;
};
static_assert(!std::is_default_constructible_v<non_default_constructible>);

struct non_copy_constructible {
    non_copy_constructible(const non_copy_constructible&) = delete;
};
static_assert(!std::is_copy_constructible_v<non_copy_constructible>);

struct non_trivially_copy_constructible {
    non_trivially_copy_constructible(const non_trivially_copy_constructible&) {}
};
static_assert(!std::is_trivially_copy_constructible_v<non_trivially_copy_constructible>);

struct non_move_constructible {
    non_move_constructible(non_move_constructible&&) = delete;
};
static_assert(!std::is_move_constructible_v<non_move_constructible>);

struct non_trivially_move_constructible {
    non_trivially_move_constructible(non_trivially_move_constructible&&) {}
};
static_assert(!std::is_trivially_copy_constructible_v<non_trivially_move_constructible>);

struct non_constructible_from_anything {
    template<class... Args>
    non_constructible_from_anything(Args&&...) = delete;
};
static_assert(!std::is_constructible_v<non_constructible_from_anything, int>);
static_assert(std::is_constructible_v<non_constructible_from_anything, const non_constructible_from_anything&>);

struct explicit_constructible_from_anything {
    template<class... Args>
    explicit explicit_constructible_from_anything(Args&&...) {}
};
static_assert(std::is_constructible_v<explicit_constructible_from_anything, int>);
static_assert(!std::is_convertible_v<int, explicit_constructible_from_anything>);

struct constructible_from_anything {
    template<class... Args>
    constructible_from_anything(Args&&...) {}
};
static_assert(std::is_constructible_v<constructible_from_anything, int>);
static_assert(std::is_convertible_v<int, constructible_from_anything>);

struct tag {
    constexpr explicit tag() = default;
};

struct constructible_from_tag_and_anything {
    template<class... Args>
    constructible_from_tag_and_anything(const tag, Args&&...) {}
};
static_assert(std::is_constructible_v<constructible_from_tag_and_anything, tag>);
static_assert(std::is_constructible_v<constructible_from_tag_and_anything, tag, int>);

struct noexcept_move_constructible {
    noexcept_move_constructible(noexcept_move_constructible&&) noexcept {}
};
static_assert(std::is_nothrow_move_constructible_v<noexcept_move_constructible>);

TEST_CASE("constructors") {
    SUBCASE("default") {
        CHECK_UNARY(std::is_default_constructible_v<int>);
        CHECK_UNARY(std::is_default_constructible_v<opt::option<int>>);
        CHECK_UNARY(std::is_default_constructible_v<opt::option<non_default_constructible>>);
    }
    SUBCASE("from opt::none") {
        CHECK_UNARY(std::is_constructible_v<opt::option<int>, opt::none_t>);
    }
    SUBCASE("copy constructor") {
        CHECK_UNARY(std::is_copy_constructible_v<opt::option<int>>);
        CHECK_UNARY(std::is_copy_constructible_v<opt::option<non_trivially_copy_constructible>>);
        CHECK_UNARY_FALSE(std::is_copy_constructible_v<opt::option<non_copy_constructible>>);

        CHECK_UNARY_FALSE(std::is_trivially_copy_constructible_v<opt::option<non_copy_constructible>>);
        CHECK_UNARY_FALSE(std::is_trivially_copy_constructible_v<opt::option<non_trivially_copy_constructible>>);
    }
    SUBCASE("move constructor") {
        CHECK_UNARY(std::is_move_constructible_v<opt::option<int>>);
        CHECK_UNARY(std::is_move_constructible_v<opt::option<non_trivially_move_constructible>>);
        CHECK_UNARY_FALSE(std::is_move_constructible_v<opt::option<non_move_constructible>>);

        CHECK_UNARY_FALSE(std::is_trivially_move_constructible_v<opt::option<non_trivially_move_constructible>>);
        CHECK_UNARY_FALSE(std::is_trivially_move_constructible_v<opt::option<non_move_constructible>>);

        CHECK_UNARY(std::is_nothrow_move_constructible_v<opt::option<int>>);
        CHECK_UNARY_FALSE(std::is_nothrow_move_constructible_v<opt::option<non_trivially_move_constructible>>);
        CHECK_UNARY(std::is_nothrow_move_constructible_v<noexcept_move_constructible>);
    }
    SUBCASE("converting copy constructor") {
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, const opt::option<int>&>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<non_constructible_from_anything>, const opt::option<int>&>);

        CHECK_UNARY(std::is_constructible_v<opt::option<explicit_constructible_from_anything>, const opt::option<int>&>);
        CHECK_UNARY_FALSE(std::is_convertible_v<const opt::option<int>&, opt::option<explicit_constructible_from_anything>>);
        CHECK_UNARY(std::is_convertible_v<const opt::option<int>&, opt::option<constructible_from_anything>>);
    }
    SUBCASE("converting move constructor") {
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, opt::option<int>&&>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<non_constructible_from_anything>, opt::option<int>&&>);

        CHECK_UNARY(std::is_constructible_v<opt::option<explicit_constructible_from_anything>, opt::option<int>&&>);
        CHECK_UNARY_FALSE(std::is_convertible_v<opt::option<int>&&, opt::option<explicit_constructible_from_anything>>);
        CHECK_UNARY(std::is_convertible_v<opt::option<int&&>, opt::option<constructible_from_anything>>);
    }
    SUBCASE("from args") {
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, const int&>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, int&&>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, const int&, const float&>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, const int&, float&&>);

        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, const int&>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, int>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, tag>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, tag, const int&>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, tag, int, float>);

        struct s1 {
            s1(int) {}
        };
        CHECK_UNARY(std::is_constructible_v<opt::option<s1>, const int&>);
        CHECK_UNARY(std::is_convertible_v<const int&, opt::option<s1>>);

        struct s2 {
            s2(int, float) {}
        };
        CHECK_UNARY(std::is_constructible_v<opt::option<s2>, const int&, const float&>);
    }
    SUBCASE("from in_place and args") {
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, std::in_place_t, int>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, std::in_place_t, int, float>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, std::in_place_t>);

        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, std::in_place_t, int>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, std::in_place_t, int, float>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, std::in_place_t, tag>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, std::in_place_t, tag, int>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, std::in_place_t, tag, int, float>);
    }
    SUBCASE("from in_place and initializer_list") {
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, std::in_place_t, std::initializer_list<int>>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, std::in_place_t, std::initializer_list<int>, int>);

        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, std::in_place_t, std::initializer_list<int>>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, std::in_place_t, std::initializer_list<int>, int>);
    }
    SUBCASE("from value") {
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, const int&>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_anything>, int>);

        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<non_constructible_from_anything>, const int&>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<non_constructible_from_anything>, int>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, const int&>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, int>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, const tag&>);
        CHECK_UNARY(std::is_constructible_v<opt::option<constructible_from_tag_and_anything>, tag&&>);

        CHECK_UNARY(std::is_constructible_v<opt::option<int&>, int&>);
        CHECK_UNARY(std::is_constructible_v<opt::option<const int&>, int&>);
        CHECK_UNARY(std::is_constructible_v<opt::option<const int&>, const int&>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<int&>, int>);
        CHECK_UNARY_FALSE(std::is_constructible_v<opt::option<const int&>, int>);
    }
}

struct non_copy_assignable {
    non_copy_assignable& operator=(const non_copy_assignable&) = delete;
};
static_assert(!std::is_copy_assignable_v<non_copy_assignable>);

struct non_trivially_copy_assignable {
    non_trivially_copy_assignable& operator=(const non_trivially_copy_assignable&) {
        return *this;
    }
};
static_assert(!std::is_trivially_copy_assignable_v<non_trivially_copy_assignable>);

struct non_trivially_destructible {
    ~non_trivially_destructible() {}
};
static_assert(!std::is_trivially_destructible_v<non_trivially_destructible>);

struct non_move_assignable {
    non_move_assignable& operator=(non_move_assignable&&) = delete; 
};
static_assert(!std::is_move_assignable_v<non_move_assignable>);

struct non_trivially_move_assignable {
    non_trivially_move_assignable& operator=(non_trivially_move_assignable&&) {
        return *this;
    }
};
static_assert(!std::is_trivially_move_assignable_v<non_trivially_move_assignable>);

struct non_assignable_from_anything {
    template<class T>
    non_assignable_from_anything& operator=(T&&) = delete;
};
static_assert(!std::is_assignable_v<non_assignable_from_anything&, int>);

struct constructible_and_assignable_from_anything {
    template<class... Args>
    constructible_and_assignable_from_anything(Args&&...) {}
    template<class T>
    constructible_and_assignable_from_anything& operator=(T&&) { return *this; }
};
static_assert(std::is_constructible_v<constructible_and_assignable_from_anything, int>);
static_assert(std::is_assignable_v<constructible_and_assignable_from_anything&, int>);

struct noexcept_move_assignable_and_constructible {
    noexcept_move_assignable_and_constructible(noexcept_move_assignable_and_constructible&&) noexcept {}
    noexcept_move_assignable_and_constructible& operator=(noexcept_move_assignable_and_constructible&&) noexcept { return *this; }
};
static_assert(std::is_nothrow_move_assignable_v<noexcept_move_assignable_and_constructible>);
static_assert(std::is_nothrow_move_constructible_v<noexcept_move_assignable_and_constructible>);

TEST_CASE("operator=") {
    SUBCASE("from opt::none") {
        CHECK_UNARY(std::is_assignable_v<opt::option<int>&, const opt::none_t&>);
        CHECK_UNARY(std::is_assignable_v<opt::option<int>&, opt::none_t&&>);
    }
    SUBCASE("copy assignment") {
        CHECK_UNARY(std::is_copy_assignable_v<opt::option<int>>);
        CHECK_UNARY_FALSE(std::is_copy_assignable_v<opt::option<non_copy_constructible>>);
        CHECK_UNARY_FALSE(std::is_copy_assignable_v<opt::option<non_copy_assignable>>);

        CHECK_UNARY(std::is_trivially_copy_assignable_v<opt::option<int>>);
        CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<opt::option<non_trivially_copy_constructible>>);
        CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<opt::option<non_trivially_copy_assignable>>);
        CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<opt::option<non_trivially_destructible>>);
        CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<opt::option<non_copy_constructible>>);
        CHECK_UNARY_FALSE(std::is_trivially_copy_assignable_v<opt::option<non_copy_assignable>>);
    }
    SUBCASE("move assignment") {
        CHECK_UNARY(std::is_move_assignable_v<opt::option<int>>);
        CHECK_UNARY_FALSE(std::is_move_assignable_v<opt::option<non_move_constructible>>);
        CHECK_UNARY_FALSE(std::is_move_assignable_v<opt::option<non_move_assignable>>);

        CHECK_UNARY(std::is_trivially_move_assignable_v<opt::option<int>>);
        CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<opt::option<non_trivially_move_constructible>>);
        CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<opt::option<non_trivially_move_assignable>>);
        CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<opt::option<non_trivially_destructible>>);
        CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<opt::option<non_move_constructible>>);
        CHECK_UNARY_FALSE(std::is_trivially_move_assignable_v<opt::option<non_move_assignable>>);

        CHECK_UNARY(std::is_nothrow_move_assignable_v<opt::option<int>>);
        CHECK_UNARY_FALSE(std::is_nothrow_move_assignable_v<opt::option<non_trivially_move_assignable>>);
        CHECK_UNARY_FALSE(std::is_nothrow_move_assignable_v<opt::option<non_trivially_move_constructible>>);
        CHECK_UNARY(std::is_nothrow_move_assignable_v<opt::option<noexcept_move_assignable_and_constructible>>);
    }
    SUBCASE("from value") {
        CHECK_UNARY(std::is_assignable_v<opt::option<int>&, int>);
        CHECK_UNARY_FALSE(std::is_assignable_v<opt::option<non_constructible_from_anything>&, int>);
        CHECK_UNARY_FALSE(std::is_assignable_v<opt::option<non_assignable_from_anything>&, int>);
        struct s1 {};
        CHECK_UNARY(std::is_assignable_v<opt::option<constructible_and_assignable_from_anything>&, s1>);
    }
    SUBCASE("converting copy assignment") {
        CHECK_UNARY(std::is_assignable_v<opt::option<constructible_and_assignable_from_anything>&, const opt::option<int>&>);

        CHECK_UNARY_FALSE(std::is_assignable_v<opt::option<non_constructible_from_anything>&, const opt::option<int>&>);
        CHECK_UNARY_FALSE(std::is_assignable_v<opt::option<non_assignable_from_anything>&, const opt::option<int>&>);
    }
    SUBCASE("converting move assignment") {
        CHECK_UNARY(std::is_assignable_v<opt::option<constructible_and_assignable_from_anything>&, opt::option<int>&&>);

        CHECK_UNARY_FALSE(std::is_assignable_v<opt::option<non_constructible_from_anything>&, opt::option<int>>);
        CHECK_UNARY_FALSE(std::is_assignable_v<opt::option<non_assignable_from_anything>&, opt::option<int>>);
    }
}

template<class T1, class T2, class = bool>
constexpr bool is_eq_comparable = false;
template<class T1, class T2>
constexpr bool is_eq_comparable<T1, T2, decltype(std::declval<T1>() == std::declval<T2>())> = true;

template<class T1, class T2, class = bool>
constexpr bool is_lt_comparable = false;
template<class T1, class T2>
constexpr bool is_lt_comparable<T1, T2, decltype(std::declval<T1>() < std::declval<T2>())> = true;

template<class T1, class T2, class = bool>
constexpr bool is_gt_comparable = false;
template<class T1, class T2>
constexpr bool is_gt_comparable<T1, T2, decltype(std::declval<T1>() > std::declval<T2>())> = true;

template<class T1, class T2, class = bool>
constexpr bool is_le_comparable = false;
template<class T1, class T2>
constexpr bool is_le_comparable<T1, T2, decltype(std::declval<T1>() <= std::declval<T2>())> = true;

template<class T1, class T2, class = bool>
constexpr bool is_ge_comparable = false;
template<class T1, class T2>
constexpr bool is_ge_comparable<T1, T2, decltype(std::declval<T1>() >= std::declval<T2>())> = true;

template<class T1, class T2, class = bool>
constexpr bool is_ne_comparable = false;
template<class T1, class T2>
constexpr bool is_ne_comparable<T1, T2, decltype(std::declval<T1>() != std::declval<T2>())> = true;

TEST_CASE("compare operators") {
    SUBCASE("operator==") {
        CHECK_UNARY(is_eq_comparable<const opt::option<int>&, const opt::option<int>&>);
        struct s1 {
            int x;
            bool operator==(const s1& r) const { return x == r.x; }
        };
        CHECK_EQ((s1{1} == s1{1}), true);
        CHECK_UNARY(is_eq_comparable<const opt::option<s1>&, const opt::option<s1>&>);
        CHECK_UNARY(is_eq_comparable<const s1&, const opt::option<s1>&>);
        CHECK_UNARY(is_eq_comparable<const opt::option<s1>&, const s1&>);
        struct s2 {
            int x;
        };
        CHECK_UNARY_FALSE(is_eq_comparable<const opt::option<s2>&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_eq_comparable<const s2&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_eq_comparable<const opt::option<s2>&, const s2&>);
        struct s3 {
            int x;
            s1 operator==(const s3& r) const { return s1{int(x == r.x)}; }
        };
        CHECK_EQ((s3{1} == s3{1}), s1{1});
        CHECK_UNARY_FALSE(is_eq_comparable<const opt::option<s3>&, const opt::option<s3>&>);
        CHECK_UNARY_FALSE(is_eq_comparable<const s3&, const opt::option<s3>&>);
        CHECK_UNARY_FALSE(is_eq_comparable<const opt::option<s3>&, const s3&>);
        struct s4 {
            int x;
            operator bool() const { return x == 0; }
        };
        CHECK_UNARY(bool(s4{0}));
        struct s5 {
            int x;
            s4 operator==(const s5& r) const { return s4{int(x != r.x)}; }
        };
        CHECK_EQ(s5{1}, s5{1});
        CHECK_EQ((s5{1} == s5{1}), s4{0});
        CHECK_UNARY(is_eq_comparable<const opt::option<s5>&, const opt::option<s5>&>);
        CHECK_UNARY(is_eq_comparable<const s5&, const opt::option<s5>&>);
        CHECK_UNARY(is_eq_comparable<const opt::option<s5>&, const s5&>);
    }
    SUBCASE("operator!=") {
        CHECK_UNARY(is_ne_comparable<const opt::option<int>&, const opt::option<int>&>);
        struct s1 {
            int x;
            bool operator!=(const s1& r) const { return x != r.x; }
        };
        CHECK_UNARY(is_ne_comparable<const opt::option<s1>&, const opt::option<s1>&>);
        CHECK_UNARY(is_ne_comparable<const s1&, const opt::option<s1>&>);
        CHECK_UNARY(is_ne_comparable<const opt::option<s1>&, const s1&>);
        struct s2 {
            int x;
        };
        CHECK_UNARY_FALSE(is_ne_comparable<const opt::option<s2>&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_ne_comparable<const s2&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_ne_comparable<const opt::option<s2>&, const s2&>);
    }
    SUBCASE("operator<") {
        CHECK_UNARY(is_lt_comparable<const opt::option<int>&, const opt::option<int>&>);
        struct s1 {
            int x;
            bool operator<(const s1& r) const { return x < r.x; }
        };
        CHECK_UNARY(is_lt_comparable<const opt::option<s1>&, const opt::option<s1>&>);
        CHECK_UNARY(is_lt_comparable<const s1&, const opt::option<s1>&>);
        CHECK_UNARY(is_lt_comparable<const opt::option<s1>&, const s1&>);
        struct s2 {
            int x;
        };
        CHECK_UNARY_FALSE(is_lt_comparable<const opt::option<s2>&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_lt_comparable<const s2&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_lt_comparable<const opt::option<s2>&, const s2&>);
    }
    SUBCASE("operator<=") {
        CHECK_UNARY(is_le_comparable<const opt::option<int>&, const opt::option<int>&>);
        struct s1 {
            int x;
            bool operator<=(const s1& r) const { return x <= r.x; }
        };
        CHECK_UNARY(is_le_comparable<const opt::option<s1>&, const opt::option<s1>&>);
        CHECK_UNARY(is_le_comparable<const s1&, const opt::option<s1>&>);
        CHECK_UNARY(is_le_comparable<const opt::option<s1>&, const s1&>);
        struct s2 {
            int x;
        };
        CHECK_UNARY_FALSE(is_le_comparable<const opt::option<s2>&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_le_comparable<const s2&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_le_comparable<const opt::option<s2>&, const s2&>);
    }
    SUBCASE("operator>") {
        CHECK_UNARY(is_gt_comparable<const opt::option<int>&, const opt::option<int>&>);
        struct s1 {
            int x;
            bool operator>(const s1& r) const { return x > r.x; }
        };
        CHECK_UNARY(is_gt_comparable<const opt::option<s1>&, const opt::option<s1>&>);
        CHECK_UNARY(is_gt_comparable<const s1&, const opt::option<s1>&>);
        CHECK_UNARY(is_gt_comparable<const opt::option<s1>&, const s1&>);
        struct s2 {
            int x;
        };
        CHECK_UNARY_FALSE(is_gt_comparable<const opt::option<s2>&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_gt_comparable<const s2&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_gt_comparable<const opt::option<s2>&, const s2&>);
    }
    SUBCASE("operator>=") {
        CHECK_UNARY(is_ge_comparable<const opt::option<int>&, const opt::option<int>&>);
        struct s1 {
            int x;
            bool operator>=(const s1& r) const { return x >= r.x; }
        };
        CHECK_UNARY(is_ge_comparable<const opt::option<s1>&, const opt::option<s1>&>);
        CHECK_UNARY(is_ge_comparable<const s1&, const opt::option<s1>&>);
        CHECK_UNARY(is_ge_comparable<const opt::option<s1>&, const s1&>);
        struct s2 {
            int x;
        };
        CHECK_UNARY_FALSE(is_ge_comparable<const opt::option<s2>&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_ge_comparable<const s2&, const opt::option<s2>&>);
        CHECK_UNARY_FALSE(is_ge_comparable<const opt::option<s2>&, const s2&>);
    }
}

// NOLINTEND(modernize-use-equals-default,performance-noexcept-move-constructor,cert-oop54-cpp)

TEST_SUITE_END();

}



