#include <doctest/doctest.h>
#include <opt/option.hpp>
#include <cstdint>
#include <string_view>
#include <cmath>
#include <functional>
#include <memory>
#include <cstddef>

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
struct opt::option_traits<empty2> {
    static constexpr std::uintmax_t max_level = 0;
};

namespace {

TEST_SUITE_BEGIN("opt::option_traits");

TEST_CASE("bool") {
    using traits = opt::option_traits<bool>;
    CHECK_EQ(traits::max_level, 254);

    bool a = true;
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
    a = false;
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));

    traits::set_level(&a, 0);
    CHECK_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    CHECK_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 2);
    CHECK_EQ(traits::get_level(&a), 2);
    traits::set_level(&a, 253);
    CHECK_EQ(traits::get_level(&a), 253);
}
TEST_CASE("std::reference_wrapper") {
    using traits = opt::option_traits<std::reference_wrapper<int>>;
    CHECK_EQ(traits::max_level, 256);

    int a = 1;
    std::reference_wrapper<int> b = a;
    CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
    traits::set_level(&b, 0);
    CHECK_EQ(traits::get_level(&b), 0);
    traits::set_level(&b, 1);
    CHECK_EQ(traits::get_level(&b), 1);
    traits::set_level(&b, 255);
    CHECK_EQ(traits::get_level(&b), 255);
    b = std::reference_wrapper<int>(a);
    CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
}
TEST_CASE("T&") {
    using traits = opt::option_traits<int&>;
    CHECK_EQ(traits::max_level, 255);

    int a = 2;
    int* b = &a;
    CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
    traits::set_level(&b, 0);
    CHECK_EQ(traits::get_level(&b), 0);
    traits::set_level(&b, 1);
    CHECK_EQ(traits::get_level(&b), 1);
    traits::set_level(&b, 254);
    CHECK_EQ(traits::get_level(&b), 254);
    b = &a;
    CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
}
TEST_CASE("T*") {
    using traits = opt::option_traits<int*>;
    constexpr std::uintmax_t max_level = sizeof(int*) == 8 ? 512 : 256;
    CHECK_EQ(traits::max_level, max_level);

    int a = 3;
    int* b = &a;
    CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
    traits::set_level(&b, 0);
    CHECK_EQ(traits::get_level(&b), 0);
    traits::set_level(&b, 1);
    CHECK_EQ(traits::get_level(&b), 1);
    traits::set_level(&b, max_level - 1);
    CHECK_EQ(traits::get_level(&b), max_level - 1);
    b = &a;
    CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
}

