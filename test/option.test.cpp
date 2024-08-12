#include <gtest/gtest.h>
#include <memory>
#include <type_traits>
#include <opt/option.hpp>
#include <utility>
#include <tuple>
#include <array>
#include <cfenv>
#include <string_view>
#include <functional>
#include <cstdint>

#include "utils.hpp"

#if OPTION_MSVC
    #pragma fenv_access(on)
#elif OPTION_CLANG
    #if __clang_major__ >= 12
        #pragma STDC FENV_ACCESS ON
    #endif
#endif

#define TypeParam T
#define V0 (this->values[0])
#define V1 (this->values[1])
#define V2 (this->values[2])
#define V3 (this->values[3])
#define V4 (this->values[4])

#define TEST_SIZE_LIST \
    struct_with_sentinel, \
    int(*)(int), std::string_view, polymorphic_type, empty_polymorphic_type, aggregate_with_empty_struct, \
    aggregate_int_float, std::array<int, 0>, \
    empty_struct, std::tuple<>, std::tuple<int, float, int>, \
    double, bool, std::reference_wrapper<int>, int*, float, \
    std::pair<int, float>, std::pair<float, int>, std::array<float, 4>
    

#define TEST_MAIN_LIST \
    int, unsigned int

namespace {

struct trivial_struct {};
static_assert(std::is_trivially_destructible_v<opt::option<trivial_struct>>);

static_assert(is_not_trivial_compatible<opt::option<nontrivial_struct>>);
static_assert(is_trivial_compatible<opt::option<int>>);

static_assert(std::is_nothrow_destructible_v<opt::option<int>>);
static_assert(!std::is_nothrow_destructible_v<opt::option<nontrivial_struct>>);

template<class T>
struct option : ::testing::Test {
    T values[5]{1, 2, 3, 4, 5};
};
template<>
struct option<float> : ::testing::Test {
    float values[5]{1.f, 2.f, 3.f, 4.f, 5.f};

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
template<>
struct option<double> : ::testing::Test {
    double values[5]{1., 2., 3., 4., 5.};

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

template<>
struct option<bool> : ::testing::Test {
    bool values[5]{false, true, false, true, false};
};
template<>
struct option<std::reference_wrapper<int>> : ::testing::Test {
    int values_orig[5]{1, 2, 3, 4, 5};
    std::reference_wrapper<int> values[5]{values_orig[0], values_orig[1], values_orig[2], values_orig[3], values_orig[4]};
};
template<>
struct option<int*> : ::testing::Test {
    int values_orig[5]{1, 2, 3, 4, 5};
    int* values[5]{&values_orig[0], &values_orig[1], &values_orig[2], &values_orig[3], &values_orig[4]};
};
template<>
struct option<std::pair<int, float>> : ::testing::Test {
    std::pair<int, float> values[5]{{1, 2.f}, {3, 4.f}, {5, 6.f}, {7, 8.f}, {9, 10.f}};
};
template<>
struct option<std::pair<float, int>> : ::testing::Test {
    std::pair<float, int> values[5]{{1.f, 2}, {3.f, 4}, {5.f, 6}, {7.f, 8}, {9.f, 10}};
};
template<>
struct option<std::array<float, 4>> : ::testing::Test {
    std::array<float, 4> values[5]{{1.f, 2.f, 3.f, 4.f}, {5.f, 6.f, 7.f, 8.f}, {9.f, 10.f, 11.f, 12.f}, {13.f, 14.f, 15.f, 16.f}, {17.f, 18.f, 19.f, 20.f}};
};
template<>
struct option<std::array<int, 0>> : ::testing::Test {
    std::array<int, 0> values[5]{{}, {}, {}, {}, {}};
};

struct empty_struct {
    bool operator==(const empty_struct&) const { return true; }
};
template<>
struct option<empty_struct> : ::testing::Test {
    empty_struct values[5]{{}, {}, {}, {}, {}};
};

template<>
struct option<std::tuple<>> : ::testing::Test {
    std::tuple<> values[5]{{}, {}, {}, {}, {}};
};
template<>
struct option<std::tuple<int, float, int>> : ::testing::Test {
    std::tuple<int, float, int> values[5]{{1, 2.f, 3}, {4, 5.f, 6}, {7, 8.f, 9}, {10, 11.f, 12}, {13, 14.f, 15}};
};

struct aggregate_int_float {
    int x;
    float y;

    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    bool operator==(const aggregate_int_float& a) const { return x == a.x && y == a.y; }
};
template<>
struct option<aggregate_int_float> : ::testing::Test {
    aggregate_int_float values[5]{{1, 2.f}, {3, 4.f}, {5, 6.f}, {7, 8.f}, {9, 10.f}};
};

struct aggregate_with_empty_struct {
    int x;
    empty_struct y;

