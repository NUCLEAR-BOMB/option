#include <gtest/gtest.h>

#include <option.hpp>

namespace opt {
    template<class T>
    void PrintTo(const opt::option<T>& value, ::std::ostream* os) {
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
    ~nontrivial_struct() {}
};
static_assert(!std::is_trivially_destructible_v<opt::option<nontrivial_struct>>);

struct option : ::testing::Test {

};

TEST_F(option, default_ctor) {
    opt::option<int> a;
    EXPECT_FALSE(a.has_value());

    opt::option<int> b(opt::none);
    EXPECT_FALSE(b.has_value());

    opt::option<int> c = opt::none;
    EXPECT_FALSE(c.has_value());
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
    a = opt::none;
    EXPECT_THROW((void)a.value_or_throw(), opt::bad_access);
}
TEST_F(option, value_or) {
    opt::option<int> a;
    EXPECT_EQ(a.value_or(2), 2);
    a = 1;
    EXPECT_EQ(a.value_or(3), 1);
    EXPECT_EQ(std::move(a).value_or(4), 1);
}
TEST_F(option, and_then) {
    const auto convert_to_float = [](int x) -> opt::option<float> {
        if (x >= 0) return opt::option<float>{float(x + 1)};
        return opt::none;
    };
    EXPECT_EQ(opt::option<int>{2}.and_then(convert_to_float), 3.f);
    EXPECT_EQ(opt::option<int>{-10}.and_then(convert_to_float), opt::none);
    EXPECT_EQ(opt::option<int>{opt::none}.and_then(convert_to_float), opt::none);
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


}