TEST_CASE_FIXTURE(fp_exception_checker, "double") {
    SUBCASE("sNaN") {
        using traits = opt::impl::internal_option_traits<double, opt::impl::option_strategy::float64_sNaN>;
        CHECK_EQ(traits::max_level, 256);

        double a = 123.456;
        CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        CHECK_UNARY(std::isnormal(a));
        traits::set_level(&a, 0);
        CHECK_EQ(traits::get_level(&a), 0);
        traits::set_level(&a, 1);
        CHECK_EQ(traits::get_level(&a), 1);
        traits::set_level(&a, 255);
        CHECK_EQ(traits::get_level(&a), 255);
        a = 1011.1213;
        CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        CHECK_UNARY(std::isnormal(a));
    }
    SUBCASE("qNaN") {
        using traits = opt::impl::internal_option_traits<double, opt::impl::option_strategy::float64_qNaN>;
        CHECK_EQ(traits::max_level, 256);

        double a = 123.456;
        CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        CHECK_UNARY(std::isnormal(a));
        traits::set_level(&a, 0);
        CHECK_EQ(traits::get_level(&a), 0);
        CHECK_UNARY(std::isnan(a));
        traits::set_level(&a, 1);
        CHECK_EQ(traits::get_level(&a), 1);
        CHECK_UNARY(std::isnan(a));
        traits::set_level(&a, 255);
        CHECK_EQ(traits::get_level(&a), 255);
        CHECK_UNARY(std::isnan(a));
        a = 1011.1213;
        CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        CHECK_UNARY(std::isnormal(a));
    }
}
TEST_CASE_FIXTURE(fp_exception_checker, "float") {
    SUBCASE("sNaN") {
        using traits = opt::impl::internal_option_traits<float, opt::impl::option_strategy::float32_sNaN>;
        CHECK_EQ(traits::max_level, 256);

        float a = 10.11f;
        CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        CHECK_UNARY(std::isnormal(a));
        traits::set_level(&a, 0);
        CHECK_EQ(traits::get_level(&a), 0);
        traits::set_level(&a, 1);
        CHECK_EQ(traits::get_level(&a), 1);
        traits::set_level(&a, 255);
        CHECK_EQ(traits::get_level(&a), 255);
        a = 1.2f;
        CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        CHECK_UNARY(std::isnormal(a));
    }
    SUBCASE("qNaN") {
        using traits = opt::impl::internal_option_traits<float, opt::impl::option_strategy::float32_qNaN>;
        CHECK_EQ(traits::max_level, 256);

        float a = 10.11f;
        CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        CHECK_UNARY(std::isnormal(a));
        traits::set_level(&a, 0);
        CHECK_EQ(traits::get_level(&a), 0);
        CHECK_UNARY(std::isnan(a));
        traits::set_level(&a, 1);
        CHECK_EQ(traits::get_level(&a), 1);
        CHECK_UNARY(std::isnan(a));
        traits::set_level(&a, 255);
        CHECK_EQ(traits::get_level(&a), 255);
        CHECK_UNARY(std::isnan(a));
        a = 1.2f;
        CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        CHECK_UNARY(std::isnormal(a));
    }
}

// TEST_CASE("std::array<T, 0>") {
//     using traits = opt::option_traits<std::array<int, 0>>;
//     CHECK_EQ(traits::max_level, 255);
// 
//     std::array<int, 0> a{};
//     traits::after_constructor(&a);
//     CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
//     traits::set_level(&a, 0);
//     CHECK_EQ(traits::get_level(&a), 0);
//     traits::set_level(&a, 1);
//     CHECK_EQ(traits::get_level(&a), 1);
//     traits::set_level(&a, 254);
//     CHECK_EQ(traits::get_level(&a), 254);
//     traits::after_assignment(&a);
//     CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
// }
TEST_CASE("empty type") {
    CHECK_EQ(sizeof(opt::option<empty1>), sizeof(empty1));
    CHECK_GT(sizeof(opt::option<empty2>), sizeof(empty2));

    struct empty {};
    using traits = opt::option_traits<empty>;
    CHECK_EQ(traits::max_level, 255);

    empty a{};
    traits::after_constructor(&a);
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    CHECK_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    CHECK_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 254);
    CHECK_EQ(traits::get_level(&a), 254);
    traits::after_assignment(&a);
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
}
TEST_CASE("polymorphic") {
    struct polymorphic {
        int x{};

        polymorphic() = default;
        polymorphic(const polymorphic&) = default;
        polymorphic& operator=(const polymorphic&) = default;
        virtual ~polymorphic() = default;
    };
    using traits = opt::option_traits<polymorphic>;
    CHECK_EQ(traits::max_level, 255);

    polymorphic a{};
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    CHECK_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    CHECK_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 254);
    CHECK_EQ(traits::get_level(&a), 254);
}

TEST_CASE("std::string_view") {
    using traits = opt::option_traits<std::string_view>;
    CHECK_EQ(traits::max_level, 255);

    std::string_view a = "123abc";
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    CHECK_EQ(traits::get_level(&a), 0);
    CHECK_EQ(a.size(), 0);
    traits::set_level(&a, 1);
    CHECK_EQ(traits::get_level(&a), 1);
    CHECK_EQ(a.size(), 1);
    traits::set_level(&a, 2);
    CHECK_EQ(traits::get_level(&a), 2);
    CHECK_EQ(a.size(), 2);
    traits::set_level(&a, 254);
    CHECK_EQ(traits::get_level(&a), 254);
    CHECK_EQ(a.size(), 254);
    a = "abc123";
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
    a = "123abc";
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
}

