
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>
#include <memory>
#include <type_traits>
#include <opt/option.hpp>
#include <utility>
#include <tuple>
#include <array>
#include <string_view>
#include <functional>
#include <cstdint>

#include "utils.hpp"

#if OPTION_MSVC
    #pragma fenv_access(on)
#elif OPTION_CLANG
    #if __clang_major__ >= 12
        #pragma STDC FENV_ACCESS ON
    #endif
#endif
    
namespace {

struct trivial_struct {};
static_assert(std::is_trivially_destructible_v<opt::option<trivial_struct>>);

static_assert(is_not_trivial_compatible<opt::option<nontrivial_struct>>);
static_assert(is_trivial_compatible<opt::option<int>>);

static_assert(std::is_nothrow_destructible_v<opt::option<int>>);
static_assert(!std::is_nothrow_destructible_v<opt::option<nontrivial_struct>>);

template<class T>
struct sample_values;

template<> struct sample_values<int> {
    int values[5]{1, 2, 3, 4, 5};
};

template<> struct sample_values<float> {
    float values[5]{1.f, 2.f, 3.f, 4.f, 5.f};

    fp_exception_checker checker{};
};
template<> struct sample_values<double> {
    double values[5]{1., 2., 3., 4., 5.};

    fp_exception_checker checker{};
};
template<> struct sample_values<bool> {
    bool values[5]{false, true, false, true, false};
};
template<> struct sample_values<std::reference_wrapper<int>> {
    int values_orig[5]{1, 2, 3, 4, 5};
    std::reference_wrapper<int> values[5]{values_orig[0], values_orig[1], values_orig[2], values_orig[3], values_orig[4]};
};
template<> struct sample_values<int*> {
    int values_orig[5]{1, 2, 3, 4, 5};
    int* values[5]{&values_orig[0], &values_orig[1], &values_orig[2], &values_orig[3], &values_orig[4]};
};
template<> struct sample_values<std::pair<int, float>> {
    std::pair<int, float> values[5]{{1, 2.f}, {3, 4.f}, {5, 6.f}, {7, 8.f}, {9, 10.f}};
};
template<> struct sample_values<std::pair<float, int>> {
    std::pair<float, int> values[5]{{1.f, 2}, {3.f, 4}, {5.f, 6}, {7.f, 8}, {9.f, 10}};
};
template<> struct sample_values<std::array<float, 4>> {
    std::array<float, 4> values[5]{{1.f, 2.f, 3.f, 4.f}, {5.f, 6.f, 7.f, 8.f}, {9.f, 10.f, 11.f, 12.f}, {13.f, 14.f, 15.f, 16.f}, {17.f, 18.f, 19.f, 20.f}};
};
template<> struct sample_values<std::array<int, 0>> {
    std::array<int, 0> values[5]{{}, {}, {}, {}, {}};
};

struct empty_struct {
    bool operator==(const empty_struct&) const { return true; }
};
template<> struct sample_values<empty_struct> {
    empty_struct values[5]{{}, {}, {}, {}, {}};
};

template<> struct sample_values<std::tuple<>> {
    std::tuple<> values[5]{{}, {}, {}, {}, {}};
};
template<> struct sample_values<std::tuple<int, float, int>> {
    std::tuple<int, float, int> values[5]{{1, 2.f, 3}, {4, 5.f, 6}, {7, 8.f, 9}, {10, 11.f, 12}, {13, 14.f, 15}};
};

struct aggregate_int_float {
    int x;
    float y;

    bool operator==(const aggregate_int_float& a) const { return x == a.x && y == a.y; }
};
template<> struct sample_values<aggregate_int_float> {
    aggregate_int_float values[5]{{1, 2.f}, {3, 4.f}, {5, 6.f}, {7, 8.f}, {9, 10.f}};
};

struct aggregate_with_empty_struct {
    int x;
    empty_struct y;

    bool operator==(const aggregate_with_empty_struct& a) const { return x == a.x; }
};

template<> struct sample_values<aggregate_with_empty_struct> {
    aggregate_with_empty_struct values[5]{
        {1, {}}, {2, {}}, {3, {}}, {4, {}}, {5, {}}
    };
};

struct empty_polymorphic_type {
    empty_polymorphic_type() = default;
    empty_polymorphic_type(const empty_polymorphic_type&) = default;
    empty_polymorphic_type& operator=(const empty_polymorphic_type&) = default;

