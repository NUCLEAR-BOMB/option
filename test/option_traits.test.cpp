#include <gtest/gtest.h>
#include <opt/option.hpp>
#include <cstdint>
#include <cfenv>
#include <string_view>
#include <cmath>
#include <array>
#include <functional>
#include <memory>

#include "utils.hpp" // NOLINT(misc-include-cleaner)

#if OPTION_MSVC
    #pragma fenv_access(on)
#elif OPTION_CLANG
    #if __clang_major__ >= 12
        #pragma STDC FENV_ACCESS ON
    #endif
#endif

struct empty1 {};
struct empty2 {};

template<>
struct opt::option_traits<empty2>;

namespace {

struct option_traits : ::testing::Test {
    void SetUp() override {
        ASSERT_EQ(std::feclearexcept(FE_ALL_EXCEPT), 0);
    }
    void TearDown() override {
        EXPECT_EQ(std::fetestexcept(FE_DIVBYZERO), 0);
        EXPECT_EQ(std::fetestexcept(FE_INEXACT), 0);
        EXPECT_EQ(std::fetestexcept(FE_INVALID), 0);
        EXPECT_EQ(std::fetestexcept(FE_OVERFLOW), 0);
        EXPECT_EQ(std::fetestexcept(FE_UNDERFLOW), 0);

        ASSERT_EQ(std::feclearexcept(FE_ALL_EXCEPT), 0);
    }
};

TEST_F(option_traits, bool) {
    using traits = opt::option_traits<bool>;
    EXPECT_EQ(traits::max_level, 254);

    bool a = true;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    a = false;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));

    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 2);
    EXPECT_EQ(traits::get_level(&a), 2);
    traits::set_level(&a, 253);
    EXPECT_EQ(traits::get_level(&a), 253);
}
TEST_F(option_traits, reference_wrapper) {
    using traits = opt::option_traits<std::reference_wrapper<int>>;
    EXPECT_EQ(traits::max_level, 256);

    int a = 1;
    std::reference_wrapper<int> b = a;
    EXPECT_EQ(traits::get_level(&b), std::uintmax_t(-1));
    traits::set_level(&b, 0);
    EXPECT_EQ(traits::get_level(&b), 0);
    traits::set_level(&b, 1);
    EXPECT_EQ(traits::get_level(&b), 1);
    traits::set_level(&b, 255);
    EXPECT_EQ(traits::get_level(&b), 255);
    b = std::reference_wrapper<int>(a);
    EXPECT_EQ(traits::get_level(&b), std::uintmax_t(-1));
}
TEST_F(option_traits, reference) {
    using traits = opt::option_traits<int&>;
    EXPECT_EQ(traits::max_level, 255);

    int a = 2;
    int* b = &a;
    EXPECT_EQ(traits::get_level(&b), std::uintmax_t(-1));
    traits::set_level(&b, 0);
    EXPECT_EQ(traits::get_level(&b), 0);
    traits::set_level(&b, 1);
    EXPECT_EQ(traits::get_level(&b), 1);
    traits::set_level(&b, 254);
    EXPECT_EQ(traits::get_level(&b), 254);
    b = &a;
    EXPECT_EQ(traits::get_level(&b), std::uintmax_t(-1));
}
TEST_F(option_traits, pointer) {
    using traits = opt::option_traits<int*>;
    constexpr std::uintmax_t max_level = sizeof(int*) == 8 ? 512 : 256;
    EXPECT_EQ(traits::max_level, max_level);

    int a = 3;
    int* b = &a;
    EXPECT_EQ(traits::get_level(&b), std::uintmax_t(-1));
    traits::set_level(&b, 0);
    EXPECT_EQ(traits::get_level(&b), 0);
    traits::set_level(&b, 1);
    EXPECT_EQ(traits::get_level(&b), 1);
    traits::set_level(&b, max_level - 1);
    EXPECT_EQ(traits::get_level(&b), max_level - 1);
    b = &a;
    EXPECT_EQ(traits::get_level(&b), std::uintmax_t(-1));
}
TEST_F(option_traits, double_sNaN) {
    using traits = opt::impl::internal_option_traits<double, opt::impl::option_strategy::float64_sNaN>;
    EXPECT_EQ(traits::max_level, 256);

    double a = 123.456;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    EXPECT_TRUE(std::isnormal(a));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 255);
    EXPECT_EQ(traits::get_level(&a), 255);
    a = 1011.1213;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    EXPECT_TRUE(std::isnormal(a));
}
TEST_F(option_traits, float_sNaN) {
    using traits = opt::impl::internal_option_traits<float, opt::impl::option_strategy::float32_sNaN>;
    EXPECT_EQ(traits::max_level, 256);

    float a = 10.11f;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    EXPECT_TRUE(std::isnormal(a));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 255);
    EXPECT_EQ(traits::get_level(&a), 255);
    a = 1.2f;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    EXPECT_TRUE(std::isnormal(a));
}
TEST_F(option_traits, double_qNaN) {
    using traits = opt::impl::internal_option_traits<double, opt::impl::option_strategy::float64_qNaN>;
    EXPECT_EQ(traits::max_level, 256);

    double a = 123.456;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    EXPECT_TRUE(std::isnormal(a));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    EXPECT_TRUE(std::isnan(a));
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    EXPECT_TRUE(std::isnan(a));
    traits::set_level(&a, 255);
    EXPECT_EQ(traits::get_level(&a), 255);
    EXPECT_TRUE(std::isnan(a));
    a = 1011.1213;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    EXPECT_TRUE(std::isnormal(a));
}
TEST_F(option_traits, float_qNaN) {
    using traits = opt::impl::internal_option_traits<float, opt::impl::option_strategy::float32_qNaN>;
    EXPECT_EQ(traits::max_level, 256);

    float a = 10.11f;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    EXPECT_TRUE(std::isnormal(a));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    EXPECT_TRUE(std::isnan(a));
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    EXPECT_TRUE(std::isnan(a));
    traits::set_level(&a, 255);
    EXPECT_EQ(traits::get_level(&a), 255);
    EXPECT_TRUE(std::isnan(a));
    a = 1.2f;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    EXPECT_TRUE(std::isnormal(a));
}
TEST_F(option_traits, array_0) {
    using traits = opt::option_traits<std::array<int, 0>>;
    EXPECT_EQ(traits::max_level, 255);

    std::array<int, 0> a{};
    traits::after_constructor(&a);
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 254);
    EXPECT_EQ(traits::get_level(&a), 254);
    traits::after_assignment(&a);
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
}
TEST_F(option_traits, empty) {
    EXPECT_EQ(sizeof(opt::option<empty1>), sizeof(empty1));
    EXPECT_GT(sizeof(opt::option<empty2>), sizeof(empty2));

    struct empty {};
    using traits = opt::option_traits<empty>;
    EXPECT_EQ(traits::max_level, 255);

    empty a{};
    traits::after_constructor(&a);
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 254);
    EXPECT_EQ(traits::get_level(&a), 254);
    traits::after_assignment(&a);
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
}
TEST_F(option_traits, polymorphic) {
    struct polymorphic {
        int x{};

        polymorphic() = default;
        polymorphic(const polymorphic&) = default;
        polymorphic& operator=(const polymorphic&) = default;
        virtual ~polymorphic() = default;
    };
    using traits = opt::option_traits<polymorphic>;
    EXPECT_EQ(traits::max_level, 255);

    polymorphic a{};
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 254);
    EXPECT_EQ(traits::get_level(&a), 254);
}
TEST_F(option_traits, string_view) {
    using traits = opt::option_traits<std::string_view>;
    EXPECT_EQ(traits::max_level, 255);

    std::string_view a = "123abc";
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    EXPECT_EQ(a.size(), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    EXPECT_EQ(a.size(), 1);
    traits::set_level(&a, 2);
    EXPECT_EQ(traits::get_level(&a), 2);
    EXPECT_EQ(a.size(), 2);
    traits::set_level(&a, 254);
    EXPECT_EQ(traits::get_level(&a), 254);
    EXPECT_EQ(a.size(), 254);
    a = "abc123";
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    a = "123abc";
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
}
TEST_F(option_traits, unique_ptr) {
    using traits = opt::option_traits<std::unique_ptr<int>>;
    EXPECT_EQ(traits::max_level, 255);

    std::unique_ptr<int> a = std::make_unique<int>(2);
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    a.reset();
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 2);
    EXPECT_EQ(traits::get_level(&a), 2);
    traits::set_level(&a, 254);
    EXPECT_EQ(traits::get_level(&a), 254);
    new (&a) std::unique_ptr<int>(std::make_unique<int>(3));
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
}
TEST_F(option_traits, pointer_to_member) {
    struct s1 { int x; };
    using traits = opt::option_traits<int s1::*>;
    EXPECT_EQ(traits::max_level, 255);

    int s1::* a = &s1::x;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    EXPECT_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    EXPECT_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 254);
    EXPECT_EQ(traits::get_level(&a), 254);
    a = &s1::x;
    EXPECT_EQ(traits::get_level(&a), std::uintmax_t(-1));
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

