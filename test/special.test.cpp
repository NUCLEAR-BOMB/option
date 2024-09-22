
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>
#include <cfenv>
#include <opt/option.hpp>
#include <tuple>
#include <functional>
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>
#include <array>
#include <memory>
#include <cstdint>
#include <cstddef>
#include <variant>
#include <sstream>

#include "utils.hpp"

#if OPTION_USE_BUILTIN_TRAITS

// NOLINTBEGIN(cert-dcl58-cpp)

template<class T1, class T2>
struct my_tuple_0 {
    T1 x;
    T2 y;

    template<std::size_t I>
    const auto& get() const {
        if constexpr (I == 0) { return x; }
        else if constexpr (I == 1) { return y; }
    }
    template<std::size_t I>
    auto& get() {
        if constexpr (I == 0) { return x; }
        else if constexpr (I == 1) { return y; }
    }
};

template<class T1, class T2>
struct std::tuple_size<my_tuple_0<T1, T2>> {
    static constexpr int value = 2;
};

template<std::size_t I, class T1, class T2>
struct std::tuple_element<I, my_tuple_0<T1, T2>> {
    using type = std::conditional_t<I == 0, T1, T2>;
};

namespace my_ns {
    template<class T1, class T2, class T3>
    struct my_tuple {
        T1 x;
        T2 y;
        T3 z;
    };

    template<std::size_t I, class T1, class T2, class T3>
    auto& get(my_tuple<T1, T2, T3>& x) {
        if constexpr (I == 0) { return x.x; }
        else if constexpr (I == 1) { return x.y; }
        else if constexpr (I == 2) { return x.z; }
    }
    template<std::size_t I, class T1, class T2, class T3>
    const auto& get(const my_tuple<T1, T2, T3>& x) {
        if constexpr (I == 0) { return x.x; }
        else if constexpr (I == 1) { return x.y; }
        else if constexpr (I == 2) { return x.z; }
    }
}

template<class T1, class T2, class T3>
struct std::tuple_size<my_ns::my_tuple<T1, T2, T3>> {
    static constexpr int value = 3;
};

template<std::size_t I, class T1, class T2, class T3>
struct std::tuple_element<I, my_ns::my_tuple<T1, T2, T3>> {
    using type = std::conditional_t<I == 0, T1, std::conditional_t<I == 1, T2, T3>>;
};

// NOLINTEND(cert-dcl58-cpp)

namespace {

TEST_SUITE_BEGIN("special");

static_assert(sizeof(opt::option<int&>) == sizeof(int*));

static_assert(is_trivial_compatible<opt::option<int&>>);
static_assert(is_trivial_compatible<opt::option<nontrivial_struct&>>);

TEST_CASE("reference") {
    SUBCASE("non-const") {
        int a = 1;
        opt::option<int&> ref{a};
        
        CHECK_UNARY(std::is_same_v<decltype(*ref), int&>);
        CHECK_UNARY(std::is_same_v<decltype(*as_const(ref)), int&>);
        CHECK_UNARY(std::is_same_v<decltype(*as_rvalue(ref)), int&>);

        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &a);
        CHECK_EQ(*ref, 1);

        *ref = 2;
        CHECK_EQ(&(ref.get()), &a);
        CHECK_EQ(a, 2);

        int b = 3;
        *ref = b;
        CHECK_EQ(&(ref.get()), &a);
        CHECK_EQ(a, 3);

        b = 4;
        ref = std::ref(b);
        CHECK_EQ(&(ref.get()), &b);
        CHECK_EQ(b, 4);

        ref = a;
        CHECK_EQ(&(ref.get()), &a);
        CHECK_EQ(a, 3);

        ref = opt::none;
        CHECK_UNARY_FALSE(ref.has_value());

        ref = opt::option<int&>{b};
        CHECK_EQ(&(ref.get()), &b);

        ref = opt::option<int&>{opt::none};
        CHECK_UNARY_FALSE(ref.has_value());

        static_assert(std::is_same_v<
            opt::option<std::reference_wrapper<int>>,
            decltype(opt::option{std::ref(b)})
        >);
        ref = opt::option{std::ref(b)};
        CHECK_EQ(&(ref.get()), &b);

        const opt::option<int&> refc{a};
        CHECK(refc.has_value());
        CHECK_EQ(&(refc.get()), &a);
        CHECK_EQ(*refc, 3);

        *refc = 4;
        CHECK_EQ(*refc, 4);
        CHECK_EQ(a, 4);
    }
    SUBCASE("const") {
        const int a = 1;
        opt::option<const int&> ref{a};

        CHECK_UNARY(std::is_same_v<decltype(*ref), const int&>);
        CHECK_UNARY(std::is_same_v<decltype(*as_const(ref)), const int&>);
        CHECK_UNARY(std::is_same_v<decltype(*as_rvalue(ref)), const int&>);

        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &a);
        CHECK_EQ(*ref, 1);

        ref = opt::none;
        CHECK_UNARY_FALSE(ref.has_value());

        int b = 2;
        ref = b;
        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &b);
        CHECK_EQ(*ref, 2);

        b = 3;
        CHECK_EQ(*ref, 3);

        ref = opt::option<int&>{b};
        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &b);
        CHECK_EQ(*ref, 3);

        ref = opt::option<int&>{opt::none};
        CHECK_UNARY_FALSE(ref.has_value());
        ref = opt::option<const int&>{opt::none};
        CHECK_UNARY_FALSE(ref.has_value());

        ref = opt::option<const int&>{std::ref(a)};
        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &a);

        ref = opt::option<const int&>{std::ref(b)};
        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &b);

        ref = opt::option<const int&>{std::cref(b)};
        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &b);

        ref = std::ref(a);
        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &a);
        CHECK_EQ(*ref, 1);

        ref = std::cref(a);
        CHECK(ref.has_value());

        ref = std::ref(b);
        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &b);

        ref = std::cref(b);
        CHECK(ref.has_value());
        CHECK_EQ(&(ref.get()), &b);

        const opt::option<const int&> refc{a};
        CHECK(refc.has_value());
        CHECK_EQ(&(refc.get()), &a);
        CHECK_EQ(*refc, 1);
    }
    SUBCASE("function") {
        opt::option<int(&)(int)> a;
        CHECK_UNARY_FALSE(a.has_value());
        const auto& f1 = [](int x) { return x + 1; };
        a = *+f1;
        CHECK(a.has_value());
        CHECK_EQ(a.ptr_or_null(), +f1);

        opt::option<int(&)(int)> b = *+[](int x) { return x + 1; };
        CHECK(b.has_value());
        a = b;
        CHECK(a.has_value());
        CHECK(b.has_value());
        CHECK_EQ(&*a, &*b);
    }
}

// NOLINTBEGIN(performance-move-const-arg,bugprone-use-after-move)
TEST_CASE("rvalue reference") {
    int a = 1;
    opt::option<int&&> b{std::move(a)};
    CHECK_EQ(b.get(), 1);
    CHECK_EQ(*b, 1);
    CHECK_EQ(std::as_const(b).get(), 1);
    CHECK_EQ(*std::as_const(b), 1);
    CHECK_EQ(as_rvalue(b).get(), 1);
    CHECK_EQ(*as_rvalue(b), 1);
    CHECK_EQ(as_const_rvalue(b).get(), 1);
    CHECK_EQ(*as_const_rvalue(b), 1);
    opt::option<const int&&> c{std::move(a)};
    CHECK_EQ(c.get(), 1);
    CHECK_EQ(*c, 1);
    CHECK_EQ(std::as_const(c).get(), 1);
    CHECK_EQ(*std::as_const(c), 1);
    CHECK_EQ(as_rvalue(c).get(), 1);
    CHECK_EQ(*as_rvalue(c), 1);
    CHECK_EQ(as_const_rvalue(c).get(), 1);
    CHECK_EQ(*as_const_rvalue(c), 1);
}
// NOLINTEND(performance-move-const-arg,bugprone-use-after-move)