    virtual ~empty_polymorphic_type() = default;

    bool operator==(const empty_polymorphic_type&) const { return true; }
};
static_assert(std::is_polymorphic_v<empty_polymorphic_type>);

template<> struct sample_values<empty_polymorphic_type> {
    empty_polymorphic_type values[5]{{}, {}, {}, {}, {}};
};

struct polymorphic_type {
    int x;

    polymorphic_type(int x_) : x(x_) {}

    polymorphic_type() = default;
    polymorphic_type(const polymorphic_type&) = default;
    polymorphic_type& operator=(const polymorphic_type&) = default;

    virtual ~polymorphic_type() = default;

    bool operator==(const polymorphic_type& other) const { return x == other.x; }
};
static_assert(std::is_polymorphic_v<polymorphic_type>);

template<> struct sample_values<polymorphic_type> {
    polymorphic_type values[5]{{1}, {2}, {3}, {4}, {5}};
};

template<> struct sample_values<std::string_view> {
    std::string_view values[5]{"a1", "b2", "c3", "d4", "e5"};
};

template<> struct sample_values<int(*)(int)> {
    using t = int(*)(int);
    t values[5]{
        [](int x) { return x + 1; },
        [](int x) { return x + 2; },
        [](int x) { return x + 3; },
        [](int x) { return x + 4; },
        [](int x) { return x + 5; }
    };
};

struct struct_with_sentinel {
    int x;
    std::uint8_t SENTINEL{};