struct optional_members {
    std::uint32_t x;
    std::uint32_t y;
    bool operator==(const optional_members& o) const { return x == o.x && y == o.y; }
};

template<>
struct opt::option_traits<optional_members> {
    static constexpr std::uintmax_t max_level = 1;

    static std::uintmax_t get_level(const optional_members* const value) {
        return value->x != std::uint32_t(-1) ? value->x : std::uintmax_t(-1);
    }
    static void set_level(optional_members* const value, const std::uintmax_t level) {
        value->x = std::uint32_t(level);
    }
    static void after_constructor(optional_members* const value) {
        value->x = std::uint32_t(-1);
    }
    static void after_assignment(optional_members* const value) {
        value->x = std::uint32_t(-1);
    }
};

namespace {

TEST_F(option_traits, optional_members) {
    EXPECT_EQ(sizeof(opt::option<optional_members>), sizeof(optional_members));
    EXPECT_GT(sizeof(opt::option<opt::option<optional_members>>), sizeof(optional_members));
    EXPECT_GT(sizeof(opt::option<opt::option<opt::option<optional_members>>>), sizeof(optional_members));

    opt::option<optional_members> a;
    EXPECT_FALSE(a.has_value());

    a = optional_members{1, 2};
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a->x, std::uint32_t(-1));
    EXPECT_EQ(a->y, 2);

    a.reset();
    EXPECT_FALSE(a.has_value());

    a.emplace(optional_members{100, 2});
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a->x, std::uint32_t(-1));
    EXPECT_EQ(a->y, 2);

    a->x = 0;
    EXPECT_FALSE(a.has_value());
    a.get_unchecked().x = std::uint32_t(-1);
    EXPECT_TRUE(a.has_value());
    a.get_unchecked().x = 0;
    EXPECT_FALSE(a.has_value());
}

}