TEST_CASE("tuple like") {
    SUBCASE("std::tuple") {
        CHECK(sizeof(opt::option<std::tuple<int, float>>) == sizeof(std::tuple<int, float>));
        CHECK(sizeof(opt::option<std::tuple<float, int>>) == sizeof(std::tuple<float, int>));
        CHECK(sizeof(opt::option<std::tuple<int, long>>) > sizeof(std::tuple<int, long>));

        opt::option<std::tuple<int, float>> a{1, 2.5f};
        CHECK(a.has_value());
        CHECK_EQ(std::get<0>(*a), 1);
        CHECK_EQ(std::get<1>(*a), 2.5f);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());

        CHECK_EQ(opt::get<0>(a), opt::none);
        CHECK_EQ(opt::get<1>(a), opt::none);
        CHECK_EQ(opt::get<int>(a), opt::none);
        CHECK_EQ(opt::get<float>(a), opt::none);
        a.emplace(1, 2.f);
        CHECK_EQ(opt::get<0>(a), 1);
        CHECK_EQ(opt::get<1>(a), 2.f);
        CHECK_EQ(opt::get<int>(a), 1);
        CHECK_EQ(opt::get<float>(a), 2.f);
        *opt::get<0>(a) = 2;
        CHECK_EQ(std::get<0>(*a), 2);
        *opt::get<1>(a) = 10.f;
        CHECK_EQ(std::get<1>(*a), 10.f);
        *opt::get<int>(a) = 123;
        CHECK_EQ(std::get<0>(*a), 123);
        *opt::get<float>(a) = 32.f;
        CHECK_EQ(std::get<1>(*a), 32.f);

        CHECK_UNARY(std::is_same_v<decltype(opt::get<0>(a)), opt::option<int&>>);
        CHECK_UNARY(std::is_same_v<decltype(opt::get<0>(std::as_const(a))), opt::option<const int&>>);
        CHECK_UNARY(std::is_same_v<decltype(opt::get<0>(as_rvalue(a))), opt::option<int&&>>);
        CHECK_UNARY(std::is_same_v<decltype(opt::get<0>(as_const_rvalue(a))), opt::option<const int&&>>);

        opt::option<std::tuple<float, double>> b{2.56f, 3.1415};
        CHECK(sizeof(b) == sizeof(std::tuple<float, double>));
        CHECK(b.has_value());
        CHECK_EQ(std::get<0>(*b), 2.56f);
        CHECK_EQ(std::get<1>(*b), 3.1415);
        b.reset();
        CHECK_UNARY_FALSE(b.has_value());

        opt::option<std::tuple<float, int, double>> c{1.f, 2, 3.};
        CHECK(sizeof(c) == sizeof(std::tuple<float, int, double>));
        CHECK(c.has_value());
        CHECK_EQ(std::get<0>(*c), 1.f);
        CHECK_EQ(std::get<1>(*c), 2);
        CHECK_EQ(std::get<2>(*c), 3.);
        c.reset();
        CHECK_UNARY_FALSE(c.has_value());

        opt::option<std::tuple<std::tuple<float, int>, long>> d{{{1.f, 2}, 3L}};
        CHECK(sizeof(d) == sizeof(std::tuple<std::tuple<float, int>, long>));
        CHECK(d.has_value());
        CHECK_EQ(std::get<0>(std::get<0>(*d)), 1.f);
        CHECK_EQ(std::get<1>(std::get<0>(*d)), 2);
        CHECK_EQ(std::get<1>(*d), 3L);
        d.reset();
        CHECK_UNARY_FALSE(c.has_value());

        opt::option<std::tuple<int, std::tuple<float, long>>> e{{1, {2.5f, 100L}}};
        CHECK(sizeof(e) == sizeof(std::tuple<int, std::tuple<float, long>>));
        CHECK(e.has_value());
        CHECK_EQ(std::get<0>(*e), 1);
        CHECK_EQ(std::get<0>(std::get<1>(*e)), 2.5f);
        CHECK_EQ(std::get<1>(std::get<1>(*e)), 100L);
        e.reset();
        CHECK_UNARY_FALSE(e.has_value());
    }
    SUBCASE("std::pair") {
        CHECK(sizeof(opt::option<std::pair<int, long>>) > sizeof(std::pair<int, long>));

        opt::option<std::pair<int, float>> a{1, 2.f};
        CHECK(sizeof(a) == sizeof(std::pair<int, float>));
        CHECK(a.has_value());
        CHECK_EQ(std::get<0>(*a), 1);
        CHECK_EQ(std::get<1>(*a), 2.f);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());

        CHECK_EQ(opt::get<0>(a), opt::none);
        CHECK_EQ(opt::get<1>(a), opt::none);
        a.emplace(10, 20.f);
        CHECK_EQ(opt::get<0>(a), 10);
        CHECK_EQ(opt::get<1>(a), 20.f);
        CHECK_EQ(opt::get<int>(a), 10);
        CHECK_EQ(opt::get<float>(a), 20.f);
        *opt::get<0>(a) = 100;
        CHECK_EQ(std::get<0>(*a), 100);
        *opt::get<1>(a) = 200.f;
        CHECK_EQ(std::get<1>(*a), 200.f);
        *opt::get<int>(a) = 300;
        CHECK_EQ(std::get<0>(*a), 300);
        *opt::get<float>(a) = 400.f;
        CHECK_EQ(std::get<1>(*a), 400.f);

        opt::option<std::pair<float, int>> b{3.f, -1};
        CHECK(sizeof(b) == sizeof(std::pair<float, int>));
        CHECK(b.has_value());
        CHECK_EQ(std::get<0>(*b), 3.f);
        CHECK_EQ(std::get<1>(*b), -1);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());

        opt::option<std::pair<std::pair<float, int>, unsigned>> c{{{100.f, 250}, 194u}};
        CHECK(sizeof(c) == sizeof(std::pair<std::pair<float, int>, unsigned>));
        CHECK(c.has_value());
        CHECK_EQ(c->first.first, 100.f);
        CHECK_EQ(c->first.second, 250);
        CHECK_EQ(c->second, 194u);
        c.reset();
        CHECK_UNARY_FALSE(c.has_value());

        opt::option<std::pair<unsigned, std::pair<float, int>>> d{{2u, {3.f, -1}}};
        CHECK(sizeof(d) == sizeof(std::pair<unsigned, std::pair<float, int>>));
        CHECK(d.has_value());
        CHECK_EQ(d->first, 2u);
        CHECK_EQ(d->second.first, 3.f);
        CHECK_EQ(d->second.second, -1);
        d.reset();
        CHECK_UNARY_FALSE(d.has_value());
    }
    SUBCASE("std::array") {
        CHECK(sizeof(opt::option<std::array<int, 2>>) > sizeof(std::array<int, 2>));

        opt::option<std::array<float, 2>> a{{1.f, 2.f}};
        CHECK(sizeof(a) == sizeof(std::array<float, 2>));

        CHECK(a.has_value());
        CHECK_EQ((*a)[0], 1.f);
        CHECK_EQ(std::get<1>(*a), 2.f);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());

        opt::option<std::array<double, 1>> b{{1000.1}};
        CHECK(sizeof(b) == sizeof(std::array<double, 1>));

        CHECK(b.has_value());
        CHECK_EQ((*b)[0], 1000.1);
        b.reset();
        CHECK_UNARY_FALSE(b.has_value());
    }
}