    bool operator==(const struct_with_sentinel& o) const { return x == o.x; }
};
template<> struct sample_values<struct_with_sentinel> {
    struct_with_sentinel values[5]{struct_with_sentinel{1}, struct_with_sentinel{2}, struct_with_sentinel{3}, struct_with_sentinel{4}, struct_with_sentinel{5}};
};

template<> struct sample_values<std::pair<empty_struct, int>> {
    std::pair<empty_struct, int> values[5]{{{}, 1}, {{}, 2}, {{}, 3}, {{}, 4}, {{}, 5}};
};
template<> struct sample_values<std::tuple<int, empty_struct, long>> {
    std::tuple<int, empty_struct, long> values[5]{{1, {}, 2l}, {3, {}, 4l}, {5, {}, 6l}, {7, {}, 8l}, {9, {}, 10l}};
};
template<> struct sample_values<std::array<empty_struct, 2>> {
    std::array<empty_struct, 2> values[5]{};
};
template<> struct sample_values<std::string> {
    std::string values[5]{"abc", "0123456789101112131415161718192021222324", "def", "2526272829303132333435363738394041424344", "ghi"};
};


TEST_CASE_TEMPLATE("opt::option", T, std::string, struct_with_sentinel, int(*)(int), std::string_view, polymorphic_type, empty_polymorphic_type, aggregate_int_float, empty_struct, std::tuple<>, std::tuple<int, float, int>, double, bool, std::reference_wrapper<int>, int*, float, std::pair<int, float>, std::pair<float, int>, std::array<float, 4>, /*std::tuple<int, empty_struct, long>,*/ /*std::array<empty_struct, 2>,*/ /*aggregate_with_empty_struct,*/ int) {
    const sample_values<T> sample;
    // Allow captured structured bindings in lambda
    const auto& v0 = sample.values[0];
    const auto& v1 = sample.values[1];
    const auto& v2 = sample.values[2];
    const auto& v3 = sample.values[3];
    const auto& v4 = sample.values[4];

    if constexpr (!std::is_same_v<T, int>) {
        CHECK_EQ(sizeof(opt::option<T>), sizeof(T));
        CHECK_EQ(sizeof(opt::option<opt::option<T>>), sizeof(T));
        CHECK_EQ(sizeof(opt::option<opt::option<opt::option<T>>>), sizeof(T));
        CHECK_EQ(sizeof(opt::option<opt::option<opt::option<opt::option<T>>>>), sizeof(T));
        CHECK_EQ(sizeof(opt::option<opt::option<opt::option<opt::option<opt::option<T>>>>>), sizeof(T));
    }

    SUBCASE("constructor") {
        const opt::option<T> a;
        CHECK_UNARY_FALSE(a.has_value());

        const opt::option<T> b(opt::none);
        CHECK_UNARY_FALSE(b.has_value());

        const opt::option<T> c = opt::none;
        CHECK_UNARY_FALSE(c.has_value());

        if constexpr (std::is_default_constructible_v<T>) {
            const opt::option<T> d{{}};
            CHECK_EQ(d, T{});
        }

        opt::option<const T> e;
        CHECK_UNARY_FALSE(e.has_value());
        e.emplace(v0);
        CHECK_UNARY(e.has_value());
        CHECK_EQ(e, v0);

        e.emplace(v1);
        CHECK_UNARY(e.has_value());
        CHECK_EQ(e, v1);
    }
    SUBCASE("std::in_place constructors") {
        if constexpr (std::is_default_constructible_v<T>) {
            const opt::option<T> a{std::in_place};
            CHECK_UNARY(a.has_value());
            CHECK_EQ(a, T{});
        }
        opt::option<T> b{std::in_place, v0};
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        b = opt::option<T>{std::in_place, v1};
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v1);
    }
    SUBCASE(".get") {
        opt::option<T> a{v0};
        CHECK_EQ(a.get(), v0);
        CHECK_EQ(std::as_const(a).get(), v0);
        CHECK_EQ(as_const_rvalue(a).get(), v0);
        CHECK_EQ(as_rvalue(a).get(), v0);
    }
    SUBCASE("operator=") {
        opt::option<T> a = v0;
        a = opt::none;
        CHECK_UNARY_FALSE(a.has_value());
        {
            const opt::option tmp{v1};
            a = tmp;
        }
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, v1);
        {
            const opt::option<T> tmp{opt::none};
            a = tmp;
        }
        CHECK_UNARY_FALSE(a.has_value());
        a = opt::option<T>(v2);
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, v2);
        a = opt::option<T>(opt::none);
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_UNARY_FALSE(a.has_value());
        {
            const T tmp = v3;
            a = tmp;
        }
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, v3);
        a = v4;
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, v4);

        opt::option<T> b;
        CHECK_UNARY_FALSE(b.has_value());
        b = v0;
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        b = v0;
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        b = v1;
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v1);
        b = opt::option<T>{};
        CHECK_UNARY_FALSE(b.has_value());
        b = opt::option<T>{};
        CHECK_UNARY_FALSE(b.has_value());
        b = opt::option<T>{v0};
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        b = opt::none;
        CHECK_UNARY_FALSE(b.has_value());
        b = opt::none;
        CHECK_UNARY_FALSE(b.has_value());

        opt::option<T> c;
        CHECK_UNARY_FALSE(c.has_value());
        c = v0;
        CHECK_UNARY(c.has_value());
        CHECK_EQ(c, v0);

        opt::option<T> d;
        CHECK_UNARY_FALSE(d.has_value());
        d = c;
        CHECK_UNARY(c.has_value());
        CHECK_UNARY(d.has_value());
        CHECK_EQ(d, v0);

        c = opt::none;
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_UNARY(d.has_value());
        CHECK_EQ(d, v0);
        d = opt::none;

        c = d;
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_UNARY_FALSE(d.has_value());

        c = opt::option<T>{d};
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_UNARY_FALSE(d.has_value());

        d = v0;
        CHECK_UNARY(d.has_value());
        CHECK_EQ(d, v0);

        c = opt::option<T>{d};
        CHECK_UNARY(c.has_value());
        CHECK_UNARY(d.has_value());
        CHECK_EQ(d, v0);
        CHECK_EQ(c, v0);

        d = opt::option<T>{c};
        CHECK_UNARY(c.has_value());
        CHECK_UNARY(d.has_value());

        d = opt::none;
        CHECK_UNARY_FALSE(d.has_value());
        c = opt::option<T>{d};
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_UNARY_FALSE(d.has_value());

        c = v0;
        CHECK_UNARY(c.has_value());
        d = std::move(c);
        CHECK_UNARY(d.has_value());
        CHECK_UNARY(c.has_value()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)

        d = opt::none;
        c = v0;
        CHECK_UNARY_FALSE(d.has_value());
        CHECK_UNARY(c.has_value());
        d = opt::option<T>{std::move(c)};
        CHECK_UNARY(d.has_value());
        CHECK_UNARY(c.has_value()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    }
    SUBCASE("ref assignment") {
        opt::option<T> a;
        CHECK_EQ(a, opt::none);
        a = v0;
        CHECK_EQ(a, v0);
        *a = v1;
        CHECK_EQ(a, v1);
        a.emplace(v1);

        T& b = *a;
        b = v0;
        CHECK_EQ(a, v0);

        const auto fn1 = [&](T& x) {
            x = v0;
        };
        a.emplace(v1);
        fn1(*a);
        CHECK_EQ(a, v0);

        const auto fn2 = [&](T& x) {
            x = v1;
        };
        a.emplace(v0);
        fn2(*a);
        CHECK_EQ(a, v1);
    }
    SUBCASE(".reset") {
        opt::option<T> a;
        CHECK_UNARY_FALSE(a.has_value());
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
        a = v0;
        CHECK_UNARY(a.has_value());
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
    }
    SUBCASE(".emplace") {
        opt::option<T> a{v0};
        CHECK_EQ(a, v0);
        a.emplace(v0);
        CHECK_EQ(a, v0);
        a.reset();
        CHECK_EQ(a, opt::none);
        a.emplace(v1);
        CHECK_EQ(a, v1);
        a.emplace(v0);
        CHECK_EQ(a, v0);
    }
    SUBCASE(".value_or_throw") {
        opt::option<T> a{v0};
        CHECK_NOTHROW((void)a.value_or_throw());
        CHECK_NOTHROW((void)a.value());
        CHECK_NOTHROW((void)as_const(a).value());
        CHECK_NOTHROW((void)as_const_rvalue(a).value());
        a = opt::none;
        CHECK_THROWS_AS((void)a.value_or_throw(), opt::bad_access);
        CHECK_THROWS_AS((void)a.value(), opt::bad_access);
        CHECK_THROWS_AS((void)as_const(a).value(), opt::bad_access);
        CHECK_THROWS_AS((void)as_const_rvalue(a).value(), opt::bad_access);
    }
    SUBCASE(".value_or") {
        opt::option<T> a;
        CHECK_EQ(a.value_or(v0), v0);
        a = v1;
        CHECK_EQ(a.value_or(v2), v1);
        CHECK_EQ(as_rvalue(a).value_or(v3), v1);
    }
    if (v0 == v1) { goto skip_and_then; }
    SUBCASE(".and_then") {
        const auto fn1 = [&](const T& x) -> opt::option<int> {
            return x == v0 ? 0 : 1;
        };
        CHECK_EQ(opt::option<T>{v0}.and_then(fn1), 0);
        CHECK_EQ(opt::option<T>{v1}.and_then(fn1), 1);
        CHECK_EQ(opt::option<T>{opt::none}.and_then(fn1), opt::none);

        const auto fn2 = [&](const T& x) -> opt::option<unsigned> {
            return x == v1 ? opt::option<unsigned>{123u} : opt::none;
        };
        CHECK_EQ(opt::option<T>{v0}.and_then(fn2), opt::none);
        CHECK_EQ(opt::option<T>{v1}.and_then(fn2), 123u);
        CHECK_EQ(opt::option<T>{opt::none}.and_then(fn2), opt::none);
    }
