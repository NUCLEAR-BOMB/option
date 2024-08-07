#include <gtest/gtest.h>
#include <cfenv>
#include <option.hpp>
#include <tuple>
#include <functional>
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>
#include <array>
#include <memory>
#include <cstdint>

#include "utils.hpp"

#if OPTION_USE_BUILTIN_TRAITS

#ifdef _MSC_VER
#pragma fenv_access (on)
#elif defined(__clang__)
#pragma STDC FENV_ACCESS ON
#endif

#define TypeParam T

namespace {

struct some_struct {
    int x;
    int y;
    int z() { return 0; } // NOLINT
};

struct struct2 {
    unsigned long long num;

    struct2(const unsigned long long num_) : num(num_) {}

    bool operator==(const struct2& value) const { return num == value.num; }
private:
    bool has = true;

    friend struct opt::option_traits<struct2>;
};

}

template<>
struct opt::option_traits<struct2> {
    static bool is_empty(const struct2& value) noexcept {
        return !value.has;
    }
    static void set_empty(struct2& value) noexcept {
        value.has = false;
    }
};

struct empty_struct {
    bool operator==(const empty_struct) const { return true; }
};
struct non_trivially_destructible_empty_struct {
    non_trivially_destructible_empty_struct() = default;
    non_trivially_destructible_empty_struct(const non_trivially_destructible_empty_struct&) = default;
    non_trivially_destructible_empty_struct& operator=(const non_trivially_destructible_empty_struct&) = default;

    ~non_trivially_destructible_empty_struct() {} // NOLINT(modernize-use-equals-default)
    bool operator==(const non_trivially_destructible_empty_struct&) const { return true; }
};

struct struct_with_sentinel {
    int data1;
    unsigned int data2;
    char OPTION_SENTINEL{};

    bool operator==(const struct_with_sentinel& a) const { return data1 == a.data1 && data2 == a.data2; }
};

namespace {

using ::testing::Test;

template<class T>
struct special;

template<>
struct special<bool> : Test {
    const bool A{false};
    const bool B{true};
};
template<>
struct special<opt::option<bool>> : Test {
    const opt::option<bool> A{false};
    const opt::option<bool> B{true};
};
template<>
struct special<int*> : Test {
    int* A = nullptr;
    int* B = nullptr;

    void SetUp() override {
        A = new int{0};
        B = new int{1};
    }
    void TearDown() override {
        delete A;
        delete B;
    }
};
static_assert((opt::option_traits<int*>::empty_value % 2) != 0);

template<>
struct special<float> : Test {
    float A = 1.1f;
    float B = 2.3f;

    void SetUp() override {
        ASSERT_EQ(std::feclearexcept(FE_ALL_EXCEPT), 0);
    }

    void TearDown() override {
        const int n = std::fetestexcept(FE_ALL_EXCEPT);
        ASSERT_EQ(n, 0);
        ASSERT_EQ(std::feclearexcept(FE_ALL_EXCEPT), 0);
    }
};
template<>
struct special<double> : Test {
    double A = 1.234;
    double B = 12.334243;

    void SetUp() override {
        ASSERT_EQ(std::feclearexcept(FE_ALL_EXCEPT), 0);
    }

    void TearDown() override {
        const int n = std::fetestexcept(FE_ALL_EXCEPT);
        ASSERT_EQ(n, 0);
        ASSERT_EQ(std::feclearexcept(FE_ALL_EXCEPT), 0);
    }
};
template<>
struct special<std::tuple<int, float>> : Test {
    std::tuple<int, float> A{2, 35.58f};
    std::tuple<int, float> B{10, 1034.124f};
};
template<>
struct special<std::tuple<>> : Test {
    std::tuple<> A{};
    std::tuple<> B{};
};
template<>
struct special<std::reference_wrapper<int>> : Test {
    static inline int Aa = 1;
    static inline int Bb = 2;

    std::reference_wrapper<int> A{Aa};
    std::reference_wrapper<int> B{Bb};
};
template<>
struct special<struct2> : Test {
    const struct2 A{1ULL};
    const struct2 B{2ULL};
};
template<>
struct special<std::string_view> : Test {
    const std::string_view A{"a"};
    const std::string_view B{"b"};
};
template<>
struct special<std::string> : Test {
    const std::string A{"a"};
    const std::string B{"b"};
};
template<>
struct special<std::vector<int>> : Test {
    const std::vector<int> A{{1, 2, 3}};
    const std::vector<int> B{{4, 5, 6}};
};
struct polymorphic {
    int x1{};
    unsigned x2{};

    virtual int do_something() { return x1; }
    virtual unsigned do_something_else() { return x2; }

    virtual ~polymorphic() = default;
    polymorphic& operator=(const polymorphic&) = default;
    polymorphic(const polymorphic&) = default; // NOLINT(clang-analyzer-core.uninitialized.Assign)