TEST_CASE("std::unique_ptr") {
    opt::option<std::unique_ptr<int>> a;
    CHECK_UNARY_FALSE(a.has_value());
    a = std::make_unique<int>(1);
    CHECK(a.has_value());
    CHECK_EQ(**a, 1);
    a = std::make_unique<int>(2);
    CHECK(a.has_value());
    CHECK_EQ(**a, 2);

    a.reset();
    CHECK_UNARY_FALSE(a.has_value());
    a = std::make_unique<int>(3);
    CHECK(a.has_value());
    CHECK_EQ(**a, 3);
    a->reset();
    CHECK(a.has_value());
    CHECK_EQ(*a, nullptr);

    CHECK(sizeof(opt::option<std::unique_ptr<int>>) == sizeof(std::unique_ptr<int>));

    opt::option<std::unique_ptr<int>> b = std::make_unique<int>(1);
    CHECK(b.has_value());
    CHECK_EQ(**b, 1);
    b.reset();
    CHECK_UNARY_FALSE(b.has_value());
    b.reset();
    CHECK_UNARY_FALSE(b.has_value());
    b.emplace(std::make_unique<int>(2));
    CHECK_EQ(**b, 2);
    b.emplace(std::make_unique<int>(3));
    CHECK_EQ(**b, 3);

    struct my_type {
        int x;
        int func() { return (x += 1); }
    };
    opt::option<std::unique_ptr<my_type>> c;
    CHECK_EQ(sizeof(c), sizeof(std::unique_ptr<my_type>));
    CHECK_UNARY_FALSE(c.has_value());
    c.emplace(std::make_unique<my_type>(my_type{1}));
    CHECK_EQ((**c).x, 1);
    c->reset();
    CHECK_EQ(c, nullptr);
}

#ifdef OPTION_HAS_BOOST_PFR

TEST_CASE("aggregate") {
    SUBCASE("basic") {
        struct s1 { int x; float y; };

        opt::option<s1> a{1, 2.f};
        CHECK_EQ(sizeof(a), sizeof(s1));
        CHECK(a.has_value());
        CHECK_EQ(a->x, 1);
        CHECK_EQ(a->y, 2.f);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());

        struct s2 { float x; int y; };
        opt::option<s2> b{10.5f, -1};
        CHECK_EQ(sizeof(b), sizeof(s2));
        CHECK(b.has_value());
        CHECK_EQ(b->x, 10.5f);
        CHECK_EQ(b->y, -1);
        b.reset();
        CHECK_UNARY_FALSE(b.has_value());

        struct s3 { int x; long y; };
        const opt::option<s3> c{5, 10L};
        CHECK(sizeof(c) > sizeof(s3));

        struct s4 { float x; double y; };
        opt::option<s4> d{10.5f, 100.};
        CHECK_EQ(sizeof(d), sizeof(s4));
        CHECK(d.has_value());
        CHECK_EQ(d->x, 10.5f);
        CHECK_EQ(d->y, 100.);
        d.reset();
        CHECK_UNARY_FALSE(d.has_value());

        struct s5 { int x; long y; float z; };
        opt::option<s5> e{10, 15L, 0.f};
        CHECK_EQ(sizeof(e), sizeof(s5));
        CHECK(e.has_value());
        CHECK_EQ(e->x, 10);
        CHECK_EQ(e->y, 15L);
        CHECK_EQ(e->z, 0.f);
        e.reset();
        CHECK_UNARY_FALSE(e.has_value());
    }
    SUBCASE("nested") {
        struct s1 { int x; float y; };
        struct s2 { int x; s1 y; };

        opt::option<s2> a{1, s1{2, 3.f}};
        CHECK_EQ(sizeof(a), sizeof(s2));
        CHECK(a.has_value());
        CHECK_EQ(a->x, 1);
        CHECK_EQ(a->y.x, 2);
        CHECK_EQ(a->y.y, 3.f);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());

        struct s3 { s1 x; float y; };
        opt::option<s3> b{s1{1, 2.f}, 3.f};
        CHECK_EQ(sizeof(b), sizeof(s3));
        CHECK(b.has_value());
        CHECK_EQ(b->x.x, 1);
        CHECK_EQ(b->x.y, 2.f);
        CHECK_EQ(b->y, 3.f);
        b.reset();
        CHECK_UNARY_FALSE(b.has_value());

        struct s4 { float x; int y; };
        struct s5 { s4 x; s4 y; };
        opt::option<s5> c{s4{1.f, 2}, s4{3.f, 4}};
        CHECK_EQ(sizeof(c), sizeof(s5));
        CHECK(c.has_value());
        CHECK_EQ(c->x.x, 1.f);
        CHECK_EQ(c->x.y, 2);
        CHECK_EQ(c->y.x, 3.f);
        CHECK_EQ(c->y.y, 4.f);
        c.reset();
        CHECK_UNARY_FALSE(c.has_value());

        struct s6 {};
        struct s7 { s6 x; int y; };
        opt::option<s7> d{s6{}, 1};
        CHECK_EQ(sizeof(d), sizeof(s7));
        CHECK(d.has_value());
        CHECK_EQ(d->y, 1);
        d.reset();
        CHECK_UNARY_FALSE(d.has_value());
    }
}

#endif

TEST_CASE("pointer to member data") {
    // NOLINTBEGIN(readability-make-member-function-const)
    SUBCASE("no inheritance") {
        struct s1 {
            int x;
            int y;
        };
        opt::option<int s1::*> a;
        CHECK_EQ(sizeof(a), sizeof(int s1::*));

        CHECK_UNARY_FALSE(a.has_value());
        a = &s1::x;
        CHECK(a.has_value());
        CHECK_EQ(a, &s1::x);
        CHECK_NE(a, &s1::y);
        a = &s1::y;
        CHECK(a.has_value());
        CHECK_EQ(a, &s1::y);
        CHECK_NE(a, &s1::x);

        a = nullptr;
        CHECK(a.has_value());
        CHECK_EQ(a, nullptr);

        struct s2 {
            int x;
            int y;
            int z;
            int w;
        };
        opt::option<int s2::*> b = &s2::y;
        CHECK_EQ(sizeof(a), sizeof(int s2::*));

        CHECK(b.has_value());
        CHECK_EQ(b, &s2::y);
        b = &s2::x;
        CHECK(b.has_value());
        CHECK_EQ(b, &s2::x);
        b = &s2::z;
        CHECK(b.has_value());
        CHECK_EQ(b, &s2::z);

        b = nullptr;
        CHECK(b.has_value());
        CHECK_EQ(b, nullptr);
    }
    SUBCASE("single inheritance") {
        struct s1 {
            int x;
            int y;
        };
        struct s2 : s1 {
            int z;
            int w;
        };
        opt::option<int s2::*> a;
        CHECK_EQ(sizeof(a), sizeof(int s2::*));

        CHECK_UNARY_FALSE(a.has_value());
        a = &s2::x;
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::x);
        a.emplace(&s2::z);
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::z);
        a = &s2::w;
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::w);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
        a = &s2::y;
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::y);

        a = nullptr;
        CHECK(a.has_value());
        CHECK_EQ(a, nullptr);

        opt::option<int s2::*> b = &s2::z;
        CHECK(b.has_value());
        CHECK_EQ(b, &s2::z);
        b = &s1::x;
        CHECK(b.has_value());
        CHECK_EQ(b, &s1::x);
        CHECK_EQ(b, &s2::x);

        b = nullptr;
        CHECK(b.has_value());
        CHECK_EQ(b, nullptr);

        b.emplace(&s1::y);
        CHECK(b.has_value());
        CHECK_EQ(b, &s1::y);
        CHECK_EQ(b, &s2::y);
    }
    SUBCASE("multiple inheritance") {
        struct s1 {
            int x;
            int y;
        };
        struct s2 {
            int z;
            int w;
        };
        struct s3 : s1, s2 {
            int v;
        };
        opt::option<int s3::*> a;
        CHECK_EQ(sizeof(a), sizeof(int s3::*));

        CHECK_UNARY_FALSE(a.has_value());
        a = &s3::v;
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::v);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
        a = &s3::x;
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::x);
#if !(OPTION_MSVC && !OPTION_CLANG) // MSVC bug
        a = &s3::z;
        CHECK(a.has_value());
        CHECK_EQ(*a, &s3::z);