skip_and_then:
    if (v0 == v1) { goto skip_map; }
    SUBCASE(".map") {
        const auto fn1 = [&](const T& x) -> T {
            return x == v0 ? v1 : v0;
        };
        CHECK_EQ(opt::option<T>{v0}.map(fn1), v1);
        CHECK_EQ(opt::option<T>{v1}.map(fn1), v0);
        CHECK_EQ(opt::option<T>{opt::none}.map(fn1), opt::none);

        const auto fn2 = [&](const T& x) -> int {
            return x == v0 ? 100 : -1;
        };
        CHECK_EQ(opt::option<T>{v0}.map(fn2), 100);
        CHECK_EQ(opt::option<T>{v1}.map(fn2), -1);
        CHECK_EQ(opt::option<T>{opt::none}.map(fn2), opt::none);

        const auto fn3 = [&](T&&) -> unsigned {
            return 1;
        };
        CHECK_EQ(opt::option<T>{v0}.map(fn3), 1u);
        CHECK_EQ(opt::option<T>{v1}.map(fn3), 1u);
        CHECK_EQ(opt::option<T>{opt::none}.map(fn3), opt::none);

        bool var = false;
        const auto fn4 = [&](T& x) -> opt::option<int> {
            if (var) { x = v0; }
            return x == v0 ? opt::none : opt::option<int>{1};
        };
        opt::option<T> a{v0};
        CHECK_EQ(a.map(fn4), opt::make_option(opt::option<int>{opt::none}));
        CHECK_EQ(a, v0);
        a = v1;
        CHECK_EQ(a.map(fn4), opt::make_option(opt::option<int>{1}));
        CHECK_EQ(a, v1);
        var = true;
        CHECK_EQ(a.map(fn4), opt::make_option(opt::option<int>{opt::none}));
        CHECK_EQ(a, v0);
    }
