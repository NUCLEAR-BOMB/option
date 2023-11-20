#include <gtest/gtest.h>

#include "utils.hpp"

#define TypeParam T

namespace {

struct some_struct {
    int x;
    int y;
    int z() { return 0; } // NOLINT
};

template<class T>
struct special : public ::testing::Test {
    const T A = false;
    const T B = true;
    const opt::option<T> E{opt::none};
};

template<>
struct special<int*> : public ::testing::Test {
    int* A = nullptr;
    int* B = nullptr;
    const opt::option<int*> E{opt::none};

    void SetUp() override {
        A = new int{0};
        B = new int{1};
    }
    void TearDown() override {
        delete A;
        delete B;
    }
};
static_assert((opt::option_flag<int*>::empty_value % 2) != 0);

template<>
struct special<float> : ::testing::Test {
    float A = 1.1f;
    float B = 2.3f;
    const opt::option<float> E{opt::none};
};
template<>
struct special<double> : ::testing::Test {
    double A = 1.234;
    double B = 12.334243;
    const opt::option<double> E{opt::none};
};

using special_types = ::testing::Types<bool, int*, opt::option<bool>, float, double>;
TYPED_TEST_SUITE(special, special_types,);

TYPED_TEST(special, basic) {
    static_assert(sizeof(opt::option<T>) == sizeof(T));
    const opt::option<T> a{};
    EXPECT_FALSE(a.has_value());
    const opt::option<T> b{this->A};
    EXPECT_TRUE(b.has_value());
}
TYPED_TEST(special, reset) {
    opt::option<T> a;
    EXPECT_FALSE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());
    a = this->A;
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(*a, this->A);
    a.reset();
    EXPECT_FALSE(a.has_value());
    a = this->B;
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(*a, this->B);
    a.reset();
    EXPECT_FALSE(a.has_value());
}
TYPED_TEST(special, assigment) {
    opt::option<T> a;
    EXPECT_FALSE(a.has_value());
    a = this->A;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, this->A);
    a = this->A;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, this->A);
    a = this->B;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, this->B);
    a = this->E;
    EXPECT_FALSE(a.has_value());
    a = this->E;
    EXPECT_FALSE(a.has_value());
    a = opt::option<T>{this->A};
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a, this->A);
    a = opt::none;
    EXPECT_FALSE(a.has_value());
    a = opt::none;
    EXPECT_FALSE(a.has_value());
}
TYPED_TEST(special, emplace) {
    opt::option<T> a{this->A};
    EXPECT_EQ(a, this->A);
    a.emplace(this->A);
    EXPECT_EQ(a, this->A);
    a.reset();
    EXPECT_EQ(a, opt::none);
    a.emplace(this->B);
    EXPECT_EQ(a, this->B);
    a.emplace(this->A);
    EXPECT_EQ(a, this->A);
}
TYPED_TEST(special, take) {
    opt::option<T> a;
    auto b = a.take();
    EXPECT_FALSE(a.has_value());
    EXPECT_FALSE(a.has_value());
    a = this->A;
    EXPECT_TRUE(a.has_value());
    b = a.take();
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b, this->A);
    EXPECT_FALSE(a.has_value());
    a = b.take();
    EXPECT_TRUE(a.has_value());
    EXPECT_FALSE(b.has_value());
    EXPECT_EQ(a, this->A);
    a = this->B;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, this->B);
    a.take();
    EXPECT_FALSE(a.has_value());
}

struct reference : ::testing::Test {};

static_assert(sizeof(opt::option<int&>) == sizeof(int*));

static_assert(is_trivial_compatible<opt::option<int&>>);
static_assert(std::is_trivially_default_constructible_v<opt::option<int&>>);
static_assert(is_trivial_compatible<opt::option<nontrivial_struct&>>);