#endif
        a.emplace(&s3::x);
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::x);

        a = nullptr;
        CHECK(a.has_value());
        CHECK_EQ(a, nullptr);
        CHECK_NE(a, &s3::x);
    }
}

TEST_CASE("pointer to member function") {
    SUBCASE("no inheritance") {
        struct s1 {
            int x;
            int f1() { return x + 100; }
            int f2() { return x + 200; }
        };
        opt::option<int (s1::*)()> a;
        s1 s1_var{12};
        CHECK_EQ(sizeof(a), sizeof(int (s1::*)()));

        CHECK_UNARY_FALSE(a.has_value());
        a = &s1::f1;
        CHECK(a.has_value());
        CHECK_EQ(a, &s1::f1);
        CHECK_EQ((s1_var.**a)(), 112);

        a.reset();
        CHECK_UNARY_FALSE(a.has_value());

        a.emplace(&s1::f2);
        CHECK(a.has_value());
        CHECK_EQ(a, &s1::f2);
        CHECK_EQ((s1_var.**a)(), 212);

        a = nullptr;
        CHECK(a.has_value());
        CHECK_EQ(a, nullptr);
        CHECK_NE(a, &s1::f2);
    }
    SUBCASE("single inheritance") {
        struct s1 {
            int x{};
            int f1(int a) { return x * a; }
            int f2(int a) { return x + a; }
        };
        struct s2 : s1 {
            int y{};
            int f3(int a) { return y - a; }
            int f4(int a) { return a - y; }
        };
        s2 s2_var{{5}, 10};
        opt::option<int (s2::*)(int)> a;
        CHECK_EQ(sizeof(a), sizeof(int (s2::*)(int)));
        CHECK_UNARY_FALSE(a.has_value());

        a = &s2::f3;
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::f3);
        CHECK_EQ((s2_var.**a)(2), 8);

        a = &s2::f4;
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::f4);
        CHECK_EQ((s2_var.**a)(5), -5);

        a = &s2::f1;
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::f1);
        CHECK_EQ((s2_var.**a)(10), 50);

        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
        a.emplace(&s1::f1);
        CHECK(a.has_value());
        CHECK_EQ(a, &s1::f1);
        CHECK_EQ(a, &s2::f1);
        CHECK_EQ((s2_var.**a)(-2), -10);

        a.emplace(&s1::f2);
        CHECK(a.has_value());
        CHECK_EQ(a, &s1::f2);
        CHECK_EQ(a, &s2::f2);
        CHECK_EQ((s2_var.**a)(-10), -5);

        a = nullptr;
        CHECK(a.has_value());
        CHECK_EQ(a, nullptr);
        CHECK_NE(a, &s1::f2);
    }
    SUBCASE("multiple inheritance") {
        struct s1 {
            int x = 100;
            int f1() { return x + 1; }
            int f2() { return x + 2; }
        };
        struct s2 {
            int y = 2000;
            int f3() { return y + 3; }
            int f4() { return y + 4; }
        };
        struct s3 : s1, s2 {
            int z = 30000;
            int f5() { return z + x + y + 5; }
            int f6() { return z + x + y + 6; }
        };
        s3 var;
        opt::option<int (s3::*)()> a;
        CHECK_EQ(sizeof(a), sizeof(int (s3::*)()));
        CHECK_UNARY_FALSE(a.has_value());

        a = &s3::f5;
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::f5);
        CHECK_EQ((var.**a)(), 32105);

        a = &s3::f6;
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::f6);
        CHECK_EQ((var.**a)(), 32106);

        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
        a.emplace(&s3::f1);
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::f1);
        CHECK_EQ((var.**a)(), 101);
        a.emplace(&s3::f2);
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::f2);
        CHECK_EQ((var.**a)(), 102);

        a = &s3::f3;
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::f3);
        CHECK_EQ((var.**a)(), 2003);
        a = &s3::f4;
        CHECK(a.has_value());
        CHECK_EQ(a, &s3::f4);
        CHECK_EQ((var.**a)(), 2004);

        a = nullptr;
        CHECK(a.has_value());
        CHECK_EQ(a, nullptr);
        CHECK_NE(a, &s3::f4);
    }
    // NOLINTBEGIN(bugprone-compare-pointer-to-member-virtual-function)
    SUBCASE("virtual inheritance") {
        struct s1 {
            int x = 100;
            virtual ~s1() = default;

            virtual int f1() { return x + 1; }
            virtual int f2() { return x + 2; }
        };
        struct s2 : s1 {
            int y = 2000;
            int f1() override { return x + y + 3; }
            int f3() { return x + y + 4; } // NOLINT(bugprone-virtual-near-miss)
        };
        s2 var;
        opt::option<int (s2::*)()> a;
        CHECK_EQ(sizeof(a), sizeof(int (s2::*)()));
        CHECK_UNARY_FALSE(a.has_value());

        a = &s2::f1;
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::f1);
        CHECK_EQ((var.**a)(), 2103);

        a.emplace(&s2::f2);
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::f2);
        CHECK_EQ((var.**a)(), 102);

        a.emplace(nullptr);
        CHECK(a.has_value());
        CHECK_EQ(a, nullptr);
        CHECK_NE(a, &s2::f2);

        a = &s2::f3;
        CHECK(a.has_value());
        CHECK_EQ(a, &s2::f3);
        CHECK_EQ((var.**a)(), 2104);
    }
    SUBCASE("unknown inheritance") {
        struct s1;

        opt::option<int (s1::*)()> a;
        CHECK_UNARY_FALSE(a.has_value());

        struct s2 {
            int y = 100;

            virtual ~s2() = default;
            virtual int f1() { return y + 1; }
        };
        struct s3 {
            int z = 2000;

            virtual ~s3() = default;
            virtual int f2() { return z + 2; }
        };

        struct s1 : s2, s3 {
            int x = 30000;

            int f1() override { return x + y + 3; }
        };
        s1 var;

        a = &s1::f1;
        CHECK(a.has_value());
        CHECK_EQ(a, &s1::f1);
        CHECK_EQ((var.**a)(), 30103);
        
        a = &s1::f2;
        CHECK(a.has_value());
        CHECK_EQ(a, &s1::f2);
        CHECK_EQ((var.**a)(), 2002);
        
        a = nullptr;
        CHECK(a.has_value());
        CHECK_EQ(a, nullptr);
        CHECK_NE(a, &s1::f2);
    }
    // NOLINTEND(bugprone-compare-pointer-to-member-virtual-function)
    // NOLINTEND(readability-make-member-function-const)
}