skip_map:
    SUBCASE(".or_else") {
        const auto fn1 = [&]() { return opt::option<T>{v0}; };
        CHECK_EQ(opt::option<T>{v0}.or_else(fn1), v0);
        CHECK_EQ(opt::option<T>{v1}.or_else(fn1), v1);
        CHECK_EQ(opt::option<T>{opt::none}.or_else(fn1), v0);
    }
    SUBCASE(".take") {
        opt::option<T> a;
        opt::option<T> b = a.take();
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_UNARY_FALSE(a.has_value());
        a = v0;
        CHECK_UNARY_FALSE(b.has_value());
        CHECK_UNARY(a.has_value());
        b = a.take();
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        CHECK_UNARY_FALSE(a.has_value());
        a = b.take();
        CHECK_UNARY(a.has_value());
        CHECK_UNARY_FALSE(b.has_value());
        CHECK_EQ(a, v0);
        a = v1;
        CHECK_UNARY(a.has_value());
        CHECK_EQ(*a, v1);
        (void)a.take();
        CHECK_UNARY_FALSE(a.has_value());
    }
    SUBCASE("opt::option_cast") {
        const opt::option<T> a = v0;
        CHECK_EQ(a, v0);
        opt::option<T> b = opt::option_cast<T>(a);
        CHECK_EQ(b, v0);
        b = opt::option_cast<T>(opt::option<T>{v1});
        CHECK_EQ(b, v1);

        if constexpr (std::is_convertible_v<T, int>) {
            const opt::option<T> c = v0;
            CHECK_EQ(c, v0);
            opt::option<int> d = opt::option_cast<int>(c);
            CHECK_EQ(d, static_cast<int>(v0));
            d = opt::option_cast<int>(opt::option<T>{v1});
            CHECK_EQ(d, static_cast<int>(v1));
        }
    }
    SUBCASE("deduction guides") {
        // NOLINTBEGIN(misc-const-correctness)
        auto a = opt::option{v0};
        CHECK_UNARY(std::is_same_v<decltype(a), opt::option<T>>);
        opt::option b{v1};
        CHECK_UNARY(std::is_same_v<decltype(b), opt::option<T>>);

        auto c = opt::option{opt::option{v0}};
        CHECK_UNARY(std::is_same_v<decltype(c), opt::option<T>>);

        auto d = opt::option{opt::option{opt::option{v1}}};
        CHECK_UNARY(std::is_same_v<decltype(d), opt::option<T>>);

        auto e = opt::make_option(opt::option{v0});
        CHECK_UNARY(std::is_same_v<decltype(e), opt::option<opt::option<T>>>);

        auto f = opt::make_option(opt::make_option(v2));
        CHECK_UNARY(std::is_same_v<decltype(f), opt::option<opt::option<T>>>);

        opt::option g(opt::make_option(v4));
        CHECK_UNARY(std::is_same_v<decltype(g), opt::option<T>>);

        auto h = opt::option{a};
        CHECK_UNARY(std::is_same_v<decltype(h), opt::option<T>>);

        auto i = opt::option{e};
        CHECK_UNARY(std::is_same_v<decltype(i), opt::option<opt::option<T>>>);

        // NOLINTEND(misc-const-correctness)
    }
    SUBCASE(".value_or_default") {
        if constexpr (std::is_default_constructible_v<T>) {
            opt::option a{v0};
            CHECK_EQ(a.value_or_default(), v0);
            a = opt::none;
            CHECK_EQ(a.value_or_default(), T{});
        }
    }
    SUBCASE(".ptr_or_null") {
        opt::option a{v0};

        CHECK_EQ(*(a.ptr_or_null()), v0);
        a = opt::none;
        CHECK_EQ(a.ptr_or_null(), nullptr);
        a = v1;
        CHECK_EQ(*(as_const(a).ptr_or_null()), v1);
    }
    if (v0 == v1) { goto skip_filter; }
    SUBCASE(".filter") {
        const auto fn1 = [&](const T& x) {
            return x == v0;
        };
        opt::option<T> a;
        a = v0;
        CHECK_EQ(a.filter(fn1), v0);
        a = v1;
        CHECK_EQ(a.filter(fn1), opt::none);
        a = opt::none;
        CHECK_EQ(a.filter(fn1), opt::none);

        const auto fn2 = [&](T& x) {
            T prev = x;
            x = v1;
            return prev == v0;
        };
        a = v0;
        CHECK_EQ(a.filter(fn2), v1);
        CHECK_EQ(a, v1);
        a = v1;
        CHECK_EQ(a.filter(fn2), opt::none);
        CHECK_EQ(a, v1);
    }