TEST_CASE("std::unique_ptr") {
    using traits = opt::option_traits<std::unique_ptr<int>>;
    CHECK_EQ(traits::max_level, 255);

    std::unique_ptr<int> a = std::make_unique<int>(2);
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
    a.reset();
    traits::set_level(&a, 0);
    CHECK_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    CHECK_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 2);
    CHECK_EQ(traits::get_level(&a), 2);
    traits::set_level(&a, 254);
    CHECK_EQ(traits::get_level(&a), 254);
    new (&a) std::unique_ptr<int>(std::make_unique<int>(3));
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
}

TEST_CASE("pointer to member") {
    struct s1 { int x; };
    using traits = opt::option_traits<int s1::*>;
    CHECK_EQ(traits::max_level, 255);

    int s1::* a = &s1::x;
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
    traits::set_level(&a, 0);
    CHECK_EQ(traits::get_level(&a), 0);
    traits::set_level(&a, 1);
    CHECK_EQ(traits::get_level(&a), 1);
    traits::set_level(&a, 254);
    CHECK_EQ(traits::get_level(&a), 254);
    a = &s1::x;
    CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
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

TEST_CASE("emptiness level 1") {
    CHECK_EQ(sizeof(opt::option<some_struct_level1>), sizeof(some_struct_level1));
    CHECK_GT(sizeof(opt::option<opt::option<some_struct_level1>>), sizeof(some_struct_level1));
    CHECK_GT(sizeof(opt::option<opt::option<opt::option<some_struct_level1>>>), sizeof(some_struct_level1));
    CHECK_GT(sizeof(opt::option<opt::option<opt::option<opt::option<some_struct_level1>>>>), sizeof(some_struct_level1));

    opt::option<some_struct_level1> a;
    CHECK_UNARY_FALSE(a.has_value());

    a = 5;
    CHECK_UNARY(a.has_value());
    CHECK_EQ(a, some_struct_level1{5});

    a.get_unchecked() = some_struct_level1{-10};
    CHECK_UNARY_FALSE(a.has_value());

    opt::option<opt::option<some_struct_level1>> b = -1;
    CHECK_UNARY(b.has_value());
    CHECK_EQ(b, some_struct_level1{-1});
    b.reset();
    CHECK_UNARY_FALSE(b.has_value());
    b.emplace(-5);
    CHECK_UNARY(b.has_value());
    CHECK_EQ(b, some_struct_level1{-5});
}

}