TEST_CASE("with sentinel member") {
    SUBCASE("uint8_t") {
        struct s1 { std::uint8_t SENTINEL{}; };
    
        opt::option<s1> a;
        CHECK_EQ(sizeof(opt::option<s1>), sizeof(s1));
    
        CHECK_UNARY_FALSE(a.has_value());
        a = s1{};
        CHECK(a.has_value());
    
        *a = s1{};
        CHECK(a.has_value());
    
        opt::option<s1> b{a};
        CHECK(b.has_value());
        b.reset();
        CHECK_UNARY_FALSE(b.has_value());
        b = a;
        CHECK(b.has_value());
    
        struct s2 { int x; std::uint8_t SENTINEL{}; };
    
        opt::option<s2> c;
        CHECK_EQ(sizeof(opt::option<s2>), sizeof(s2));
    
        CHECK_UNARY_FALSE(c.has_value());
        c.emplace(5);
        CHECK(c.has_value());
        CHECK_EQ(c->x, 5);
    
        c->x = 10;
        CHECK(c.has_value());
        CHECK_EQ(c->x, 10);
        *c = s2{-100};
        CHECK(c.has_value());
        CHECK_EQ(c->x, -100);
    
#if !OPTION_MSVC // fatal error
        opt::option<s2> d{c};
        CHECK(d.has_value());
        CHECK_EQ(d->x, -100);
        
        d = opt::none;
        CHECK_UNARY_FALSE(d.has_value());
        
        d = c;
        CHECK(d.has_value());
        CHECK_EQ(d->x, -100);
#endif
    }
    SUBCASE("uint16_t") {
        struct s1 { std::uint16_t SENTINEL{}; };

        opt::option<s1> a;
        CHECK_EQ(sizeof(opt::option<s1>), sizeof(s1));

        CHECK_UNARY_FALSE(a.has_value());
        a.emplace();
        CHECK(a.has_value());

        a = s1{};
        CHECK(a.has_value());

        opt::option<s1> b = a;
        CHECK(b.has_value());

        *b = s1{};
        CHECK(b.has_value());

        *b = *a;
        CHECK(b.has_value());
        b.reset();
        b = a;
        CHECK(b.has_value());

        a = b;
        CHECK(a.has_value());

        struct s2 { int x; std::uint16_t SENTINEL{}; };

        opt::option<s2> c;
        CHECK_EQ(sizeof(opt::option<s2>), sizeof(s2));

        CHECK_UNARY_FALSE(c.has_value());

        c = s2{123};
        CHECK(c.has_value());
        CHECK_EQ(c->x, 123);

        *c = s2{-1};
        CHECK(c.has_value());
        CHECK_EQ(c->x, -1);

        c.reset();
        CHECK_UNARY_FALSE(c.has_value());
    }
    SUBCASE("uint32_t") {
        struct s1 { std::uint32_t SENTINEL{}; };

        opt::option<s1> a;
        CHECK_EQ(sizeof(opt::option<s1>), sizeof(s1));

        CHECK_UNARY_FALSE(a.has_value());
        a.emplace();
        CHECK(a.has_value());

        a = s1{};
        CHECK(a.has_value());

        opt::option<s1> b = a;
        CHECK(b.has_value());

        *b = s1{};
        CHECK(b.has_value());

        *b = *a;
        CHECK(b.has_value());
        b.reset();
        b = a;
        CHECK(b.has_value());

        a = b;
        CHECK(a.has_value());

        struct s2 { int x; std::uint32_t SENTINEL{}; };

        opt::option<s2> c;
        CHECK_EQ(sizeof(opt::option<s2>), sizeof(s2));

        CHECK_UNARY_FALSE(c.has_value());

        c = s2{123};
        CHECK(c.has_value());
        CHECK_EQ(c->x, 123);

        *c = s2{-1};
        CHECK(c.has_value());
        CHECK_EQ(c->x, -1);

        c.reset();
        CHECK_UNARY_FALSE(c.has_value());
    }
}

TEST_CASE("polymorphic") {
    SUBCASE("no inheritance") {
        struct s1 {
            int x;
            s1(int x_) : x{x_} {}

            s1(const s1&) = default;
            s1& operator=(const s1&) = default;
            virtual ~s1() = default;
        };
        opt::option<s1> a;
        CHECK_EQ(sizeof(a), sizeof(s1));
        CHECK_UNARY_FALSE(a.has_value());

        a = s1{1};
        CHECK(a.has_value());
        CHECK_EQ(a->x, 1);

        a = s1{10};
        CHECK(a.has_value());
        CHECK_EQ(a->x, 10);

        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
    }
    SUBCASE("single inheritance") {
        SUBCASE("virtual derived") {
            struct s1 {
                int x;
            };
            struct s2 : s1 {
                int y;
                s2(int x_, int y_) : s1{x_}, y{y_} {}

                s2(const s2&) = default;
                s2& operator=(const s2&) = default;
                virtual ~s2() = default;
            };
            opt::option<s2> a;
            CHECK_EQ(sizeof(a), sizeof(s2));
            CHECK_UNARY_FALSE(a.has_value());

            a = s2{1, 2};
            CHECK(a.has_value());
            CHECK_EQ(a->x, 1);
            CHECK_EQ(a->y, 2);

            a = s2{10, 20};
            CHECK(a.has_value());
            CHECK_EQ(a->x, 10);
            CHECK_EQ(a->y, 20);

            a.reset();
            CHECK_UNARY_FALSE(a.has_value());
        }
        SUBCASE("virtual base, virtual derived") {
            struct s3 {
                int x;
                s3(int x_) : x{x_} {}

                s3(const s3&) = default;
                s3& operator=(const s3&) = default;
                virtual ~s3() = default;
            };
            struct s4 : s3 {
                int y;
                s4(int x_, int y_) : s3{x_}, y{y_} {}

                s4(const s4&) = default;
                s4& operator=(const s4&) = default;
                virtual ~s4() override = default; // NOLINT(modernize-use-override)
            };
            opt::option<s4> b;
            CHECK_EQ(sizeof(b), sizeof(s4));
            CHECK_UNARY_FALSE(b.has_value());

            b.emplace(1, 2);
            CHECK(b.has_value());
            CHECK_EQ(b->x, 1);
            CHECK_EQ(b->y, 2);

            b = s4{10, 20};
            CHECK(b.has_value());
            CHECK_EQ(b->x, 10);
            CHECK_EQ(b->y, 20);

            b.reset();
            CHECK_UNARY_FALSE(b.has_value());
        }
        SUBCASE("virtual base") {
            struct s5 {
                int x;
                s5(int x_) : x{x_} {}

                s5(const s5&) = default;
                s5& operator=(const s5&) = default;
                virtual ~s5() = default;
            };
            struct s6 : s5 {
                int y;
                s6(int x_, int y_) : s5{x_}, y{y_} {}
            };
            opt::option<s6> c;
            CHECK_EQ(sizeof(c), sizeof(s6));
            CHECK_UNARY_FALSE(c.has_value());

            c = s6{1, 2};
            CHECK(c.has_value());
            CHECK_EQ(c->x, 1);
            CHECK_EQ(c->y, 2);

            c = s6{10, 20};
            CHECK(c.has_value());
            CHECK_EQ(c->x, 10);
            CHECK_EQ(c->y, 20);

            c.reset();
            CHECK_UNARY_FALSE(c.has_value());
        }
    }
    SUBCASE("multiple inheritance") {
        SUBCASE("virtual derived") {
            struct s1 {
                int x;
            };
            struct s2 {
                int y;
            };
            struct s3 : s1, s2 {
                int z;
                s3(int x_, int y_, int z_) : s1{x_}, s2{y_}, z{z_} {}

                s3(const s3&) = default;
                s3& operator=(const s3&) = default;
                virtual ~s3() = default;
            };
            opt::option<s3> a;
            CHECK_EQ(sizeof(a), sizeof(s3));
            CHECK_UNARY_FALSE(a.has_value());

            a = s3{1, 2, 3};
            CHECK(a.has_value());
            CHECK_EQ(a->x, 1);
            CHECK_EQ(a->y, 2);
            CHECK_EQ(a->z, 3);

            a = s3{10, 20, 30};
            CHECK(a.has_value());
            CHECK_EQ(a->x, 10);
            CHECK_EQ(a->y, 20);
            CHECK_EQ(a->z, 30);

            a.reset();
            CHECK_UNARY_FALSE(a.has_value());
        }
        SUBCASE("virtual derived, single virtual base") {
            struct s4 {
                int x;
                s4(int x_) : x{x_} {}

                s4(const s4&) = default;
                s4& operator=(const s4&) = default;
                virtual ~s4() = default;
            };
            struct s5 {
                int y;
            };
            struct s6 : s4, s5 {
                int z;
                s6(int x_, int y_, int z_) : s4{x_}, s5{y_}, z{z_} {}

                s6(const s6&) = default;
                s6& operator=(const s6&) = default;
                virtual ~s6() override = default; // NOLINT(modernize-use-override)
            };
            opt::option<s6> b;
            CHECK_EQ(sizeof(b), sizeof(s6));
            CHECK_UNARY_FALSE(b.has_value());

            b.emplace(1, 2, 3);
            CHECK(b.has_value());
            CHECK_EQ(b->x, 1);
            CHECK_EQ(b->y, 2);
            CHECK_EQ(b->z, 3);

            b = s6{10, 20, 30};
            CHECK(b.has_value());
            CHECK_EQ(b->x, 10);
            CHECK_EQ(b->y, 20);
            CHECK_EQ(b->z, 30);

            b.reset();
            CHECK_UNARY_FALSE(b.has_value());
        }
        SUBCASE("single virtual derived") {
            struct s7 {
                int x;
                s7(int x_) : x{x_} {}

                s7(const s7&) = default;
                s7& operator=(const s7&) = default;
                virtual ~s7() = default;
            };
            struct s8 {
                int y;
            };
            struct s9 : s7, s8 {
                int z;
                s9(int x_, int y_, int z_) : s7{x_}, s8{y_}, z{z_} {}
            };
            opt::option<s9> c;
            CHECK_EQ(sizeof(c), sizeof(s9));
            CHECK_UNARY_FALSE(c.has_value());

            c = s9{1, 2, 3};
            CHECK(c.has_value());
            CHECK_EQ(c->x, 1);
            CHECK_EQ(c->y, 2);
            CHECK_EQ(c->z, 3);

            c = s9{10, 20, 30};
            CHECK(c.has_value());
            CHECK_EQ(c->x, 10);
            CHECK_EQ(c->y, 20);
            CHECK_EQ(c->z, 30);

            c.reset();
            CHECK_UNARY_FALSE(c.has_value());
        }
        SUBCASE("double virtual base") {
            struct s10 {
                int x;
                s10(int x_) : x{x_} {}

                s10(const s10&) = default;
                s10& operator=(const s10&) = default;
                virtual ~s10() = default;
            };
            struct s11 {
                int y;
                s11(int y_) : y{y_} {}

                s11(const s11&) = default;
                s11& operator=(const s11&) = default;
                virtual ~s11() = default;
            };
            struct s12 : s10, s11 {
                int z;
                s12(int x_, int y_, int z_) : s10{x_}, s11{y_}, z{z_} {}
            };
            opt::option<s12> d;
            CHECK_EQ(sizeof(d), sizeof(s12));
            CHECK_UNARY_FALSE(d.has_value());

            d = s12{1, 2, 3};
            CHECK(d.has_value());
            CHECK_EQ(d->x, 1);
            CHECK_EQ(d->y, 2);
            CHECK_EQ(d->z, 3);

            d = s12{10, 20, 30};
            CHECK(d.has_value());
            CHECK_EQ(d->x, 10);
            CHECK_EQ(d->y, 20);
            CHECK_EQ(d->z, 30);

            d.reset();
            CHECK_UNARY_FALSE(d.has_value());
        }
    }
}

