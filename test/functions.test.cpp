
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <opt/option.hpp>
#include <doctest/doctest.h>
#include <sstream>
#include <vector>
#include <string>
#include <type_traits>
#include <map>
#include <set>
#include <variant>

#include "utils.hpp"

struct type_with_traits {
    int x;

    bool operator==(const type_with_traits& other) const { return x == other.x; }
};
template<>
struct opt::option_traits<type_with_traits> {
    static constexpr std::uintmax_t max_level = 1;
    static constexpr std::uintmax_t get_level(const type_with_traits* const value) noexcept {
        return std::uintmax_t(value->x - 123);
    }
    static constexpr void set_level(type_with_traits* const value, const std::uintmax_t) noexcept {
        value->x = 123;
    }
};

namespace {

TEST_SUITE_BEGIN("functions");

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

    struct ct {
        int operator[](std::size_t) const { return 1; }
        // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
        std::size_t size() const { return 1; }
    };
    ct c;
    CHECK_EQ(c[0], 1);

    CHECK_EQ(opt::at(c, 0), 1);
    CHECK_UNARY(std::is_same_v<decltype(opt::at(c, 0)), opt::option<int>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at(as_const(c), 0)), opt::option<int>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at(as_rvalue(c), 0)), opt::option<int>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at(as_const_rvalue(c), 0)), opt::option<int>>);
}

TEST_CASE("opt::at_front") {
    std::vector<int> a{{1, 2, 3}};
    CHECK_EQ(a.size(), 3);

    CHECK_EQ(opt::at_front(a), 1);
    *opt::at_front(a) = 2;
    CHECK_EQ(a[0], 2);
    CHECK_EQ(a.front(), 2);
    CHECK_EQ(opt::at_front(a), 2);

    a.clear();
    CHECK_EQ(a.size(), 0);
    CHECK_EQ(opt::at_front(a), opt::none);
    CHECK_NE(opt::at_front(a), 2);

    CHECK_UNARY(std::is_same_v<decltype(opt::at_front(a)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at_front(std::as_const(a))), opt::option<const int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at_front(as_rvalue(a))), opt::option<int&&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at_front(as_const_rvalue(a))), opt::option<const int&&>>);

    std::string b = "123";
    CHECK_EQ(b.size(), 3);

    CHECK_EQ(opt::at_front(b), '1');
    *opt::at_front(b) = '2';
    CHECK_EQ(b[0], '2');
    CHECK_EQ(b.front(), '2');
    CHECK_EQ(opt::at_front(b), '2');

    b.clear();
    CHECK_EQ(b.size(), 0);
    CHECK_EQ(opt::at_front(b), opt::none);
}

TEST_CASE("opt::at_back") {
    std::vector<int> a{{4, 5, 6}};
    CHECK_EQ(a.size(), 3);

    CHECK_EQ(opt::at_back(a), 6);
    *opt::at_back(a) = 7;
    CHECK_EQ(a[2], 7);
    CHECK_EQ(a.back(), 7);
    CHECK_EQ(opt::at_back(a), 7);

    a.clear();
    CHECK_EQ(opt::at_back(a), opt::none);

    CHECK_UNARY(std::is_same_v<decltype(opt::at_back(a)), opt::option<int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at_back(std::as_const(a))), opt::option<const int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at_back(as_rvalue(a))), opt::option<int&&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::at_back(as_const_rvalue(a))), opt::option<const int&&>>);
}

TEST_CASE("opt::lookup") {
    std::map<int, std::string> a{{{1, "a"}, {2, "ab"}, {3, "abc"}, {4, "abcd"}}};
    CHECK_EQ(a.size(), 4);

    CHECK_EQ(opt::lookup(a, 1), "a");
    CHECK_EQ(opt::lookup(a, 2), "ab");
    CHECK_EQ(opt::lookup(a, 3), "abc");
    CHECK_EQ(opt::lookup(a, 4), "abcd");
    *opt::lookup(a, 1) = "a1";
    CHECK_EQ(a.find(1)->second, "a1");
    CHECK_EQ(opt::lookup(a, 1), "a1");

    CHECK_EQ(a.find(5), a.end());
    CHECK_EQ(opt::lookup(a, 5), opt::none);
    CHECK_EQ(a.find(-1), a.end());
    CHECK_EQ(opt::lookup(a, -1), opt::none);

    CHECK_UNARY(std::is_same_v<decltype(opt::lookup(a, 0)), opt::option<std::string&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::lookup(as_const(a), 0)), opt::option<const std::string&>>);

    std::set<int> b{{10, 11, 12}};
    CHECK_EQ(b.size(), 3);

    CHECK_EQ(opt::lookup(b, 10), 10);
    CHECK_EQ(opt::lookup(b, 11), 11);
    CHECK_EQ(opt::lookup(b, 12), 12);

    CHECK_UNARY(std::is_same_v<decltype(opt::lookup(b, 0)), opt::option<const int&>>);
    CHECK_UNARY(std::is_same_v<decltype(opt::lookup(as_const(b), 0)), opt::option<const int&>>);
}

TEST_CASE("opt::as_option") {
    CHECK_EQ(opt::as_option(type_with_traits{1}), type_with_traits{1});
    CHECK_EQ(opt::as_option(type_with_traits{122}), type_with_traits{122});
    CHECK_EQ(opt::as_option(type_with_traits{134}), type_with_traits{134});
    CHECK_EQ(opt::as_option(type_with_traits{123}), opt::none);

    CHECK_EQ(opt::as_option(std::array{type_with_traits{1}, type_with_traits{123}}), std::array{type_with_traits{1}, type_with_traits{123}});
    CHECK_EQ(opt::as_option(std::array{type_with_traits{123}, type_with_traits{1}}), opt::none);
}

TEST_CASE("opt::get") {
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

TEST_SUITE_END();

}