struct some_struct_level2 {
    int x;
    int y;
    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
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

TEST_CASE("emptiness level 2") {
    CHECK_EQ(sizeof(opt::option<some_struct_level2>), sizeof(some_struct_level2));
    CHECK_EQ(sizeof(opt::option<opt::option<some_struct_level2>>), sizeof(some_struct_level2));
    CHECK_GT(sizeof(opt::option<opt::option<opt::option<some_struct_level2>>>), sizeof(some_struct_level2));
    CHECK_GT(sizeof(opt::option<opt::option<opt::option<opt::option<some_struct_level2>>>>), sizeof(some_struct_level2));

    opt::option<some_struct_level2> a;
    CHECK_UNARY_FALSE(a.has_value());

    a.emplace(1, 2);
    CHECK_UNARY(a.has_value());
    CHECK_EQ(a, (some_struct_level2{1, 2}));

    a = opt::none;
    CHECK_UNARY_FALSE(a.has_value());

    a = some_struct_level2{9, 10};
    CHECK_UNARY(a.has_value());
    CHECK_EQ(a, (some_struct_level2{9, 10}));

    a.get_unchecked() = some_struct_level2{-1, 1};
    CHECK_UNARY_FALSE(a.has_value());

    a.get_unchecked() = some_struct_level2{0, -1};
    CHECK_UNARY_FALSE(a.has_value());

    a.get_unchecked() = some_struct_level2{0, 0};
    CHECK_UNARY(a.has_value());

    opt::option<opt::option<some_struct_level2>> b{5, 15};
    CHECK_UNARY(b.has_value());
    CHECK_EQ(b, (some_struct_level2{5, 15}));

    b.reset();
    CHECK_UNARY_FALSE(b.has_value());

    b.get_unchecked().get_unchecked() = some_struct_level2{-1, 0};
    CHECK_UNARY_FALSE(b->has_value());

    b.get_unchecked().get_unchecked() = some_struct_level2{0, -1};
    CHECK_UNARY_FALSE(b.has_value());

    b.get_unchecked().get_unchecked() = some_struct_level2{-1, -1};
    CHECK_UNARY_FALSE(b->has_value());
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

TEST_CASE("optional members") {
    CHECK_EQ(sizeof(opt::option<optional_members>), sizeof(optional_members));
    CHECK_GT(sizeof(opt::option<opt::option<optional_members>>), sizeof(optional_members));
    CHECK_GT(sizeof(opt::option<opt::option<opt::option<optional_members>>>), sizeof(optional_members));

    opt::option<optional_members> a;
    CHECK_UNARY_FALSE(a.has_value());

    a = optional_members{1, 2};
    CHECK_UNARY(a.has_value());
    CHECK_EQ(a->x, std::uint32_t(-1));
    CHECK_EQ(a->y, 2);

    a.reset();
    CHECK_UNARY_FALSE(a.has_value());

    a.emplace(100u, 2u);
    a.emplace(100u, 2u);
    CHECK_UNARY(a.has_value());
    CHECK_EQ(a->x, std::uint32_t(-1));
    CHECK_EQ(a->y, 2);

    a->x = 0;
    CHECK_UNARY_FALSE(a.has_value());
    a.get_unchecked().x = std::uint32_t(-1);
    CHECK_UNARY(a.has_value());
    a.get_unchecked().x = 0;
    CHECK_UNARY_FALSE(a.has_value());
}

TEST_CASE("enumeration") {
    SUBCASE("uint8_t") {
        SUBCASE("empty") {
            enum class empty : std::uint8_t {};

            using traits = opt::option_traits<empty>;
            CHECK_EQ(traits::max_level, 0);
        }
        SUBCASE("single enumerator") {
            enum class enumeration : std::uint8_t {
                enumerator1
            };

            using traits = opt::option_traits<enumeration>;
            CHECK_EQ(traits::max_level, 255);

            enumeration a{};
            CHECK_EQ(a, enumeration::enumerator1);
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
            traits::set_level(&a, 0);
            CHECK_EQ(traits::get_level(&a), 0);
            traits::set_level(&a, 1);
            CHECK_EQ(traits::get_level(&a), 1);
            traits::set_level(&a, 254);
            CHECK_EQ(traits::get_level(&a), 254);
            a = enumeration::enumerator1;
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
        }
        SUBCASE("multiple enumerators") {
            enum class enumeration : std::uint8_t {
                enumerator1, enumerator2, enumerator3, enumerator4
            };
            using traits = opt::option_traits<enumeration>;
            CHECK_EQ(traits::max_level, 252);

            enumeration b{};
            b = enumeration::enumerator1;
            CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
            b = enumeration::enumerator2;
            CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
            b = enumeration::enumerator3;
            CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
            b = enumeration::enumerator4;
            CHECK_EQ(traits::get_level(&b), std::uintmax_t(-1));
            traits::set_level(&b, 0);
            CHECK_EQ(traits::get_level(&b), 0);
            traits::set_level(&b, 1);
            CHECK_EQ(traits::get_level(&b), 1);
        }
        SUBCASE("full enumeration") {
            enum class enumeration1 : std::uint8_t {
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
            using traits1 = opt::option_traits<enumeration1>;
            CHECK_EQ(traits1::max_level, 0);

            enum class enumeration2 : std::uint8_t {
                a255 = 255
            };
            using traits2 = opt::option_traits<enumeration2>;
            CHECK_EQ(traits2::max_level, 0);

            enum class enumeration3 : std::uint8_t {
                a254 = 127
            };
            using traits3 = opt::option_traits<enumeration3>;
            CHECK_GT(traits3::max_level, 0);

            enumeration3 c = enumeration3::a254;
            CHECK_EQ(traits3::get_level(&c), std::uintmax_t(-1));
            traits3::set_level(&c, 0);
            CHECK_EQ(traits3::get_level(&c), 0);
        }
        // NOLINTBEGIN(clang-analyzer-optin.core.EnumCastOutOfRange)
        SUBCASE("flag enum") {
            enum class flag_enum : std::uint8_t {
                a = 1,
                b = 2,
                c = 4,
                d = 8,
                e = 16
            };
            const auto combine = [](auto... x) {
                return flag_enum((std::uint8_t(x) | ...));
            };

            using traits = opt::option_traits<flag_enum>;
            CHECK_EQ(traits::max_level, 224);

            flag_enum a{};
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
            a = flag_enum::a;
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
            a = combine(flag_enum::a, flag_enum::b);
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
            a = combine(flag_enum::a, flag_enum::c);
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
            a = combine(flag_enum::d, flag_enum::e);
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
            a = combine(flag_enum::a, flag_enum::b, flag_enum::c, flag_enum::d, flag_enum::e);
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
            traits::set_level(&a, 0);
            CHECK_EQ(traits::get_level(&a), 0);
            CHECK(std::uint8_t(a) > std::uint8_t(combine(flag_enum::a, flag_enum::b, flag_enum::c, flag_enum::d, flag_enum::e)));
            traits::set_level(&a, 1);
            CHECK_EQ(traits::get_level(&a), 1);
            CHECK(std::uint8_t(a) > std::uint8_t(combine(flag_enum::a, flag_enum::b, flag_enum::c, flag_enum::d, flag_enum::e)));
            traits::set_level(&a, 223);
            CHECK_EQ(traits::get_level(&a), 223);
            CHECK(std::uint8_t(a) > std::uint8_t(combine(flag_enum::a, flag_enum::b, flag_enum::c, flag_enum::d, flag_enum::e)));
        }
        SUBCASE("full enum flag") {
            enum class enum_flag : std::uint8_t {
                a = 1 << 0,
                b = 1 << 1,
                c = 1 << 2,
                d = 1 << 3,
                e = 1 << 4,
                f = 1 << 5,
                g = 1 << 6,
                h = 1 << 7
            };
            using traits = opt::option_traits<enum_flag>;
            CHECK_EQ(traits::max_level, 0);
        }
        // NOLINTEND(clang-analyzer-optin.core.EnumCastOutOfRange)
    }
    SUBCASE("std::byte") {
        using traits = opt::option_traits<std::byte>;
        CHECK_EQ(traits::max_level, 0);
    }
    SUBCASE("uint16_t") {
        SUBCASE("empty") {
            enum class empty : std::uint16_t {};
            using traits = opt::option_traits<empty>;

            CHECK_EQ(traits::max_level, 0);
        }
        SUBCASE("enumerators") {
            enum class enumeration : std::uint16_t {
                a = 500
            };
            using traits = opt::option_traits<enumeration>;
            CHECK_GT(traits::max_level, 1);

            enumeration a = enumeration::a;
            CHECK_EQ(traits::get_level(&a), std::uintmax_t(-1));
            traits::set_level(&a, 0);
            CHECK_EQ(traits::get_level(&a), 0);
        }
    }
}

TEST_CASE("opt::has_option_traits") {
    CHECK_UNARY_FALSE(opt::has_option_traits<int>);
    CHECK_UNARY(opt::has_option_traits<float>);
    CHECK_UNARY(opt::has_option_traits<double>);
}

TEST_SUITE_END();

}