TEST_CASE("enumeration") {
    SUBCASE("with sentinel") {
        enum class enum1 { // NOLINT(performance-enum-size)
            a, b, c, d, e,
            SENTINEL
        };
        const auto combine = [](auto... vals) {
            return enum1((int(vals) | ...));
        };

        opt::option<enum1> a;
        CHECK_EQ(sizeof(a), sizeof(enum1));
        a = enum1::a;
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, enum1::a);
        a = enum1::b;
        CHECK_EQ(a, enum1::b);
        a = enum1::c;
        CHECK_EQ(a, enum1::c);
        a = enum1::d;
        CHECK_EQ(a, enum1::d);
        a = enum1::e;
        CHECK_EQ(a, enum1::e);

        a.get_unchecked() = enum1::SENTINEL;
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_EQ(a.get_unchecked(), enum1::SENTINEL);

        a.get_unchecked() = combine(enum1::b, enum1::e);
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_EQ(a.get_unchecked(), enum1::SENTINEL);

        CHECK_GT(sizeof(opt::option<opt::option<enum1>>), sizeof(enum1));

        enum class enum2 : std::int8_t {
            a1 = -1, a2 = 0, a3 = 1, a4 = 2,
            SENTINEL = -10
        };
        opt::option<enum2> b;
        CHECK_EQ(sizeof(b), sizeof(enum2));

        b = enum2::a1;
        CHECK_EQ(b, enum2::a1);
        b = enum2::a2;
        CHECK_EQ(b, enum2::a2);
        b = enum2::a3;
        CHECK_EQ(b, enum2::a3);
        b = enum2::a4;
        CHECK_EQ(b, enum2::a4);

        b.reset();
        CHECK_UNARY_FALSE(b.has_value());
        CHECK_EQ(b.get_unchecked(), enum2::SENTINEL);
    }
    SUBCASE("with sentinel start") {
        enum class enum1 : std::uint8_t {
            a, b, c, d, e, f, g,
            SENTINEL_START
        };
        const auto combine = [](auto... vals) {
            return enum1((int(vals) | ...));
        };

        opt::option<enum1> a;
        CHECK_EQ(sizeof(a), sizeof(enum1));
        CHECK_UNARY_FALSE(a.has_value());

        a = enum1::a;
        CHECK_EQ(a, enum1::a);
        a = enum1::b;
        CHECK_EQ(a, enum1::b);
        a = enum1::c;
        CHECK_EQ(a, enum1::c);
        a = enum1::d;
        CHECK_EQ(a, enum1::d);
        a = enum1::e;
        CHECK_EQ(a, enum1::e);
        a = enum1::f;
        CHECK_EQ(a, enum1::f);
        a = enum1::g;
        CHECK_EQ(a, enum1::g);

        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_EQ(a.get_unchecked(), enum1::SENTINEL_START);

        a.get_unchecked() = combine(enum1::a, enum1::b, enum1::c, enum1::d, enum1::e, enum1::f, enum1::g);
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_EQ(a.get_unchecked(), enum1::SENTINEL_START);

        opt::option<opt::option<enum1>> b;
        CHECK_EQ(sizeof(b), sizeof(enum1));
        CHECK_UNARY_FALSE(b.has_value());

        b = enum1::a;
        CHECK_EQ(b, enum1::a);

        b->reset();
        CHECK_EQ(b->get_unchecked(), enum1::SENTINEL_START);

        b.reset();
        // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
        CHECK_EQ(b.get_unchecked().get_unchecked(), enum1(std::uint8_t(enum1::SENTINEL_START) + 1));

        enum class enum2 : std::uint8_t {
            a1, b1, c1,
            SENTINEL_START = 255
        };
        opt::option<enum2> c;
        CHECK_EQ(sizeof(c), sizeof(enum2));

        c = enum2::a1;
        CHECK_EQ(c, enum2::a1);
        c.reset();
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_EQ(c.get_unchecked(), enum2::SENTINEL_START);

        CHECK_GT(sizeof(opt::option<opt::option<enum2>>), sizeof(enum2));

        enum class enum3 : std::int8_t {
            a1 = -11, a2 = -12, a3 = -13, SENTINEL_START = -10
        };
        opt::option<enum3> d;
        CHECK_EQ(sizeof(d), sizeof(enum3));

        d = enum3::a1;
        CHECK_EQ(d, enum3::a1);
        d = enum3::a2;
        CHECK_EQ(d, enum3::a2);
        d = enum3::a3;
        CHECK_EQ(d, enum3::a3);

        d.reset();
        CHECK_UNARY_FALSE(d.has_value());
        CHECK_EQ(d.get_unchecked(), enum3::SENTINEL_START);
    }
    SUBCASE("with sentinel start") {
        enum class enum1 : std::uint8_t {
            a1, a2, a3, a4, a5,
            SENTINEL_START, SENTINEL_END = SENTINEL_START + 5,
            a6, a7, a8, a9, a10
        };
        using traits1 = opt::option_traits<enum1>;
        CHECK_EQ(traits1::max_level, 6);

        opt::option<enum1> a;
        CHECK_EQ(sizeof(a), sizeof(enum1));
        CHECK_UNARY_FALSE(a.has_value());

        a = enum1::a1;
        CHECK_EQ(a, enum1::a1);
        a = enum1::a5;
        CHECK_EQ(a, enum1::a5);
        a = enum1::a6;
        CHECK_EQ(a, enum1::a6);
        a = enum1::a10;
        CHECK_EQ(a, enum1::a10);

        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_EQ(a.get_unchecked(), enum1::SENTINEL_START);

        enum class enum2 : std::int8_t {
            a1 = -101, SENTINEL_START = -100, SENTINEL_END = SENTINEL_START + 1,
            a2, a3, a4
        };
        opt::option<enum2> b;
        CHECK_EQ(sizeof(b), sizeof(enum2));
        CHECK_UNARY_FALSE(b.has_value());

        b = enum2::a1;
        CHECK_EQ(b, enum2::a1);
        b = enum2::a2;
        CHECK_EQ(b, enum2::a2);
        b = enum2::a3;
        CHECK_EQ(b, enum2::a3);

        b.reset();
        CHECK_UNARY_FALSE(b.has_value());
        CHECK_EQ(b.get_unchecked(), enum2::SENTINEL_START);
    }
}

