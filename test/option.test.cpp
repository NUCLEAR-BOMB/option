#include <gtest/gtest.h>

#include "utils.hpp"

namespace {

struct trivial_struct {};
static_assert(std::is_trivially_destructible_v<opt::option<trivial_struct>>);

static_assert(is_not_trivial_compatible<opt::option<nontrivial_struct>>);
static_assert(is_trivial_compatible<opt::option<int>>);

struct option : ::testing::Test {

};

TEST_F(option, default_ctor) {
    const opt::option<int> a;
    EXPECT_FALSE(a.has_value());

    const opt::option<int> b(opt::none);
    EXPECT_FALSE(b.has_value());

    const opt::option<int> c = opt::none;
    EXPECT_FALSE(c.has_value());
}
TEST_F(option, constructors) {
    const opt::option<int> a{{}};
    EXPECT_EQ(a, 0);
}
TEST_F(option, get) {
    opt::option<int> a{1};
    EXPECT_EQ(a.get(), 1);
    EXPECT_EQ(std::as_const(a).get(), 1);
    EXPECT_EQ(as_const_rvalue(a).get(), 1);
    EXPECT_EQ(as_rvalue(a).get(), 1);
}
TEST_F(option, assigment) {
    opt::option<int> a = 1;
    a = opt::none;
    EXPECT_FALSE(a.has_value());
    a = as_lvalue(opt::option<int>(2));
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, 2);
    a = as_lvalue(opt::option<int>(opt::none));
    EXPECT_FALSE(a.has_value());
    a = opt::option<int>(3);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, 3);
    a = opt::option<int>(opt::none);
    EXPECT_FALSE(a.has_value());
    a = as_lvalue(4);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, 4);
    a = 5;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, 5);
}

