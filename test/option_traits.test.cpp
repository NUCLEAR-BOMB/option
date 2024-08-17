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

struct enumeration : ::testing::Test {};

TEST_F(enumeration, uint8_t) {
    enum class enum1 : std::uint8_t {};

    using traits1 = opt::option_traits<enum1>;
    EXPECT_EQ(traits1::max_level, 0);

    enum class enum2 : std::uint8_t {
        enumerator1
    };

    using traits2 = opt::option_traits<enum2>;
    EXPECT_EQ(traits2::max_level, 255);

    enum2 a{};
    EXPECT_EQ(a, enum2::enumerator1);
    EXPECT_EQ(traits2::get_level(&a), std::uintmax_t(-1));
    traits2::set_level(&a, 0);
    EXPECT_EQ(traits2::get_level(&a), 0);
    traits2::set_level(&a, 1);
    EXPECT_EQ(traits2::get_level(&a), 1);
    traits2::set_level(&a, 254);
    EXPECT_EQ(traits2::get_level(&a), 254);
    a = enum2::enumerator1;
    EXPECT_EQ(traits2::get_level(&a), std::uintmax_t(-1));

    enum class enum3 : std::uint8_t {
        enumerator1, enumerator2, enumerator3, enumerator4
    };
    using traits3 = opt::option_traits<enum3>;
    EXPECT_EQ(traits3::max_level, 252);

    enum3 b{};
    b = enum3::enumerator1;
    EXPECT_EQ(traits3::get_level(&b), std::uintmax_t(-1));
    b = enum3::enumerator2;
    EXPECT_EQ(traits3::get_level(&b), std::uintmax_t(-1));
    b = enum3::enumerator3;
    EXPECT_EQ(traits3::get_level(&b), std::uintmax_t(-1));
    b = enum3::enumerator4;
    EXPECT_EQ(traits3::get_level(&b), std::uintmax_t(-1));
    traits3::set_level(&b, 0);
    EXPECT_EQ(traits3::get_level(&b), 0);
    traits3::set_level(&b, 1);
    EXPECT_EQ(traits3::get_level(&b), 1);

    enum class enum4 : std::uint8_t {
        a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19,
        a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32, a33, a34, a35, a36, a37,
        a38, a39, a40, a41, a42, a43, a44, a45, a46, a47, a48, a49, a50, a51, a52, a53, a54, a55,
        a56, a57, a58, a59, a60, a61, a62, a63, a64, a65, a66, a67, a68, a69, a70, a71, a72, a73,
        a74, a75, a76, a77, a78, a79, a80, a81, a82, a83, a84, a85, a86, a87, a88, a89, a90, a91,
        a92, a93, a94, a95, a96, a97, a98, a99, a100, a101, a102, a103, a104, a105, a106, a107,
        a108, a109, a110, a111, a112, a113, a114, a115, a116, a117, a118, a119, a120, a121, a122,
        a123, a124, a125, a126, a127, a128, a129, a130, a131, a132, a133, a134, a135, a136, a137,
        a138, a139, a140, a141, a142, a143, a144, a145, a146, a147, a148, a149, a150, a151, a152,
        a153, a154, a155, a156, a157, a158, a159, a160, a161, a162, a163, a164, a165, a166, a167,
        a168, a169, a170, a171, a172, a173, a174, a175, a176, a177, a178, a179, a180, a181, a182,
        a183, a184, a185, a186, a187, a188, a189, a190, a191, a192, a193, a194, a195, a196, a197,
        a198, a199, a200, a201, a202, a203, a204, a205, a206, a207, a208, a209, a210, a211, a212,
        a213, a214, a215, a216, a217, a218, a219, a220, a221, a222, a223, a224, a225, a226, a227,
        a228, a229, a230, a231, a232, a233, a234, a235, a236, a237, a238, a239, a240, a241, a242,
        a243, a244, a245, a246, a247, a248, a249, a250, a251, a252, a253, a254, a255
    };
    using traits4 = opt::option_traits<enum4>;
    EXPECT_EQ(traits4::max_level, 0);

    enum class enum5 : std::uint8_t {
        a255 = 255
    };
    using traits5 = opt::option_traits<enum5>;
    EXPECT_EQ(traits5::max_level, 0);

    enum class enum6 : std::uint8_t {
        a254 = 254
    };
    using traits6 = opt::option_traits<enum6>;
    EXPECT_EQ(traits6::max_level, 1);

    enum6 c = enum6::a254;
    EXPECT_EQ(traits6::get_level(&c), std::uintmax_t(-1));
    traits6::set_level(&c, 0);
    EXPECT_EQ(traits6::get_level(&c), 0);
}

}