TEST_CASE("tuple like") {
    SUBCASE("member get<I>") {
        opt::option<my_tuple_0<int, long>> a;
        CHECK_GT(sizeof(a), sizeof(my_tuple_0<int, long>));
        CHECK_UNARY_FALSE(a.has_value());

        a.emplace(1, 2L);
        CHECK_EQ(a->get<0>(), 1);
        CHECK_EQ(a->get<1>(), 2L);

        opt::option<my_tuple_0<int, float>> b;
        CHECK_EQ(sizeof(b), sizeof(my_tuple_0<int, float>));
        CHECK_UNARY_FALSE(b.has_value());

        b.emplace(1, 2.f);
        CHECK_EQ(b->get<0>(), 1);
        CHECK_EQ(b->get<1>(), 2.f);
        
        opt::option<my_tuple_0<float, int>> c;
        CHECK_EQ(sizeof(c), sizeof(my_tuple_0<float, int>));
        CHECK_UNARY_FALSE(c.has_value());

        c.emplace(1.f, 2);
        CHECK_EQ(c->get<0>(), 1.f);
        CHECK_EQ(c->get<1>(), 2);

        opt::option<my_tuple_0<float, double>> d;
        CHECK_EQ(sizeof(d), sizeof(my_tuple_0<float, double>));
        CHECK_UNARY_FALSE(d.has_value());

        d.emplace(1.f, 2.);
        CHECK_EQ(d->get<0>(), 1.f);
        CHECK_EQ(d->get<1>(), 2.);
    }
    SUBCASE("ADL get<I>") {
        opt::option<my_ns::my_tuple<int, long, unsigned>> a;
        CHECK_GT(sizeof(a), sizeof(my_ns::my_tuple<int, long, unsigned>));
        CHECK_UNARY_FALSE(a.has_value());

        a.emplace(1, 2l, 3u);
        CHECK_EQ(my_ns::get<0>(*a), 1);
        CHECK_EQ(my_ns::get<1>(*a), 2l);
        CHECK_EQ(my_ns::get<2>(*a), 3u);

        opt::option<my_ns::my_tuple<int, long, float>> b;
        CHECK_EQ(sizeof(b), sizeof(my_ns::my_tuple<int, long, float>));
        CHECK_UNARY_FALSE(b.has_value());

        b.emplace(1, 2l, 3.f);
        CHECK_EQ(my_ns::get<0>(*b), 1);
        CHECK_EQ(my_ns::get<1>(*b), 2l);
        CHECK_EQ(my_ns::get<2>(*b), 3.f);
    }
}

template<class T, class Hash = std::hash<T>>
inline constexpr bool is_hashable = std::conjunction_v<
    std::is_default_constructible<Hash>,
    std::is_copy_constructible<Hash>,
    std::is_move_constructible<Hash>,
    std::is_destructible<Hash>,
    std::is_invocable_r<std::size_t, Hash, const T&>
>;

// NOLINTBEGIN(misc-const-correctness)
TEST_CASE("std::hash") {
    opt::option<int> a;
    [[maybe_unused]] const std::size_t hash_a = std::hash<opt::option<int>>{}(a);
    CHECK_UNARY(is_hashable<decltype(a)>);
    CHECK_UNARY(is_hashable<int>);
    CHECK_UNARY(noexcept(std::hash<opt::option<int>>{}(a)));

    opt::option<const float> b;
    [[maybe_unused]] const std::size_t hash_b = std::hash<opt::option<const float>>{}(b);
    CHECK_UNARY(is_hashable<decltype(b)>);
    CHECK_UNARY(is_hashable<float>);
    CHECK_UNARY(noexcept(std::hash<opt::option<const float>>{}(b)));

    struct unhashable_type {
        int x;
    };
    opt::option<unhashable_type> c;
    CHECK_UNARY_FALSE(is_hashable<decltype(c)>);
    CHECK_UNARY_FALSE(is_hashable<unhashable_type>);

    opt::option<const unhashable_type> d;
    CHECK_UNARY_FALSE(is_hashable<decltype(d)>);
}
// NOLINTEND(misc-const-correctness)

TEST_CASE("std::string") {
    opt::option<std::string> a;
    CHECK_EQ(sizeof(a), sizeof(std::string));
    CHECK_UNARY_FALSE(a.has_value());
    a = "";
    CHECK_UNARY(a.has_value());
    CHECK_UNARY(a->empty());
    for (std::size_t i = 1; i <= 100; ++i) {
        CAPTURE(i);
        a->append("a");
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a->size(), i);
    }
    for (std::size_t i = 99;; --i) {
        CAPTURE(i);
        a->pop_back();
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a->size(), i);
        if (i == 0) { break; }
    }
    CHECK_UNARY(a.has_value());
    CHECK_EQ(a->size(), 0);
    a->shrink_to_fit();
    CHECK_UNARY(a.has_value());
    // not 0 because SSO
    CHECK_GT(a->capacity(), 0);
    a->clear();
    a->clear();
    CHECK_UNARY(a.has_value());
}

TEST_CASE("std::variant") {
    std::variant<int, float, unsigned> a{1};
    CHECK_EQ(opt::get<0>(a), 1);
    CHECK_EQ(opt::get<1>(a), opt::none);
    CHECK_EQ(opt::get<2>(a), opt::none);
    a = 1.f;
    CHECK_EQ(opt::get<0>(a), opt::none);
    CHECK_EQ(opt::get<1>(a), 1.f);
    CHECK_EQ(opt::get<2>(a), opt::none);
    a.emplace<unsigned>(2u);
    CHECK_EQ(opt::get<0>(a), opt::none);
    CHECK_EQ(opt::get<1>(a), opt::none);
    CHECK_EQ(opt::get<2>(a), 2u);

    *opt::get<2>(a) = 10u;
    CHECK_EQ(std::get<2>(a), 10u);

    CHECK_UNARY(std::is_same_v<decltype(opt::get<0>(a)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::get<0>(std::as_const(a))), opt::option<const int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::get<0>(as_rvalue(a))), opt::option<int&&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::get<0>(as_const_rvalue(a))), opt::option<const int&&>>);

    CHECK_UNARY(std::is_same_v<decltype(opt::get<1>(a)), opt::option<float&>>);

    CHECK_EQ(opt::get<unsigned>(a), 10u);
    a.emplace<int>(1);
    CHECK_EQ(opt::get<int>(a), 1);
    *opt::get<int>(a) = 2;
    CHECK_EQ(std::get<0>(a), 2);
    CHECK_EQ(opt::get<float>(a), opt::none);
    CHECK_EQ(opt::get<unsigned>(a), opt::none);

    CHECK_UNARY(std::is_same_v<decltype(opt::get<int>(a)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::get<int>(std::as_const(a))), opt::option<const int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::get<int>(as_rvalue(a))), opt::option<int&&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::get<int>(as_const_rvalue(a))), opt::option<const int&&>>);
}