skip_filter:
    if (v0 == v1) { goto skip_flatten; }
    SUBCASE(".flatten") {
        auto a = opt::make_option(opt::make_option(v0));
        CHECK_EQ(**a, v0);
        auto b = a.flatten();
        CHECK_EQ(*b, v0);

        a = opt::option{opt::option<T>{opt::none}};
        b = a.flatten();
        CHECK_UNARY_FALSE(b.has_value());
        a = opt::option<T>{opt::none};
        b = a.flatten();
        CHECK_UNARY_FALSE(b.has_value());
        a = opt::none;
        b = a.flatten();
        CHECK_UNARY_FALSE(b.has_value());
    }
skip_flatten:
    if (v0 == v1) { goto skip_map_or; }
    SUBCASE(".map_or") {
        const auto fn1 = [&](const T& x) {
            return x == v0 ? v1 : v0;
        };
        CHECK_EQ(opt::option<T>{v0}.map_or(v0, fn1), v1);
        CHECK_EQ(opt::option<T>{v0}.map_or(v1, fn1), v1);
        CHECK_EQ(opt::option<T>{v1}.map_or(v0, fn1), v0);
        CHECK_EQ(opt::option<T>{v1}.map_or(v1, fn1), v0);
        CHECK_EQ(opt::option<T>{opt::none}.map_or(v0, fn1), v0);
        CHECK_EQ(opt::option<T>{opt::none}.map_or(v1, fn1), v1);

        const auto fn2 = [&](const T& x) -> int {
            return x == v0 ? 0 : 1;
        };
        CHECK_EQ(opt::option<T>{v0}.map_or(3, fn2), 0);
        CHECK_EQ(opt::option<T>{v1}.map_or(3, fn2), 1);
        CHECK_EQ(opt::option<T>{opt::none}.map_or(3, fn2), 3);
    }
skip_map_or:
    if (v0 == v1) { goto skip_map_or_else; }
    SUBCASE(".map_or_else") {
        const auto fn1 = [&](const T& x) {
            return x == v0 ? v1 : v0;
        };
        const auto fn2 = [&]() {
            return v0;
        };
        CHECK_EQ(opt::option<T>{v0}.map_or_else(fn2, fn1), v1);
        CHECK_EQ(opt::option<T>{v1}.map_or_else(fn2, fn1), v0);
        CHECK_EQ(opt::option<T>{opt::none}.map_or_else(fn2, fn1), v0);

        const auto fn3 = [&](const T& x) -> int {
            return x == v0 ? 1 : -1;
        };
        const auto fn4 = []() -> int {
            return 0;
        };
        CHECK_EQ(opt::option<T>{v0}.map_or_else(fn4, fn3), 1);
        CHECK_EQ(opt::option<T>{v1}.map_or_else(fn4, fn3), -1);
        CHECK_EQ(opt::option<T>{opt::none}.map_or_else(fn4, fn3), 0);
    }