TEST_F(reference, basic) {
    int a = 1;
    opt::option<int&> ref{a};

    static_assert(std::is_same_v<decltype(*ref), int&>);
    static_assert(std::is_same_v<decltype(*as_const(ref)), int&>);
    static_assert(std::is_same_v<decltype(*as_rvalue(ref)), int&&>);

    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &a);
    EXPECT_EQ(*ref, 1);

    *ref = 2;
    EXPECT_EQ(&(ref.get()), &a);
    EXPECT_EQ(a, 2);

    int b = 3;
    *ref = b;
    EXPECT_EQ(&(ref.get()), &a);
    EXPECT_EQ(a, 3);

    b = 4;
    ref = std::ref(b);
    EXPECT_EQ(&(ref.get()), &b);
    EXPECT_EQ(b, 4);

    ref = a;
    EXPECT_EQ(&(ref.get()), &a);
    EXPECT_EQ(a, 3);

    ref = opt::none;
    EXPECT_FALSE(ref.has_value());

    ref = opt::option<int&>{b};
    EXPECT_EQ(&(ref.get()), &b);

    ref = opt::option<int&>{opt::none};
    EXPECT_FALSE(ref.has_value());

    static_assert(std::is_same_v<
        opt::option<std::reference_wrapper<int>>,
        decltype(opt::option{std::ref(b)})
    >);
    ref = opt::option{std::ref(b)};
    EXPECT_EQ(&(ref.get()), &b);

    const opt::option<int&> refc{a};
    EXPECT_TRUE(refc.has_value());
    EXPECT_EQ(&(refc.get()), &a);
    EXPECT_EQ(*refc, 3);

    *refc = 4;
    EXPECT_EQ(*refc, 4);
    EXPECT_EQ(a, 4);
}

TEST_F(reference, const_basic) {
    const int a = 1;
    opt::option<const int&> ref{a};

    static_assert(std::is_same_v<decltype(*ref), const int&>);
    static_assert(std::is_same_v<decltype(*as_const(ref)), const int&>);
    static_assert(std::is_same_v<decltype(*as_rvalue(ref)), const int&&>);

    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &a);
    EXPECT_EQ(*ref, 1);

    ref = opt::none;
    EXPECT_FALSE(ref.has_value());

    int b = 2;
    ref = b;
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &b);
    EXPECT_EQ(*ref, 2);

    b = 3;
    EXPECT_EQ(*ref, 3);

    ref = opt::option<int&>{b};
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &b);
    EXPECT_EQ(*ref, 3);

    ref = opt::option<int&>{opt::none};
    EXPECT_FALSE(ref.has_value());
    ref = opt::option<const int&>{opt::none};
    EXPECT_FALSE(ref.has_value());

    ref = opt::option<const int&>{std::ref(a)};
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &a);

    ref = opt::option<const int&>{std::ref(b)};
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &b);

    ref = opt::option<const int&>{std::cref(b)};
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &b);

    ref = std::ref(a);
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &a);
    EXPECT_EQ(*ref, 1);

    ref = std::cref(a);
    EXPECT_TRUE(ref.has_value());

    ref = std::ref(b);
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &b);

    ref = std::cref(b);
    EXPECT_TRUE(ref.has_value());
    EXPECT_EQ(&(ref.get()), &b);

    const opt::option<const int&> refc{a};
    EXPECT_TRUE(refc.has_value());
    EXPECT_EQ(&(refc.get()), &a);
    EXPECT_EQ(*refc, 1);
}

enum class some_enum {
    x, y, z,
    OPTION_EXPLOIT_UNUSED_VALUE
};
enum class simple_enum {
    a, b, c
};

struct exploit_enum : ::testing::Test {};

static_assert(sizeof(opt::option<some_enum>) == sizeof(some_enum));
static_assert(sizeof(opt::option<simple_enum>) > sizeof(some_enum));

TEST_F(exploit_enum, basic) {
    opt::option<some_enum> a{some_enum::x};
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, some_enum::x);

    a = some_enum::y;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, some_enum::y);

    a.reset();
    EXPECT_FALSE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());

    a = some_enum::z;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, some_enum::z);
    a = some_enum::z;
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, some_enum::z);

    [[maybe_unused]] constexpr opt::option<some_enum> ca{};
    [[maybe_unused]] constexpr opt::option<some_enum> cb{some_enum::x};
}

}
