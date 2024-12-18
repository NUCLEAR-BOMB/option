
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>
#include <opt/option.hpp>

#include "utils.hpp"

struct my_type1 {
    int x;
    bool operator==(const my_type1& other) const { return x == other.x; }
};
template<>
struct opt::sentinel_option_traits<my_type1> {
    static constexpr my_type1 sentinel_value{1};
};

namespace {

TEST_SUITE_BEGIN("opt::sentinel");

TEST_CASE("int") {
    {
        opt::option<opt::sentinel<int>> a;
        CHECK_GT(sizeof(a), sizeof(int));
        CHECK_UNARY_FALSE(a.has_value());
        a = 1;
        CHECK_EQ(a, 1);
        a = opt::none;
        CHECK_UNARY_FALSE(a.has_value());

        a = 2;
        int& b = a.get();
        b = 3;
        CHECK_EQ(b, 3);
        a.get() = 1;
        CHECK_EQ(std::move(a.get()), 1); // NOLINT(performance-move-const-arg)
    }
    {
        opt::option<opt::sentinel<int, 0>> a;
        CHECK_EQ(sizeof(a), sizeof(int));
        CHECK_UNARY_FALSE(a.has_value());
        a = 1;
        CHECK_EQ(a, 1);
        a = opt::none;
        CHECK_UNARY_FALSE(a.has_value());
        a.get_unchecked() = 0;
        CHECK_UNARY_FALSE(a.has_value());
    }
    {
        opt::option<opt::option<opt::sentinel<int, 0>>> a;
        CHECK_GT(sizeof(a), sizeof(int));
        CHECK_UNARY_FALSE(a.has_value());
        a = 2;
        CHECK_UNARY(a.has_value());
        CHECK_UNARY(a->has_value());
        CHECK_EQ(a, 2);
        a = opt::none;
        CHECK_UNARY_FALSE(a.has_value());
        a.get_unchecked().get_unchecked() = 0;
        CHECK_UNARY_FALSE(a.has_value());
    }
    SUBCASE(".map") {
        const auto fn = [](int x) -> opt::option<opt::sentinel<int, -1>> {
            return x + 1;
        };
        CHECK_EQ(opt::option{1}.map(fn), 2);
        CHECK_EQ(opt::option{0}.map(fn), 1);
        CHECK_EQ(opt::option{-1}.map(fn), 0);
    }
}
TEST_CASE("uint") {
    {
        const opt::option<opt::sentinel<unsigned>> a;
        CHECK_GT(sizeof(a), sizeof(unsigned));
    }
    {
        opt::option<opt::sentinel<unsigned, ~0U>> a;
        CHECK_EQ(sizeof(a), sizeof(unsigned));
        CHECK_UNARY_FALSE(a.has_value());
        a = 1U;
        CHECK_EQ(a, 1U);
        a = 0U;
        CHECK_EQ(a, 0U);
        a = opt::none;
        CHECK_UNARY_FALSE(a.has_value());
        a = 2U;
        CHECK_EQ(a, 2U);
        a.get_unchecked() = unsigned(-1);
        CHECK_UNARY_FALSE(a.has_value());
    }
    {
        opt::option<opt::sentinel<unsigned, 1, 2>> a;
        CHECK_EQ(sizeof(a), sizeof(unsigned));
        CHECK_UNARY_FALSE(a.has_value());
        a.emplace(0U);
        CHECK_EQ(a, 0U);
        a = 3U;
        CHECK_EQ(a, 3U);
        a = opt::none;
        CHECK_UNARY_FALSE(a.has_value());
    }
    {
        opt::option<opt::option<opt::sentinel<unsigned, 1, 2>>> a;
        CHECK_EQ(sizeof(a), sizeof(unsigned));
        CHECK_UNARY_FALSE(a.has_value());
        a = 0U;
        CHECK_EQ(a, 0U);
        a = 3U;
        CHECK_EQ(a, 3U);
        a->reset();
        CHECK_UNARY_FALSE(a->has_value());
        *a = 4U;
        CHECK_EQ(a, 4U);
        a.reset();
        CHECK_EQ(a, opt::none);
    }
    {
        opt::option<opt::option<opt::option<opt::sentinel<unsigned, 10, 20>>>> a;
        CHECK_GT(sizeof(a), sizeof(unsigned));
        CHECK_UNARY_FALSE(a.has_value());
        a = 0U;
        CHECK_EQ(a, 0U);
        a = 1U;
        CHECK_EQ(a, 1U);
        a = 2U;
        CHECK_EQ(a, 2U);
        (**a).reset();
        CHECK_UNARY_FALSE((**a).has_value());
        (*a).reset();
        CHECK_UNARY_FALSE((*a).has_value());
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
    }
}
TEST_CASE("pointer") {
    {
        opt::option<opt::sentinel<int*, nullptr>> a;
        CHECK_EQ(sizeof(a), sizeof(int*));
        CHECK_UNARY_FALSE(a.has_value());
        int b{};
        a = &b;
        CHECK_EQ(a, &b);
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
    }
}

TEST_SUITE_END();

TEST_CASE("opt::member") {
    struct my_type {
        float x;

        my_type(float x_) : x{x_ + 1.f} {}
    };
    CHECK_UNARY_FALSE(std::is_aggregate_v<my_type>);
    CHECK_GT(sizeof(opt::option<my_type>), sizeof(my_type));
    opt::option<opt::member<my_type, &my_type::x>> a;
    CHECK_EQ(sizeof(a), sizeof(my_type));
    CHECK_UNARY_FALSE(a.has_value());
    a = my_type{1.f};
    CHECK_EQ(a->m.x, 2.f);
    a.reset();
    CHECK_UNARY_FALSE(a.has_value());
}

TEST_CASE("opt::enforce") {
    CHECK_EQ(sizeof(opt::option<opt::enforce<float>>), sizeof(float));
    CHECK_EQ(sizeof(opt::option<opt::enforce<double>>), sizeof(double));
    const opt::option<opt::enforce<float>> a;
    CHECK_UNARY_FALSE(a.has_value());
}

TEST_CASE("opt::sentinel_f") {
    struct compare { bool operator()(int x, int y) const { return (x + 1) == (y - 1); } };
    struct set { void operator()(int& x, int y) const { x = (y - 2); } };

    CHECK_GT(sizeof(opt::option<opt::sentinel_f<int, compare, set>>), sizeof(int));
    opt::option<opt::sentinel_f<int, compare, set, -1>> a;
    CHECK_EQ(sizeof(a), sizeof(int));
    CHECK_UNARY_FALSE(a.has_value());
    a.emplace(1);
    CHECK_EQ(a, 1);
    a.reset();
    CHECK_EQ(a, opt::none);
    a.reset();
    CHECK_EQ(a, opt::none);
}

TEST_CASE("opt::sentinel_option_traits") {
    opt::option<my_type1> a;
    CHECK_EQ(sizeof(a), sizeof(my_type1));
    CHECK_UNARY_FALSE(a.has_value());
    a = my_type1{2};
    CHECK_EQ(a, my_type1{2});
    a->x = 3;
    CHECK_EQ(a, my_type1{3});
    a.reset();
    CHECK_UNARY_FALSE(a.has_value());
    CHECK_EQ(a.get_unchecked(), my_type1{1});
    a.get_unchecked() = my_type1{0};
    CHECK_EQ(a, my_type1{0});
    a->x = 1;
    CHECK_UNARY_FALSE(a.has_value());
}

}