skip_map_or_else:
    if (v0 == v1) { goto skip_take_if; }
    SUBCASE(".take_if") {
        opt::option<T> a = v0;
        opt::option<T> b = a.take_if([&](const T& x) { return x == v0; });
        CHECK_EQ(b, v0);
        CHECK_UNARY_FALSE(a.has_value());

        a.emplace(v0);

        b = a.take_if([&](const T& x) { return x == v1; });
        CHECK_EQ(a, v0);
        CHECK_UNARY_FALSE(b.has_value());

        a = v1;
        CHECK_EQ(a, v1);
        b = a.take_if([&](T& x) {
            x = v0;
            return true;
        });
        CHECK_EQ(b, v0);
        CHECK_UNARY_FALSE(a.has_value());
    }
skip_take_if:
    SUBCASE(".has_value_and") {
        opt::option a{v0};
        CHECK_UNARY(a.has_value_and([&](const T& x) { return x == v0; }));
        if (!(v0 == v1)) {
            CHECK_UNARY_FALSE(a.has_value_and([&](const T& x) { return x == v1; }));
        }
        a.reset();
        CHECK_UNARY_FALSE(a.has_value_and([&](const T& x) { return x == v0; }));
        CHECK_UNARY_FALSE(a.has_value_and([&](const T& x) { return x == v1; }));
    }
    if (v0 == v1) { goto skip_inspect; }
    SUBCASE(".inspect") {
        opt::option<T> a;
        a.inspect([&](T& x) { x = v0; });
        CHECK_UNARY_FALSE(a.has_value());

        a = v0;
        CHECK_EQ(a, v0);
        a.inspect([&](T& x) { x = v1; });
        CHECK_EQ(a, v1);

        a.inspect([&](T& x) { x = v2; }).inspect([&](T& x) { x = v3; });
        CHECK_EQ(a, v3);
    }
