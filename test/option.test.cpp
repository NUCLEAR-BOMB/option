#include <gtest/gtest.h>

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

namespace {

// https://stackoverflow.com/a/67059296
template<class T>
T& unmove(T&& x) { return static_cast<T&>(x); }

template<class T>
std::size_t hash_fn(const T& x) { return std::hash<T>{}(x); }

static_assert(std::is_trivially_destructible_v<opt::option<int>>);

struct trivial_struct {};
static_assert(std::is_trivially_destructible_v<opt::option<trivial_struct>>);

struct nontrivial_struct {
    ~nontrivial_struct() {} // NOLINT
};
static_assert(!std::is_trivially_destructible_v<opt::option<nontrivial_struct>>);

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
    EXPECT_EQ(std::move(std::as_const(a)).get(), 1);
    EXPECT_EQ(std::move(a).get(), 1);
}
TEST_F(option, assigment) {
    opt::option<int> a = 1;
    a = opt::none;
    EXPECT_FALSE(a.has_value());
    a = unmove(opt::option<int>(2));
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, 2);
    a = unmove(opt::option<int>(opt::none));
    EXPECT_FALSE(a.has_value());
    a = opt::option<int>(3);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, 3);
    a = opt::option<int>(opt::none);
    EXPECT_FALSE(a.has_value());
    a = unmove(4);
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
    EXPECT_NO_THROW((void)std::as_const(a).value());
    EXPECT_NO_THROW((void)std::move(std::as_const(a)).value());
    a = opt::none;
    EXPECT_THROW((void)a.value_or_throw(), opt::bad_access);
    EXPECT_THROW((void)a.value(), opt::bad_access);
    EXPECT_THROW((void)std::as_const(a).value(), opt::bad_access);
    EXPECT_THROW((void)std::move(std::as_const(a)).value(), opt::bad_access);
}
TEST_F(option, value_or) {
    opt::option<int> a;
    EXPECT_EQ(a.value_or(2), 2);
    a = 1;
    EXPECT_EQ(a.value_or(3), 1);
    EXPECT_EQ(std::move(a).value_or(4), 1);
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
    b = opt::option_cast<unsigned>(std::move(a));
    EXPECT_EQ(*b, 1u);
}

}