TEST_F(option, reset) {
    opt::option<int> a;
    EXPECT_FALSE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());
    a = 1;
    EXPECT_TRUE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());
}
TEST_F(option, emplace) {
    opt::option<int> a;
    a.emplace(1);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, 1);
}
TEST_F(option, hash) {
    opt::option<int> a{1}, b{1};
    EXPECT_EQ(hash_fn(a), hash_fn(b));
    a = opt::none;
    EXPECT_NE(hash_fn(a), hash_fn(b));
    b = opt::none;
    EXPECT_EQ(hash_fn(a), hash_fn(b));
    
}
TEST_F(option, value_or_throw) {
    opt::option<int> a{1};
    EXPECT_NO_THROW((void)a.value_or_throw());
    EXPECT_NO_THROW((void)a.value());
    EXPECT_NO_THROW((void)as_const(a).value());
    EXPECT_NO_THROW((void)as_const_rvalue(a).value());
    a = opt::none;
    EXPECT_THROW((void)a.value_or_throw(), opt::bad_access);
    EXPECT_THROW((void)a.value(), opt::bad_access);
    EXPECT_THROW((void)as_const(a).value(), opt::bad_access);
    EXPECT_THROW((void)as_const_rvalue(a).value(), opt::bad_access);
}
TEST_F(option, value_or) {
    opt::option<int> a;
    EXPECT_EQ(a.value_or(2), 2);
    a = 1;
    EXPECT_EQ(a.value_or(3), 1);
    EXPECT_EQ(as_rvalue(a).value_or(4), 1);
}
TEST_F(option, and_then) {
    const auto convert_to_uint = [](int x) -> opt::option<unsigned> {
        if (x >= 0) return opt::option<unsigned>{unsigned(x + 1)};
        return opt::none;
    };
    EXPECT_EQ(opt::option<int>{2}.and_then(convert_to_uint), 3u);
    EXPECT_EQ(opt::option<int>{-10}.and_then(convert_to_uint), opt::none);
    EXPECT_EQ(opt::option<int>{opt::none}.and_then(convert_to_uint), opt::none);
}
TEST_F(option, map) {
    const auto func = [](auto x) { return x - 1; };
    EXPECT_EQ(opt::option<int>{1}.map(func), 0);
    EXPECT_EQ(opt::option<int>{}.map(func), opt::none);
    EXPECT_EQ(opt::option<int>{10}.map(func).map(func), 8);
}
TEST_F(option, or_else) {
    const auto func = []() { return 1 << 10; };
    EXPECT_EQ(opt::option<int>{1}.or_else(func), 1);
    EXPECT_EQ(opt::option<int>{}.or_else(func), 1 << 10);
    EXPECT_EQ(opt::option<int>{}.or_else(func).or_else(func), 1 << 10);
}
TEST_F(option, take) {
    opt::option<int> a{1};
    auto b = a.take();
    EXPECT_EQ(a, opt::none);
    EXPECT_EQ(b, 1);
}
TEST_F(option, option_cast) {
    opt::option<int> a{1};
    opt::option<unsigned> b = opt::option_cast<unsigned>(a);
    EXPECT_EQ(*b, 1u);
    b = opt::option_cast<unsigned>(as_rvalue(a));
    EXPECT_EQ(*b, 1u);
}
TEST_F(option, deduction_guides) {
    auto a = opt::option{1}; // NOLINT(misc-const-correctness)
    static_assert(std::is_same_v<decltype(a), opt::option<int>>);
    opt::option b{1u}; // NOLINT(misc-const-correctness)
    static_assert(std::is_same_v<decltype(b), opt::option<unsigned>>);
}
TEST_F(option, value_or_default) {
    opt::option a{1};
    EXPECT_EQ(a.value_or_default(), 1);
    a = opt::none;
    EXPECT_EQ(a.value_or_default(), int{});
}
TEST_F(option, ptr_or_pull) {
    opt::option a{1};

    EXPECT_EQ(*(a.ptr_or_null()), 1);
    a = opt::none;
    EXPECT_EQ(a.ptr_or_null(), nullptr);
    a = 2;
    EXPECT_EQ(*(as_const(a).ptr_or_null()), 2);
}
TEST_F(option, filter) {
    const auto is_even = [](int x) { return x % 2 == 0; };

    opt::option a{1};
    EXPECT_EQ(a.filter(is_even), opt::none);
    a = 2;
    EXPECT_EQ(a.filter(is_even), 2);
    a = opt::none;
    EXPECT_EQ(a.filter(is_even), opt::none);
}
TEST_F(option, flatten) {
    auto a = opt::option{opt::option{1}};
    EXPECT_EQ(**a, 1);
    auto b = a.flatten();
    EXPECT_EQ(*b, 1);

    a = opt::option{opt::option<int>{opt::none}};
    b = a.flatten();
    EXPECT_FALSE(b.has_value());
    a = opt::option<int>{opt::none};
    b = a.flatten();
    EXPECT_FALSE(b.has_value());
    a = opt::none;
    b = a.flatten();
    EXPECT_FALSE(b.has_value());
}
TEST_F(option, map_or) {
    const auto add_one = [](int x) { return x + 1; };
    opt::option a{1};
    EXPECT_EQ(a.map_or(10, add_one), 2);
    a = opt::none;
    EXPECT_EQ(a.map_or(11, add_one), 11);

    const auto add_two = [](int x) { return float(x) + 2.f; };
    a = 2;
    EXPECT_EQ(a.map_or(0.f, add_two), 4.f);
    a = opt::none;
    EXPECT_EQ(a.map_or(5.f, add_two), 5.f);
}
TEST_F(option, map_or_else) {
    const auto default_fn = []() { return 2; };
    const auto do_fn = [](int x) { return x - 1; };

    opt::option a{1};
    EXPECT_EQ(a.map_or_else(default_fn, do_fn), 0);
    a = opt::none;
    EXPECT_EQ(a.map_or_else(default_fn, do_fn), 2);
}
TEST_F(option, take_if) {
    opt::option a{1};
    auto b = a.take_if([](int) { return false; });
    EXPECT_FALSE(b.has_value());
    EXPECT_TRUE(a.has_value());

    b = a.take_if([](int& x) {
        return ++x == 2;
    });
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(*b, 2);
    EXPECT_FALSE(a.has_value());

    auto c = a.take_if([](int) { return false; });
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(a.has_value());

    c = a.take_if([](int) { return true; });
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(a.has_value());
}
TEST_F(option, has_value_and) {
    opt::option a{1};
    EXPECT_TRUE(a.has_value_and([](int x) { return x == 1; }));
    EXPECT_FALSE(a.has_value_and([](int x) { return x == 2; }));
    a.reset();
    EXPECT_FALSE(a.has_value_and([](int x) { return x == 1; }));
    EXPECT_FALSE(a.has_value_and([](int x) { return x == 2; }));
}
TEST_F(option, insert) {
    opt::option a{1};
    auto& x = a.insert(2);
    EXPECT_EQ(&x, a.ptr_or_null());
    EXPECT_EQ(*a, 2);

    auto& y = a.insert(as_lvalue(3));
    EXPECT_EQ(&y, a.ptr_or_null());
    EXPECT_EQ(*a, 3);
}
TEST_F(option, inspect) {
    opt::option a{1};
    int x = 0;
    a.inspect([&](int y) { x += y; });
    EXPECT_EQ(x, 1);
    as_rvalue(a)
        .inspect([&](const int& y) { x += y; })
        .inspect([&](int& y) { return x += y * 2; });
    EXPECT_EQ(x, 4);
}

}