    bool operator==(const aggregate_with_empty_struct& a) const { return x == a.x; }
};
template<>
struct option<aggregate_with_empty_struct> : ::testing::Test {
    aggregate_with_empty_struct values[5]{
        {1, {}}, {2, {}}, {3, {}}, {4, {}}, {5, {}}
    };
};

struct empty_polymorphic_type {
    empty_polymorphic_type() = default;
    empty_polymorphic_type(const empty_polymorphic_type&) = default;
    empty_polymorphic_type& operator=(const empty_polymorphic_type&) = default;

    virtual ~empty_polymorphic_type() = default;

    bool operator==(const empty_polymorphic_type&) const { return true; }
};
static_assert(std::is_polymorphic_v<empty_polymorphic_type>);

template<>
struct option<empty_polymorphic_type> : ::testing::Test {
    empty_polymorphic_type values[5]{{}, {}, {}, {}, {}};
};

struct polymorphic_type {
    int x;

    polymorphic_type(int x_) : x(x_) {}

    polymorphic_type() = default;
    polymorphic_type(const polymorphic_type&) = default; // NOLINT(clang-analyzer-core.uninitialized.Assign)
    polymorphic_type& operator=(const polymorphic_type&) = default;

    virtual ~polymorphic_type() = default;

    bool operator==(const polymorphic_type& other) const { return x == other.x; }
};
static_assert(std::is_polymorphic_v<polymorphic_type>);

template<>
struct option<polymorphic_type> : ::testing::Test {
    polymorphic_type values[5]{{1}, {2}, {3}, {4}, {5}};
};

template<>
struct option<std::string_view> : ::testing::Test {
    std::string_view values[5]{"a1", "b2", "c3", "d4", "e5"};
};

template<>
struct option<int(*)(int)> : ::testing::Test {
    using t = int(*)(int);
    t values[5]{
        [](int x) { return x + 1; },
        [](int x) { return x + 2; },
        [](int x) { return x + 3; },
        [](int x) { return x + 4; },
        [](int x) { return x + 5; }
    };
};

struct struct_with_sentinel {
    int x;
    std::uint8_t SENTINEL{};

    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    bool operator==(const struct_with_sentinel& o) const { return x == o.x; }
};
template<>
struct option<struct_with_sentinel> : ::testing::Test {
    struct_with_sentinel values[5]{struct_with_sentinel{1}, struct_with_sentinel{2}, struct_with_sentinel{3}, struct_with_sentinel{4}, struct_with_sentinel{5}};
};


using test_size_types = ::testing::Types<TEST_SIZE_LIST>;

template<class T>
struct test_size : ::testing::Test {};
TYPED_TEST_SUITE(test_size, test_size_types);
TYPED_TEST(test_size, check) {
    EXPECT_EQ(sizeof(T), sizeof(opt::option<T>));
    EXPECT_EQ(sizeof(T), sizeof(opt::option<opt::option<T>>));
    EXPECT_EQ(sizeof(T), sizeof(opt::option<opt::option<opt::option<T>>>));
    EXPECT_EQ(sizeof(T), sizeof(opt::option<opt::option<opt::option<opt::option<T>>>>));
}

using test_main_types = ::testing::Types<TEST_SIZE_LIST, TEST_MAIN_LIST>;
TYPED_TEST_SUITE(option, test_main_types);

TYPED_TEST(option, default_ctor) {
    const opt::option<T> a;
    EXPECT_FALSE(a.has_value());

    const opt::option<T> b(opt::none);
    EXPECT_FALSE(b.has_value());

    const opt::option<T> c = opt::none;
    EXPECT_FALSE(c.has_value());
}
TYPED_TEST(option, constructors) {
    if constexpr (std::is_default_constructible_v<T>) {
        const opt::option<T> a{{}};
        EXPECT_EQ(a, T{});
    }
}
TYPED_TEST(option, get) {
    opt::option<T> a{V0};
    EXPECT_EQ(a.get(), V0);
    EXPECT_EQ(std::as_const(a).get(), V0);
    EXPECT_EQ(as_const_rvalue(a).get(), V0);
    EXPECT_EQ(as_rvalue(a).get(), V0);
}
TYPED_TEST(option, assignment) {
    opt::option<T> a = V0;
    a = opt::none;
    EXPECT_FALSE(a.has_value());
    {
        const opt::option tmp{V1};
        a = tmp;
    }
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, V1);
    {
        const opt::option<T> tmp{opt::none};
        a = tmp;
    }
    EXPECT_FALSE(a.has_value());
    a = opt::option<T>(V2);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, V2);
    a = opt::option<T>(opt::none);
    EXPECT_FALSE(a.has_value());
    EXPECT_FALSE(a.has_value());
    {
        const T tmp = V3;
        a = tmp;
    }
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, V3);
    a = V4;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, V4);

    opt::option<T> b;
    EXPECT_FALSE(b.has_value());
    b = V0;
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, V0);
    b = V0;
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, V0);
    b = V1;
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, V1);
    b = opt::option<T>{};
    EXPECT_FALSE(b.has_value());
    b = opt::option<T>{};
    EXPECT_FALSE(b.has_value());
    b = opt::option<T>{V0};
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, V0);
    b = opt::none;
    EXPECT_FALSE(b.has_value());
    b = opt::none;
    EXPECT_FALSE(b.has_value());

    opt::option<T> c;
    EXPECT_FALSE(c.has_value());
    c = V0;
    EXPECT_TRUE(c.has_value());
    EXPECT_EQ(c, V0);

    opt::option<T> d;
    EXPECT_FALSE(d.has_value());
    d = c;
    EXPECT_TRUE(c.has_value());
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(d, V0);

    c = opt::none;
    EXPECT_FALSE(c.has_value());
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(d, V0);
    d = opt::none;

    c = d;
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(d.has_value());

    c = opt::option<T>{d};
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(d.has_value());

    d = V0;
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(d, V0);

    c = opt::option<T>{d};
    EXPECT_TRUE(c.has_value());
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(d, V0);
    EXPECT_EQ(c, V0);

    d = opt::option<T>{c};
    EXPECT_TRUE(c.has_value());
    EXPECT_TRUE(d.has_value());

    d = opt::none;
    EXPECT_FALSE(d.has_value());
    c = opt::option<T>{d};
    EXPECT_FALSE(c.has_value());
    EXPECT_FALSE(d.has_value());

    c = V0;
    EXPECT_TRUE(c.has_value());
    d = std::move(c);
    EXPECT_TRUE(d.has_value());
    EXPECT_TRUE(c.has_value()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)

    d = opt::none;
    c = V0;
    EXPECT_FALSE(d.has_value());
    EXPECT_TRUE(c.has_value());
    d = opt::option<T>{std::move(c)};
    EXPECT_TRUE(d.has_value());
    EXPECT_TRUE(c.has_value()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
}

