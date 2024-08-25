
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>
#include <opt/option.hpp>

#include "utils.hpp"

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

TEST_SUITE_END();

}