    polymorphic(int x1_, unsigned x2_) : x1(x1_), x2(x2_) {}
    bool operator==(const polymorphic& other) const { return x1 == other.x1 && x2 == other.x2; }
};
template<>
struct special<polymorphic> : Test {
    const polymorphic A{1, 2};
    const polymorphic B{3, 4};
};
template<>
struct special<empty_struct> : Test {
    const empty_struct A{};
    const empty_struct B{};
};
template<>
struct special<non_trivially_destructible_empty_struct> : Test {
    const non_trivially_destructible_empty_struct A{};
    const non_trivially_destructible_empty_struct B{};
};
template<>
struct special<struct_with_sentinel> : Test {
    const struct_with_sentinel A{1, 2};
    const struct_with_sentinel B{3, 4};
};

using special_types = ::testing::Types<
    float, bool, int*, opt::option<bool>, double, std::tuple<int, float>,
    std::reference_wrapper<int>, struct2, std::string_view, std::string, std::vector<int>,
    polymorphic, empty_struct, non_trivially_destructible_empty_struct, struct_with_sentinel
>;
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
    a = opt::option<T>{};
    EXPECT_FALSE(a.has_value());
    a = opt::option<T>{};
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
    static_assert(std::is_same_v<decltype(*as_rvalue(ref)), int&>);

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
    static_assert(std::is_same_v<decltype(*as_rvalue(ref)), const int&>);

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

enum class some_enum : std::uint8_t {
    x, y, z,
    OPTION_EXPLOIT_UNUSED_VALUE
};

struct exploit_enum : ::testing::Test {};

static_assert(sizeof(opt::option<some_enum>) == sizeof(some_enum));

#ifdef OPTION_HAS_MAGIC_ENUM
enum class classic_enum_class {
    n, m, k, l
};
static_assert(sizeof(opt::option<classic_enum_class>) == sizeof(classic_enum_class));

enum class unoptimizing_enum : std::uint8_t {
    A, B, C = 255,
};
static_assert(sizeof(opt::option<unoptimizing_enum>) > sizeof(unoptimizing_enum));
#endif

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
}

struct tuple_like : ::testing::Test {};

TEST_F(tuple_like, tuple) {
    static_assert(sizeof(opt::option<std::tuple<int, float>>) == sizeof(std::tuple<int, float>));
    static_assert(sizeof(opt::option<std::tuple<float, int>>) == sizeof(std::tuple<float, int>));
    static_assert(sizeof(opt::option<std::tuple<int, long>>) > sizeof(std::tuple<int, long>));

    opt::option<std::tuple<int, float>> a{1, 2.5f};
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(std::get<0>(*a), 1);
    EXPECT_EQ(std::get<1>(*a), 2.5f);
    a.reset();
    EXPECT_FALSE(a.has_value());

    opt::option<std::tuple<float, double>> b{2.56f, 3.1415};
    static_assert(sizeof(b) == sizeof(std::tuple<float, double>));
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(std::get<0>(*b), 2.56f);
    EXPECT_EQ(std::get<1>(*b), 3.1415);
    b.reset();
    EXPECT_FALSE(b.has_value());

    opt::option<std::tuple<float, int, double>> c{1.f, 2, 3.};
    static_assert(sizeof(c) == sizeof(std::tuple<float, int, double>));
    EXPECT_TRUE(c.has_value());
    EXPECT_EQ(std::get<0>(*c), 1.f);
    EXPECT_EQ(std::get<1>(*c), 2);
    EXPECT_EQ(std::get<2>(*c), 3.);
    c.reset();
    EXPECT_FALSE(c.has_value());

    opt::option<std::tuple<std::tuple<float, int>, long>> d{{{1.f, 2}, 3L}};
    static_assert(sizeof(d) == sizeof(std::tuple<std::tuple<float, int>, long>));
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(std::get<0>(std::get<0>(*d)), 1.f);
    EXPECT_EQ(std::get<1>(std::get<0>(*d)), 2);
    EXPECT_EQ(std::get<1>(*d), 3L);
    d.reset();
    EXPECT_FALSE(c.has_value());

    opt::option<std::tuple<int, std::tuple<float, long>>> e{{1, {2.5f, 100L}}};
    static_assert(sizeof(e) == sizeof(std::tuple<int, std::tuple<float, long>>));
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(std::get<0>(*e), 1);
    EXPECT_EQ(std::get<0>(std::get<1>(*e)), 2.5f);
    EXPECT_EQ(std::get<1>(std::get<1>(*e)), 100L);
    e.reset();
    EXPECT_FALSE(e.has_value());
}
TEST_F(tuple_like, pair) {
    static_assert(sizeof(opt::option<std::pair<int, long>>) > sizeof(std::pair<int, long>));

    opt::option<std::pair<int, float>> a{1, 2.f};
    static_assert(sizeof(a) == sizeof(std::pair<int, float>));
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(std::get<0>(*a), 1);
    EXPECT_EQ(std::get<1>(*a), 2.f);
    a.reset();
    EXPECT_FALSE(a.has_value());

    opt::option<std::pair<float, int>> b{3.f, -1};
    static_assert(sizeof(b) == sizeof(std::pair<float, int>));
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(std::get<0>(*b), 3.f);
    EXPECT_EQ(std::get<1>(*b), -1);
    a.reset();
    EXPECT_FALSE(a.has_value());

    opt::option<std::pair<std::pair<float, int>, unsigned>> c{{{100.f, 250}, 194u}};
    static_assert(sizeof(c) == sizeof(std::pair<std::pair<float, int>, unsigned>));
    EXPECT_TRUE(c.has_value());
    EXPECT_EQ(c->first.first, 100.f);
    EXPECT_EQ(c->first.second, 250);
    EXPECT_EQ(c->second, 194u);
    c.reset();
    EXPECT_FALSE(c.has_value());

    opt::option<std::pair<unsigned, std::pair<float, int>>> d{{2u, {3.f, -1}}};
    static_assert(sizeof(d) == sizeof(std::pair<unsigned, std::pair<float, int>>));
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(d->first, 2u);
    EXPECT_EQ(d->second.first, 3.f);
    EXPECT_EQ(d->second.second, -1);
    d.reset();
    EXPECT_FALSE(d.has_value());
}
TEST_F(tuple_like, array) {
    static_assert(sizeof(opt::option<std::array<int, 2>>) > sizeof(std::array<int, 2>));

    opt::option<std::array<float, 2>> a{{1.f, 2.f}};
    static_assert(sizeof(a) == sizeof(std::array<float, 2>));
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ((*a)[0], 1.f);
    EXPECT_EQ(std::get<1>(*a), 2.f);
    a.reset();
    EXPECT_FALSE(a.has_value());

    opt::option<std::array<double, 1>> b{{1000.1}};
    static_assert(sizeof(b) == sizeof(std::array<double, 1>));
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ((*b)[0], 1000.1);
    b.reset();
    EXPECT_FALSE(b.has_value());
}