TYPED_TEST(option, reset) {
    opt::option<T> a;
    EXPECT_FALSE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());
    a = V0;
    EXPECT_TRUE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());
}
TYPED_TEST(option, emplace) {
    opt::option<T> a{V0};
    EXPECT_EQ(a, V0);
    a.emplace(V0);
    EXPECT_EQ(a, V0);
    a.reset();
    EXPECT_EQ(a, opt::none);
    a.emplace(V1);
    EXPECT_EQ(a, V1);
    a.emplace(V0);
    EXPECT_EQ(a, V0);
}
TYPED_TEST(option, hash) {
    if constexpr (is_hashable<T>) {
        opt::option<T> a{V0};
        opt::option<T> b{V0};
        EXPECT_EQ(hash_fn(a), hash_fn(b));
        a = opt::none;
        EXPECT_NE(hash_fn(a), hash_fn(b));
        b = opt::none;
        EXPECT_EQ(hash_fn(a), hash_fn(b));
    }
}
TYPED_TEST(option, value_or_throw) {
    opt::option<T> a{V0};
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
TYPED_TEST(option, value_or) {
    opt::option<T> a; // NOLINT(clang-analyzer-core.uninitialized.UndefReturn)
    EXPECT_EQ(a.value_or(V0), V0); // NOLINT(clang-analyzer-core.UndefinedBinaryOperatorResult, clang-analyzer-core.uninitialized.Assign)
    a = V1;
    EXPECT_EQ(a.value_or(V2), V1);
    EXPECT_EQ(as_rvalue(a).value_or(V3), V1);
}
TYPED_TEST(option, and_then) {
    if constexpr (std::is_same_v<T, int>) {
        const auto convert_to_uint = [](int x) -> opt::option<unsigned> {
            if (x >= 0) { return opt::option<unsigned>{unsigned(x + 1)}; }
            return opt::none;
        };
        EXPECT_EQ(opt::option<int>{2}.and_then(convert_to_uint), 3u);
        EXPECT_EQ(opt::option<int>{-10}.and_then(convert_to_uint), opt::none);
        EXPECT_EQ(opt::option<int>{opt::none}.and_then(convert_to_uint), opt::none);
    }
}
TYPED_TEST(option, map) {
    if constexpr (std::is_same_v<T, int>) {
        const auto func = [](auto x) { return x - 1; };
        EXPECT_EQ(opt::option<int>{1}.map(func), 0);
        EXPECT_EQ(opt::option<int>{}.map(func), opt::none);
        EXPECT_EQ(opt::option<int>{10}.map(func).map(func), 8);
    }
}
TYPED_TEST(option, or_else) {
    if constexpr (std::is_same_v<T, int>) {
        const auto func = []() { return opt::option{1 << 10}; };
        EXPECT_EQ(opt::option<int>{1}.or_else(func), 1);
        EXPECT_EQ(opt::option<int>{}.or_else(func), 1 << 10);
        EXPECT_EQ(opt::option<int>{}.or_else(func).or_else(func), 1 << 10);
    }
}
TYPED_TEST(option, take) {
    opt::option<T> a;
    opt::option<T> b = a.take();
    EXPECT_FALSE(a.has_value());
    EXPECT_FALSE(a.has_value());
    a = V0;
    EXPECT_FALSE(b.has_value());
    EXPECT_TRUE(a.has_value());
    b = a.take();
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, V0);
    EXPECT_FALSE(a.has_value());
    a = b.take();
    EXPECT_TRUE(a.has_value());
    EXPECT_FALSE(b.has_value());
    EXPECT_EQ(a, V0);
    a = V1;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, V1);
    a.take();
    EXPECT_FALSE(a.has_value());
}
TYPED_TEST(option, option_cast) {
    if constexpr (std::is_same_v<T, int>) {
        opt::option<int> a{1};
        opt::option<unsigned> b = opt::option_cast<unsigned>(a);
        EXPECT_EQ(*b, 1u);
        b = opt::option_cast<unsigned>(as_rvalue(a));
        EXPECT_EQ(*b, 1u);
    }
}
TYPED_TEST(option, deduction_guides) {
    auto a = opt::option{V0}; // NOLINT(misc-const-correctness)
    static_assert(std::is_same_v<decltype(a), opt::option<T>>);
    opt::option b{V1}; // NOLINT(misc-const-correctness)
    static_assert(std::is_same_v<decltype(b), opt::option<T>>);

    auto c = opt::option{opt::option{V0}};
    static_assert(std::is_same_v<decltype(c), opt::option<opt::option<T>>>);

    auto d = opt::option{opt::option{opt::option{V1}}};
    static_assert(std::is_same_v<decltype(d), opt::option<opt::option<opt::option<T>>>>);
}
TYPED_TEST(option, value_or_default) {
    if constexpr (std::is_default_constructible_v<T>) {
        opt::option a{V0};
        EXPECT_EQ(a.value_or_default(), V0);
        a = opt::none;
        EXPECT_EQ(a.value_or_default(), T{});
    }
}
TYPED_TEST(option, ptr_or_pull) {
    opt::option a{V0};

    EXPECT_EQ(*(a.ptr_or_null()), V0);
    a = opt::none;
    EXPECT_EQ(a.ptr_or_null(), nullptr);
    a = V1;
    EXPECT_EQ(*(as_const(a).ptr_or_null()), V1);
}
TYPED_TEST(option, filter) {
    if constexpr (std::is_same_v<T, int>) {
        const auto is_even = [](int x) { return x % 2 == 0; };

        opt::option a{1};
        EXPECT_EQ(a.filter(is_even), opt::none);
        a = 2;
        EXPECT_EQ(a.filter(is_even), 2);
        a = opt::none;
        EXPECT_EQ(a.filter(is_even), opt::none);
    }
}
TYPED_TEST(option, flatten) {
    auto a = opt::option{opt::option{V0}};
    EXPECT_EQ(**a, V0);
    auto b = a.flatten();
    EXPECT_EQ(*b, V0);

    a = opt::option{opt::option<T>{opt::none}};
    b = a.flatten();
    EXPECT_FALSE(b.has_value());
    a = opt::option<T>{opt::none};
    b = a.flatten();
    EXPECT_FALSE(b.has_value());
    a = opt::none;
    b = a.flatten();
    EXPECT_FALSE(b.has_value());
}
TYPED_TEST(option, map_or) {
    if constexpr (std::is_same_v<T, int>) {
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
}
TYPED_TEST(option, map_or_else) {
    if constexpr (std::is_same_v<T, int>) {
        const auto default_fn = []() { return 2; };
        const auto do_fn = [](int x) { return x - 1; };

        opt::option a{1};
        EXPECT_EQ(a.map_or_else(default_fn, do_fn), 0);
        a = opt::none;
        EXPECT_EQ(a.map_or_else(default_fn, do_fn), 2);
    }
}
TYPED_TEST(option, take_if) {
    if constexpr (std::is_same_v<T, int>) {
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
}
TYPED_TEST(option, has_value_and) {
    opt::option a{V0};
    EXPECT_TRUE(a.has_value_and([&](const T& x) { return x == V0; }));
    if (!(V0 == V1)) {
        EXPECT_FALSE(a.has_value_and([&](const T& x) { return x == V1; }));
    }
    a.reset();
    EXPECT_FALSE(a.has_value_and([&](const T& x) { return x == V0; }));
    EXPECT_FALSE(a.has_value_and([&](const T& x) { return x == V1; }));
}
TYPED_TEST(option, insert) {
    opt::option a{V0};
    const T& x = a.insert(V1);
    EXPECT_EQ(&x, a.ptr_or_null());
    EXPECT_EQ(*a, V1);

    const T tmp = V2;
    const T& y = a.insert(tmp);

    EXPECT_EQ(&y, a.ptr_or_null());
    EXPECT_EQ(*a, V2);
}
TYPED_TEST(option, inspect) {
    if constexpr (std::is_same_v<T, int>) {
        opt::option a{1};
        int x = 0;
        a.inspect([&](int y) { x += y; });
        EXPECT_EQ(x, 1);
        opt::option<int>{a}
            .inspect([&](const int& y) { x += y; })
            .inspect([&](int& y) { return x += y * 2; });
        EXPECT_EQ(x, 4);
    }
}
TYPED_TEST(option, assume_has_value) {
    opt::option a{V0};
    a.assume_has_value();
    EXPECT_EQ(*a, V0);
}
TYPED_TEST(option, unzip) {
    { // std::tuple
        opt::option a{std::tuple{V0, V1, V2, V3}};
        EXPECT_TRUE(a.has_value());

        auto b = a.unzip();
        static_assert(std::is_same_v<decltype(b), std::tuple<
            opt::option<T>, opt::option<T>, opt::option<T>, opt::option<T>
        >>);
        auto& [b1, b2, b3, b4] = b;

        EXPECT_TRUE(b1.has_value());
        EXPECT_EQ(*b1, V0);
        EXPECT_TRUE(b2.has_value());
        EXPECT_EQ(*b2, V1);
        EXPECT_TRUE(b3.has_value());
        EXPECT_EQ(*b3, V2);
        EXPECT_TRUE(b4.has_value());
        EXPECT_EQ(*b4, V3);

        a.reset();
        auto c = a.unzip();
        auto& [c1, c2, c3, c4] = c;
        EXPECT_FALSE(c1.has_value());
        EXPECT_FALSE(c2.has_value());
        EXPECT_FALSE(c3.has_value());
        EXPECT_FALSE(c4.has_value());
    }
    { // std::pair
        opt::option a{std::pair{V0, V1}};
        EXPECT_TRUE(a.has_value());

        auto b = a.unzip();
        static_assert(std::is_same_v<decltype(b), std::pair<opt::option<T>, opt::option<T>>>);
        auto& [b1, b2] = b;

        EXPECT_TRUE(b1.has_value());
        EXPECT_EQ(*b1, V0);
        EXPECT_TRUE(b2.has_value());
        EXPECT_EQ(*b2, V1);

        a.reset();
        auto c = a.unzip();
        auto& [c1, c2] = c;
        EXPECT_FALSE(c1.has_value());
        EXPECT_FALSE(c2.has_value());
    }
    { // std::array
        opt::option a{std::array{V0, V1, V2}};
        EXPECT_TRUE(a.has_value());

        auto b = a.unzip();
        static_assert(std::is_same_v<decltype(b), std::array<opt::option<T>, 3>>);
        auto& [b1, b2, b3] = b;

        EXPECT_TRUE(b1.has_value());
        EXPECT_EQ(*b1, V0);
        EXPECT_TRUE(b2.has_value());
        EXPECT_EQ(*b2, V1);
        EXPECT_TRUE(b3.has_value());
        EXPECT_EQ(*b3, V2);

        a.reset();
        auto c = a.unzip();
        auto& [c1, c2, c3] = c;
        EXPECT_FALSE(c1.has_value());
        EXPECT_FALSE(c2.has_value());
        EXPECT_FALSE(c3.has_value());
    }
}
TYPED_TEST(option, function_zip) {
    opt::option<T> a{V0};
    opt::option<T> b{V1};

    auto c = opt::zip(a, b);
    EXPECT_TRUE(c.has_value());

    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93147
    EXPECT_EQ(std::tuple_size_v<typename decltype(c)::value_type>, 2);
    if constexpr (std::tuple_size_v<typename decltype(c)::value_type> == 2) {
        EXPECT_EQ(std::get<0>(*c), V0);
        EXPECT_EQ(std::get<1>(*c), V1);
    }

    a.reset();
    c = opt::zip(a, b);
    EXPECT_FALSE(c.has_value());

    b.reset();
    c = opt::zip(a, b);
    EXPECT_FALSE(c.has_value());
}
TYPED_TEST(option, function_zip_with) {
    if constexpr (std::is_same_v<T, int>) {
        struct point { float x, y; };
        const auto construct_point = [](float x, float y) {
            return point{x, y};
        };

        const opt::option<int> a{1};
        const opt::option<float> b{2.f};

        auto c = opt::zip_with(construct_point, opt::option_cast<float>(a), b);

        EXPECT_TRUE(c.has_value());
        EXPECT_EQ(c->x, 1.f);
        EXPECT_EQ(c->y, 2.f);

        c = opt::zip_with(construct_point, opt::option<float>{}, b);
        EXPECT_FALSE(c.has_value());

        c = opt::zip_with(construct_point, opt::option<float>{}, opt::option<float>{});
        EXPECT_FALSE(c.has_value());

        const auto do_something_else = [](int x, float y) {
            (void)x;
            (void)y;
        };
        opt::zip_with(do_something_else, a, b);
        opt::zip_with(do_something_else, opt::option<int>{}, b);
        opt::zip_with(do_something_else, a, opt::option<float>{});
    }
}
TYPED_TEST(option, replace) {
    opt::option a{std::make_unique<T>(V0)};

    auto c = a.replace(std::make_unique<T>(V1));
    EXPECT_TRUE(c.has_value());
    EXPECT_EQ(**c, V0);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(**a, V1);
}
TYPED_TEST(option, from_nullable) {
    T a = V0;
    T* ptr = &a;

    const opt::option<T> b = opt::from_nullable(ptr);
    EXPECT_EQ(b, V0);

    ptr = nullptr;
    const opt::option<T> c = opt::from_nullable(ptr);
    EXPECT_FALSE(c.has_value());
}

}