TEST_CASE("opt::io") {
    std::stringstream str;
    opt::option<int> a = 1;
    str << opt::io(a);
    CHECK_EQ(str.str(), "1");
    str << opt::io(std::as_const(a));
    CHECK_EQ(str.str(), "11");
    str << opt::io(a, "a");
    CHECK_EQ(str.str(), "111");
    str << opt::io(std::as_const(a), "a");
    CHECK_EQ(str.str(), "1111");
    a = opt::none;
    str << opt::io(a, "2");
    CHECK_EQ(str.str(), "11112");
    str << opt::io(std::as_const(a), "2");
    CHECK_EQ(str.str(), "111122");

    a.emplace(0);
    str >> *a;
    str.seekg(0);
    CHECK_EQ(a, 111122);

    int b{};
    a.emplace(0);
    str >> opt::io(a, b);
    str.seekg(0);
    CHECK_EQ(a, 111122);
    CHECK_EQ(b, 0);

    a.reset();
    b = 0;
    str >> opt::io(a, b);
    str.seekg(0);
    CHECK_EQ(a, opt::none);
    CHECK_EQ(b, 111122);
}

TEST_CASE("opt::at") {
    std::vector<int> a{{1, 2, 3}};
    CHECK_EQ(a.size(), 3);

    CHECK_EQ(opt::at(a, 0), 1);
    CHECK_EQ(opt::at(a, 1), 2);
    CHECK_EQ(opt::at(a, 2), 3);
    CHECK_EQ(opt::at(a, 3), opt::none);
    CHECK_EQ(opt::at(a, 4), opt::none);

    *opt::at(a, 0) = 10;
    CHECK_EQ(a[0], 10);
    *opt::at(a, 2) = 20;
    CHECK_EQ(a[2], 20);

    CHECK_UNARY(std::is_same_v<decltype(opt::at(a, 1)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at(std::as_const(a), 1)), opt::option<const int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at(as_rvalue(a), 1)), opt::option<int&&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at(as_const_rvalue(a), 1)), opt::option<const int&&>>);

    std::string b = "abcdefg";
    CHECK_EQ(b.size(), 7);

    CHECK_EQ(opt::at(b, 0), 'a');
    CHECK_EQ(opt::at(b, 1), 'b');
    CHECK_EQ(opt::at(b, 2), 'c');
    CHECK_EQ(opt::at(b, 3), 'd');
    CHECK_EQ(opt::at(b, 4), 'e');
    CHECK_EQ(opt::at(b, 5), 'f');
    CHECK_EQ(opt::at(b, 6), 'g');
    CHECK_EQ(opt::at(b, 7), opt::none);

    *opt::at(b, 0) = 'z';
    CHECK_EQ(b[0], 'z');
}

TEST_CASE("option<bool>") {
    const opt::option<bool> a{false};
    const opt::option<int> b{a};
    CHECK_EQ(b, 0);

    // Test for LWG 3836
    struct bool_convertible {
        operator bool() const { return false; }
    };
    const opt::option<bool_convertible> c{std::in_place};
    CHECK_UNARY(c.has_value());
    const opt::option<bool> d{c};
    CHECK_EQ(d, false);
}

TEST_CASE("internal invoke") {
    const opt::option<int> a = opt::option<int>{1}.map([](int x) { return x + 1; });
    CHECK_EQ(a, 2);
    const auto b_fn = [](int x) { return x + 2; };
    const opt::option<int> b = opt::option<int>{10}.map(std::ref(b_fn));
    CHECK_EQ(b, 12);

    struct c_t {
        int value;
    };
    const opt::option<int> c = opt::option<c_t>{20}.map(&c_t::value);
    CHECK_EQ(c, 20);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<c_t>&>().map(&c_t::value)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<const opt::option<c_t>&>().map(&c_t::value)), opt::option<const int&>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<c_t>>().map(&c_t::value)), opt::option<int&&>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<const opt::option<c_t>>().map(&c_t::value)), opt::option<const int&&>>);

    c_t d_v{30};
    const opt::option<int> d = opt::option{std::ref(d_v)}.map(&c_t::value);
    CHECK_EQ(d, 30);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<std::reference_wrapper<c_t>>>().map(&c_t::value)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<std::reference_wrapper<const c_t>>>().map(&c_t::value)), opt::option<const int&>>);

    const opt::option<int> c2 = opt::option<c_t*>{&d_v}.map(&c_t::value);
    CHECK_EQ(c2, 30);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<c_t*>&>().map(&c_t::value)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<const c_t*>&>().map(&c_t::value)), opt::option<const int&>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<c_t*>>().map(&c_t::value)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<const c_t*>>().map(&c_t::value)), opt::option<const int&>>);

#if 0 // boost.pfr errors
    struct e_t : c_t {
        unsigned value2;
    };
    const opt::option<int> e = opt::option<e_t>{{40, 50}}.map(&e_t::value);
#endif

    struct f_t {
        int value;
        float function() const { return float(value) + 8.f; }
    };
    const opt::option<float> f = opt::option<f_t>{30}.map(&f_t::function);
    CHECK_EQ(f, 38.f);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<f_t>&>().map(&f_t::function)), opt::option<float>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<const opt::option<f_t>&>().map(&f_t::function)), opt::option<float>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<f_t>>().map(&f_t::function)), opt::option<float>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<const opt::option<f_t>>().map(&f_t::function)), opt::option<float>>);

    f_t g_v{40};
    const opt::option<float> g = opt::option{std::ref(g_v)}.map(&f_t::function);
    CHECK_EQ(g, 48.f);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<std::reference_wrapper<f_t>>>().map(&f_t::function)), opt::option<float>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<std::reference_wrapper<const f_t>>>().map(&f_t::function)), opt::option<float>>);

    const opt::option<float> f2 = opt::option<f_t*>{&g_v}.map(&f_t::function);
    CHECK_EQ(f2, 48.f);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<f_t*>&>().map(&f_t::function)), opt::option<float>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<const f_t*>&>().map(&f_t::function)), opt::option<float>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<f_t*>>().map(&f_t::function)), opt::option<float>>);
    CHECK_UNARY(std::is_same_v<decltype(std::declval<opt::option<const f_t*>>().map(&f_t::function)), opt::option<float>>);
}

TEST_CASE("value_or") {
#if !OPTION_MSVC // for some reason MSVC internal errors with this code
    struct s1 {
        int x = 1;
        int y = 2;
        bool operator==(const s1& r) const { return x == r.x && y == r.y; }
    };
    opt::option<s1> a;
    CHECK_EQ(a.value_or({}), s1{1, 2});
    CHECK_EQ(a.value_or({3, 4}), s1{3, 4});
    a = {5, 6};
    CHECK_EQ(a.value_or({}), s1{5, 6});
    CHECK_EQ(a.value_or({7, 8}), s1{5, 6});
#endif
}

TEST_SUITE_END();

struct struct1 {
    int a;
    float x;
};

}

template<>
struct opt::option_traits<struct1> {
    static constexpr int max_level = 0;
};

static_assert(sizeof(opt::option<struct1>) > sizeof(struct1));

#endif // OPTION_USE_BUILTIN_TRAITS