struct fancy_pointer : ::testing::Test {};

TEST_F(fancy_pointer, unique_ptr) {
    static_assert(sizeof(opt::option<std::unique_ptr<int>>) == sizeof(std::unique_ptr<int>));

    opt::option<std::unique_ptr<int>> a = std::make_unique<int>(1);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(**a, 1);
    a.reset();
    EXPECT_FALSE(a.has_value());
    a.reset();
    EXPECT_FALSE(a.has_value());
    a.emplace(std::make_unique<int>(2));
    EXPECT_EQ(**a, 2);
    a.emplace(std::make_unique<int>(3));
    EXPECT_EQ(**a, 3);
}

#ifdef OPTION_HAS_BOOST_PFR

struct structures : ::testing::Test {};

TEST_F(structures, basic) {
    struct s1 { int x; float y; };

    opt::option<s1> a{1, 2.f};
    static_assert(sizeof(a) == sizeof(s1));
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(a->x, 1);
    EXPECT_EQ(a->y, 2.f);
    a.reset();
    EXPECT_FALSE(a.has_value());

    struct s2 { float x; int y; };
    opt::option<s2> b{10.5f, -1};
    static_assert(sizeof(b) == sizeof(s2));
    EXPECT_TRUE(b.has_value());
    EXPECT_EQ(b->x, 10.5f);
    EXPECT_EQ(b->y, -1);
    b.reset();
    EXPECT_FALSE(b.has_value());

    struct s3 { int x; long y; };
    const opt::option<s3> c{5, 10L};
    static_assert(sizeof(c) > sizeof(s3));

    struct s4 { float x; double y; };
    opt::option<s4> d{10.5f, 100.};
    static_assert(sizeof(d) == sizeof(s4));
    EXPECT_TRUE(d.has_value());
    EXPECT_EQ(d->x, 10.5f);
    EXPECT_EQ(d->y, 100.);
    d.reset();
    EXPECT_FALSE(d.has_value());

    struct s5 { int x; long y; float z; };
    opt::option<s5> e{10, 15L, 0.f};
    static_assert(sizeof(e) == sizeof(s5));
    EXPECT_TRUE(e.has_value());
    EXPECT_EQ(e->x, 10);
    EXPECT_EQ(e->y, 15L);
    EXPECT_EQ(e->z, 0.f);
    e.reset();
    EXPECT_FALSE(e.has_value());
}

#endif

struct struct1 {
    int a;
    float x;
};

}

template<>
struct opt::option_traits<struct1>;

static_assert(sizeof(opt::option<struct1>) > sizeof(struct1));

#endif // OPTION_USE_BUILTIN_TRAITS