skip_inspect:
    SUBCASE(".assume_has_value") {
        opt::option a{v0};
        a.assume_has_value();
        CHECK_EQ(*a, v0);
    }
    SUBCASE(".unzip") {
        SUBCASE("std::tuple") {
            opt::option a{std::tuple{v0, v1, v2, v3}};
            CHECK_UNARY(a.has_value());

            auto b = a.unzip();
            CHECK_UNARY(std::is_same_v<decltype(b), std::tuple<
                opt::option<T>, opt::option<T>, opt::option<T>, opt::option<T>
            >>);
            auto& [b1, b2, b3, b4] = b;

            CHECK_UNARY(b1.has_value());
            CHECK_EQ(*b1, v0);
            CHECK_UNARY(b2.has_value());
            CHECK_EQ(*b2, v1);
            CHECK_UNARY(b3.has_value());
            CHECK_EQ(*b3, v2);
            CHECK_UNARY(b4.has_value());
            CHECK_EQ(*b4, v3);

            a.reset();
            auto c = a.unzip();
            auto& [c1, c2, c3, c4] = c;
            CHECK_UNARY_FALSE(c1.has_value());
            CHECK_UNARY_FALSE(c2.has_value());
            CHECK_UNARY_FALSE(c3.has_value());
            CHECK_UNARY_FALSE(c4.has_value());
        }
        SUBCASE("std::pair") {
            opt::option a{std::pair{v0, v1}};
            CHECK_UNARY(a.has_value());

            auto b = a.unzip();
            CHECK_UNARY(std::is_same_v<decltype(b), std::pair<opt::option<T>, opt::option<T>>>);
            auto& [b1, b2] = b;

            CHECK_UNARY(b1.has_value());
            CHECK_EQ(*b1, v0);
            CHECK_UNARY(b2.has_value());
            CHECK_EQ(*b2, v1);

            a.reset();
            auto c = a.unzip();
            auto& [c1, c2] = c;
            CHECK_UNARY_FALSE(c1.has_value());
            CHECK_UNARY_FALSE(c2.has_value());
        }
        SUBCASE("std::array") {
            opt::option a{std::array{v0, v1, v2}};
            CHECK_UNARY(a.has_value());

            auto b = a.unzip();
            CHECK_UNARY(std::is_same_v<decltype(b), std::array<opt::option<T>, 3>>);
            auto& [b1, b2, b3] = b;

            CHECK_UNARY(b1.has_value());
            CHECK_EQ(*b1, v0);
            CHECK_UNARY(b2.has_value());
            CHECK_EQ(*b2, v1);
            CHECK_UNARY(b3.has_value());
            CHECK_EQ(*b3, v2);

            a.reset();
            auto c = a.unzip();
            auto& [c1, c2, c3] = c;
            CHECK_UNARY_FALSE(c1.has_value());
            CHECK_UNARY_FALSE(c2.has_value());
            CHECK_UNARY_FALSE(c3.has_value());
        }
    }
    SUBCASE("opt::zip") {
        opt::option<T> a{v0};
        opt::option<T> b{v1};

        auto c = opt::zip(a, b);
        CHECK_UNARY(c.has_value());

        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93147
        CHECK_EQ(std::tuple_size_v<typename decltype(c)::value_type>, 2);
        if constexpr (std::tuple_size_v<typename decltype(c)::value_type> == 2) {
            CHECK_EQ(std::get<0>(*c), v0);
            CHECK_EQ(std::get<1>(*c), v1);
        }

        a.reset();
        c = opt::zip(a, b);
        CHECK_UNARY_FALSE(c.has_value());

        b.reset();
        c = opt::zip(a, b);
        CHECK_UNARY_FALSE(c.has_value());
    }
    if (v0 == v1) { goto skip_zip_with; }
    SUBCASE("opt::zip_with") {
        const auto fn1 = [&](const T& x) -> int {
            return x == v0 ? 1 : 0;
        };
        CHECK_EQ(opt::zip_with(fn1, opt::option<T>{v0}), 1);
        CHECK_EQ(opt::zip_with(fn1, opt::option<T>{v1}), 0);
        CHECK_EQ(opt::zip_with(fn1, opt::option<T>{opt::none}), opt::none);

        struct s1 {
            T x;
            T y;
            int z;
        };
        const auto fn2 = [&](const T& x, const T& y) {
            return s1{(x == v0 ? v1 : v0), (y), (x == v0 && y == v0 ? 1 : -1)};
        };
        opt::option<s1> a = opt::zip_with(fn2, opt::option<T>{v0}, opt::option<T>{v0});
        CHECK_EQ(a->x, v1);
        CHECK_EQ(a->y, v0);
        CHECK_EQ(a->z, 1);
        a = opt::zip_with(fn2, opt::option<T>{v1}, opt::option<T>{v0});
        CHECK_EQ(a->x, v0);
        CHECK_EQ(a->y, v0);
        CHECK_EQ(a->z, -1);
        a = opt::zip_with(fn2, opt::option<T>{opt::none}, opt::option<T>{v1});
        CHECK_EQ(a, opt::none);
        a = opt::zip_with(fn2, opt::option<T>{v1}, opt::option<T>{opt::none});
        CHECK_EQ(a, opt::none);

        const auto fn3 = [&]() {
            return v0;
        };
        opt::option<T> b = opt::zip_with(fn3);
        CHECK_UNARY(b.has_value());

        const auto fn4 = [&](T& x, const T& y) {
            x = (y == v0 ? v1 : v0);
        };
        opt::zip_with(fn4, b, opt::option<T>{v0});
        CHECK_EQ(b, v1);
        opt::zip_with(fn4, b, opt::option<T>{v1});
        CHECK_EQ(b, v0);
    }
skip_zip_with:
    SUBCASE(".replace") {
        opt::option a{std::make_unique<T>(v0)};

        auto c = a.replace(std::make_unique<T>(v1));
        CHECK_UNARY(c.has_value());
        CHECK_EQ(**c, v0);
        CHECK_UNARY(a.has_value());
        CHECK_EQ(**a, v1);
    }
    SUBCASE("opt::from_nullable") {
        T a = v0;
        T* ptr = &a;

        const opt::option<T> b = opt::from_nullable(ptr);
        CHECK_EQ(b, v0);

        ptr = nullptr;
        const opt::option<T> c = opt::from_nullable(ptr);
        CHECK_UNARY_FALSE(c.has_value());
    }
}

}
