#include <gtest/gtest.h>
#include <option.hpp>
#include <cstdint>

#include "utils.hpp" // NOLINT(misc-include-cleaner)

struct empty1 {};
struct empty2 {};

template<>
struct opt::option_traits<empty2>;

namespace {

struct option_traits : ::testing::Test {};

TEST_F(option_traits, empty) {
    EXPECT_EQ(sizeof(opt::option<empty1>), sizeof(empty1));
    EXPECT_GT(sizeof(opt::option<empty2>), sizeof(empty2));
}

}

struct some_struct_level1 {
    int x;
    bool operator==(const some_struct_level1& o) const { return x == o.x; }
};

template<>
struct opt::option_traits<some_struct_level1> {
    static constexpr std::uintmax_t max_level = 1;

    static constexpr std::uintmax_t get_level(const some_struct_level1* const value) {
        return value->x == -10 ? 0 : std::uintmax_t(-1);
    }
    static constexpr void set_level(some_struct_level1* const value, [[maybe_unused]] const std::uintmax_t level) {
        value->x = -10;
    }
};

namespace {

TEST_F(option_traits, level_1) {
    EXPECT_EQ(sizeof(opt::option<some_struct_level1>), sizeof(some_struct_level1));
    EXPECT_GT(sizeof(opt::option<opt::option<some_struct_level1>>), sizeof(some_struct_level1));
    EXPECT_GT(sizeof(opt::option<opt::option<opt::option<some_struct_level1>>>), sizeof(some_struct_level1));
    EXPECT_GT(sizeof(opt::option<opt::option<opt::option<opt::option<some_struct_level1>>>>), sizeof(some_struct_level1));

    opt::option<some_struct_level1> a;
    EXPECT_FALSE(a.has_value());

    a = 5;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, some_struct_level1{5});

    a.get_unchecked() = some_struct_level1{-10};
    EXPECT_FALSE(a.has_value());

    opt::option<opt::option<some_struct_level1>> b = -1;
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, some_struct_level1{-1});
    b.reset();
    EXPECT_FALSE(b.has_value());
    b.emplace(-5);
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, some_struct_level1{-5});
}

}

struct some_struct_level2 {
    int x;
    int y;
    bool operator==(const some_struct_level2& o) const { return x == o.x && y == o.y; }
};

template<>
struct opt::option_traits<some_struct_level2> {
    static constexpr std::uintmax_t max_level = 2;

    static std::uintmax_t get_level(const some_struct_level2* const value) {
        if (value->x == -1) { return 0; }
        if (value->y == -1) { return 1; }
        return std::uintmax_t(-1);
    }
    static void set_level(some_struct_level2* const value, const std::uintmax_t level) {
        if (level == 0) { value->x = -1; }
        if (level == 1) { value->y = -1; }
    }
};

namespace {

TEST_F(option_traits, level_2) {
    EXPECT_EQ(sizeof(opt::option<some_struct_level2>), sizeof(some_struct_level2));
    EXPECT_EQ(sizeof(opt::option<opt::option<some_struct_level2>>), sizeof(some_struct_level2));
    EXPECT_GT(sizeof(opt::option<opt::option<opt::option<some_struct_level2>>>), sizeof(some_struct_level2));
    EXPECT_GT(sizeof(opt::option<opt::option<opt::option<opt::option<some_struct_level2>>>>), sizeof(some_struct_level2));

    opt::option<some_struct_level2> a;
    EXPECT_FALSE(a.has_value());

    a.emplace(some_struct_level2{1, 2});
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, (some_struct_level2{1, 2}));

    a = opt::none;
    EXPECT_FALSE(a.has_value());

    a = some_struct_level2{9, 10};
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, (some_struct_level2{9, 10}));

    a.get_unchecked() = some_struct_level2{-1, 1};
    EXPECT_FALSE(a.has_value());

    a.get_unchecked() = some_struct_level2{0, -1};
    EXPECT_FALSE(a.has_value());

    a.get_unchecked() = some_struct_level2{0, 0};
    EXPECT_TRUE(a.has_value());

    opt::option<opt::option<some_struct_level2>> b{5, 15};
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, (some_struct_level2{5, 15}));

    b.reset();
    EXPECT_FALSE(b.has_value());

    b.get_unchecked().get_unchecked() = some_struct_level2{-1, 0};
    EXPECT_FALSE(b->has_value());

    b.get_unchecked().get_unchecked() = some_struct_level2{0, -1};
    EXPECT_FALSE(b.has_value());

    b.get_unchecked().get_unchecked() = some_struct_level2{-1, -1};
    EXPECT_FALSE(b->has_value());
}

}

