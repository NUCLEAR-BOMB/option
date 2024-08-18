#include <doctest/doctest.h>
#include <memory>
#include <type_traits>
#include <opt/option.hpp>
#include <utility>
#include <tuple>
#include <array>
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

#define TEST_SIZE_LIST \
    struct_with_sentinel, \
    int(*)(int), std::string_view, polymorphic_type, empty_polymorphic_type, aggregate_with_empty_struct, \
    aggregate_int_float, std::array<int, 0>, \
    empty_struct, std::tuple<>, std::tuple<int, float, int>, \
    double, bool, std::reference_wrapper<int>, int*, float, \
    std::pair<int, float>, std::pair<float, int>, std::array<float, 4>, int
    
namespace {

struct trivial_struct {};
static_assert(std::is_trivially_destructible_v<opt::option<trivial_struct>>);

static_assert(is_not_trivial_compatible<opt::option<nontrivial_struct>>);
static_assert(is_trivial_compatible<opt::option<int>>);

static_assert(std::is_nothrow_destructible_v<opt::option<int>>);
static_assert(!std::is_nothrow_destructible_v<opt::option<nontrivial_struct>>);

template<class T>
struct sample_values;

template<> struct sample_values<int> {
    int values[5]{1, 2, 3, 4, 5};
};

template<> struct sample_values<float> {
    float values[5]{1.f, 2.f, 3.f, 4.f, 5.f};

    fp_exception_checker checker{};
};
template<> struct sample_values<double> {
    double values[5]{1., 2., 3., 4., 5.};

    fp_exception_checker checker{};
};
template<> struct sample_values<bool> {
    bool values[5]{false, true, false, true, false};
};
template<> struct sample_values<std::reference_wrapper<int>> {
    int values_orig[5]{1, 2, 3, 4, 5};
    std::reference_wrapper<int> values[5]{values_orig[0], values_orig[1], values_orig[2], values_orig[3], values_orig[4]};
};
template<> struct sample_values<int*> {
    int values_orig[5]{1, 2, 3, 4, 5};
    int* values[5]{&values_orig[0], &values_orig[1], &values_orig[2], &values_orig[3], &values_orig[4]};
};
template<> struct sample_values<std::pair<int, float>> {
    std::pair<int, float> values[5]{{1, 2.f}, {3, 4.f}, {5, 6.f}, {7, 8.f}, {9, 10.f}};
};
template<> struct sample_values<std::pair<float, int>> {
    std::pair<float, int> values[5]{{1.f, 2}, {3.f, 4}, {5.f, 6}, {7.f, 8}, {9.f, 10}};
};
template<> struct sample_values<std::array<float, 4>> {
    std::array<float, 4> values[5]{{1.f, 2.f, 3.f, 4.f}, {5.f, 6.f, 7.f, 8.f}, {9.f, 10.f, 11.f, 12.f}, {13.f, 14.f, 15.f, 16.f}, {17.f, 18.f, 19.f, 20.f}};
};
template<> struct sample_values<std::array<int, 0>> {
    std::array<int, 0> values[5]{{}, {}, {}, {}, {}};
};

struct empty_struct {
    bool operator==(const empty_struct&) const { return true; }
};
template<> struct sample_values<empty_struct> {
    empty_struct values[5]{{}, {}, {}, {}, {}};
};

template<> struct sample_values<std::tuple<>> {
    std::tuple<> values[5]{{}, {}, {}, {}, {}};
};
template<> struct sample_values<std::tuple<int, float, int>> {
    std::tuple<int, float, int> values[5]{{1, 2.f, 3}, {4, 5.f, 6}, {7, 8.f, 9}, {10, 11.f, 12}, {13, 14.f, 15}};
};

struct aggregate_int_float {
    int x;
    float y;

    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    bool operator==(const aggregate_int_float& a) const { return x == a.x && y == a.y; }
};
template<> struct sample_values<aggregate_int_float> {
    aggregate_int_float values[5]{{1, 2.f}, {3, 4.f}, {5, 6.f}, {7, 8.f}, {9, 10.f}};
};

struct aggregate_with_empty_struct {
    int x;
    empty_struct y;

    bool operator==(const aggregate_with_empty_struct& a) const { return x == a.x; }
};
template<> struct sample_values<aggregate_with_empty_struct> {
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

template<> struct sample_values<empty_polymorphic_type> {
    empty_polymorphic_type values[5]{{}, {}, {}, {}, {}};
};

struct polymorphic_type {
    int x;

    polymorphic_type(int x_) : x(x_) {}

    polymorphic_type() = default;
    polymorphic_type(const polymorphic_type&) = default; // NOLINT(clang-analyzer-core.uninitialized.Assign)
    polymorphic_type& operator=(const polymorphic_type&) = default; // NOLINT(clang-analyzer-core.uninitialized.Assign)

    virtual ~polymorphic_type() = default;

    bool operator==(const polymorphic_type& other) const { return x == other.x; }
};
static_assert(std::is_polymorphic_v<polymorphic_type>);

template<> struct sample_values<polymorphic_type> {
    polymorphic_type values[5]{{1}, {2}, {3}, {4}, {5}};
};

template<> struct sample_values<std::string_view> {
    std::string_view values[5]{"a1", "b2", "c3", "d4", "e5"};
};

template<> struct sample_values<int(*)(int)> {
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
template<> struct sample_values<struct_with_sentinel> {
    struct_with_sentinel values[5]{struct_with_sentinel{1}, struct_with_sentinel{2}, struct_with_sentinel{3}, struct_with_sentinel{4}, struct_with_sentinel{5}};
};

TEST_CASE_TEMPLATE("opt::option<T>", T, TEST_SIZE_LIST) {
    const sample_values<T> sample;
    // Allow captured structured bindings in lambda
    const auto& v0 = sample.values[0];
    const auto& v1 = sample.values[1];
    const auto& v2 = sample.values[2];
    const auto& v3 = sample.values[3];
    const auto& v4 = sample.values[4];

    SUBCASE("constructor") {
        const opt::option<T> a;
        CHECK_UNARY_FALSE(a.has_value());

        const opt::option<T> b(opt::none);
        CHECK_UNARY_FALSE(b.has_value());

        const opt::option<T> c = opt::none;
        CHECK_UNARY_FALSE(c.has_value());

        if constexpr (std::is_default_constructible_v<T>) {
            const opt::option<T> d{{}};
            CHECK_EQ(d, T{});
        }
    }
    SUBCASE(".get") {
        opt::option<T> a{v0};
        CHECK_EQ(a.get(), v0);
        CHECK_EQ(std::as_const(a).get(), v0);
        CHECK_EQ(as_const_rvalue(a).get(), v0);
        CHECK_EQ(as_rvalue(a).get(), v0);
    }
    SUBCASE("operator=") {
        opt::option<T> a = v0;
        a = opt::none;
        CHECK_UNARY_FALSE(a.has_value());
        {
            const opt::option tmp{v1};
            a = tmp;
        }
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, v1);
        {
            const opt::option<T> tmp{opt::none};
            a = tmp;
        }
        CHECK_UNARY_FALSE(a.has_value());
        a = opt::option<T>(v2);
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, v2);
        a = opt::option<T>(opt::none);
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_UNARY_FALSE(a.has_value());
        {
            const T tmp = v3;
            a = tmp;
        }
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, v3);
        a = v4;
        CHECK_UNARY(a.has_value());
        CHECK_EQ(a, v4);

        opt::option<T> b;
        CHECK_UNARY_FALSE(b.has_value());
        b = v0;
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        b = v0;
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        b = v1;
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v1);
        b = opt::option<T>{};
        CHECK_UNARY_FALSE(b.has_value());
        b = opt::option<T>{};
        CHECK_UNARY_FALSE(b.has_value());
        b = opt::option<T>{v0};
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        b = opt::none;
        CHECK_UNARY_FALSE(b.has_value());
        b = opt::none;
        CHECK_UNARY_FALSE(b.has_value());

        opt::option<T> c;
        CHECK_UNARY_FALSE(c.has_value());
        c = v0;
        CHECK_UNARY(c.has_value());
        CHECK_EQ(c, v0);

        opt::option<T> d;
        CHECK_UNARY_FALSE(d.has_value());
        d = c;
        CHECK_UNARY(c.has_value());
        CHECK_UNARY(d.has_value());
        CHECK_EQ(d, v0);

        c = opt::none;
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_UNARY(d.has_value());
        CHECK_EQ(d, v0);
        d = opt::none;

        c = d;
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_UNARY_FALSE(d.has_value());

        c = opt::option<T>{d};
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_UNARY_FALSE(d.has_value());

        d = v0;
        CHECK_UNARY(d.has_value());
        CHECK_EQ(d, v0);

        c = opt::option<T>{d};
        CHECK_UNARY(c.has_value());
        CHECK_UNARY(d.has_value());
        CHECK_EQ(d, v0);
        CHECK_EQ(c, v0);

        d = opt::option<T>{c};
        CHECK_UNARY(c.has_value());
        CHECK_UNARY(d.has_value());

        d = opt::none;
        CHECK_UNARY_FALSE(d.has_value());
        c = opt::option<T>{d};
        CHECK_UNARY_FALSE(c.has_value());
        CHECK_UNARY_FALSE(d.has_value());

        c = v0;
        CHECK_UNARY(c.has_value());
        d = std::move(c);
        CHECK_UNARY(d.has_value());
        CHECK_UNARY(c.has_value()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)

        d = opt::none;
        c = v0;
        CHECK_UNARY_FALSE(d.has_value());
        CHECK_UNARY(c.has_value());
        d = opt::option<T>{std::move(c)};
        CHECK_UNARY(d.has_value());
        CHECK_UNARY(c.has_value()); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
    }
    SUBCASE(".reset") {
        opt::option<T> a;
        CHECK_UNARY_FALSE(a.has_value());
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
        a = v0;
        CHECK_UNARY(a.has_value());
        a.reset();
        CHECK_UNARY_FALSE(a.has_value());
    }
    SUBCASE(".emplace") {
        opt::option<T> a{v0};
        CHECK_EQ(a, v0);
        a.emplace(v0);
        CHECK_EQ(a, v0);
        a.reset();
        CHECK_EQ(a, opt::none);
        a.emplace(v1);
        CHECK_EQ(a, v1);
        a.emplace(v0);
        CHECK_EQ(a, v0);
    }
    SUBCASE("std::hash") {
        if constexpr (is_hashable<T>) {
            opt::option<T> a{v0};
            opt::option<T> b{v0};
            CHECK_EQ(hash_fn(a), hash_fn(b));
            a = opt::none;
            CHECK_NE(hash_fn(a), hash_fn(b));
            b = opt::none;
            CHECK_EQ(hash_fn(a), hash_fn(b));
        }
    }
    SUBCASE(".value_or_throw") {
        opt::option<T> a{v0};
        CHECK_NOTHROW((void)a.value_or_throw());
        CHECK_NOTHROW((void)a.value());
        CHECK_NOTHROW((void)as_const(a).value());
        CHECK_NOTHROW((void)as_const_rvalue(a).value());
        a = opt::none;
        CHECK_THROWS_AS((void)a.value_or_throw(), opt::bad_access);
        CHECK_THROWS_AS((void)a.value(), opt::bad_access);
        CHECK_THROWS_AS((void)as_const(a).value(), opt::bad_access);
        CHECK_THROWS_AS((void)as_const_rvalue(a).value(), opt::bad_access);
    }
    SUBCASE(".value_or") {
        opt::option<T> a; // NOLINT(clang-analyzer-core.uninitialized.UndefReturn)
        CHECK_EQ(a.value_or(v0), v0); // NOLINT(clang-analyzer-core.UndefinedBinaryOperatorResult, clang-analyzer-core.uninitialized.Assign)
        a = v1;
        CHECK_EQ(a.value_or(v2), v1);
        CHECK_EQ(as_rvalue(a).value_or(v3), v1);
    }
    SUBCASE(".and_then") {
        if constexpr (std::is_same_v<T, int>) {
            const auto convert_to_uint = [](int x) -> opt::option<unsigned> {
                if (x >= 0) { return opt::option<unsigned>{unsigned(x + 1)}; }
                return opt::none;
            };
            CHECK_EQ(opt::option<int>{2}.and_then(convert_to_uint), 3u);
            CHECK_EQ(opt::option<int>{-10}.and_then(convert_to_uint), opt::none);
            CHECK_EQ(opt::option<int>{opt::none}.and_then(convert_to_uint), opt::none);
        }
    }
    SUBCASE(".map") {
        if constexpr (std::is_same_v<T, int>) {
            const auto func = [](auto x) { return x - 1; };
            CHECK_EQ(opt::option<int>{1}.map(func), 0);
            CHECK_EQ(opt::option<int>{}.map(func), opt::none);
            CHECK_EQ(opt::option<int>{10}.map(func).map(func), 8);
        }
    }
    SUBCASE(".or_else") {
        if constexpr (std::is_same_v<T, int>) {
            const auto func = []() { return opt::option{1 << 10}; };
            CHECK_EQ(opt::option<int>{1}.or_else(func), 1);
            CHECK_EQ(opt::option<int>{}.or_else(func), 1 << 10);
            CHECK_EQ(opt::option<int>{}.or_else(func).or_else(func), 1 << 10);
        }
    }
    SUBCASE(".take") {
        opt::option<T> a;
        opt::option<T> b = a.take();
        CHECK_UNARY_FALSE(a.has_value());
        CHECK_UNARY_FALSE(a.has_value());
        a = v0;
        CHECK_UNARY_FALSE(b.has_value());
        CHECK_UNARY(a.has_value());
        b = a.take();
        CHECK_UNARY(b.has_value());
        CHECK_EQ(b, v0);
        CHECK_UNARY_FALSE(a.has_value());
        a = b.take();
        CHECK_UNARY(a.has_value());
        CHECK_UNARY_FALSE(b.has_value());
        CHECK_EQ(a, v0);
        a = v1;
        CHECK_UNARY(a.has_value());
        CHECK_EQ(*a, v1);
        a.take();
        CHECK_UNARY_FALSE(a.has_value());
    }
    SUBCASE("opt::option_cast") {
        if constexpr (std::is_same_v<T, int>) {
            opt::option<int> a{1};
            opt::option<unsigned> b = opt::option_cast<unsigned>(a);
            CHECK_EQ(*b, 1u);
            b = opt::option_cast<unsigned>(as_rvalue(a));
            CHECK_EQ(*b, 1u);
        }
    }
    SUBCASE("deduction guides") {
        auto a = opt::option{v0}; // NOLINT(misc-const-correctness)
        CHECK_UNARY(std::is_same_v<decltype(a), opt::option<T>>);
        opt::option b{v1}; // NOLINT(misc-const-correctness)
        CHECK_UNARY(std::is_same_v<decltype(b), opt::option<T>>);

        auto c = opt::option{opt::option{v0}};
        CHECK_UNARY(std::is_same_v<decltype(c), opt::option<opt::option<T>>>);

        auto d = opt::option{opt::option{opt::option{v1}}};
        CHECK_UNARY(std::is_same_v<decltype(d), opt::option<opt::option<opt::option<T>>>>);
    }
    SUBCASE(".value_or_default") {
        if constexpr (std::is_default_constructible_v<T>) {
            opt::option a{v0};
            CHECK_EQ(a.value_or_default(), v0);
            a = opt::none;
            CHECK_EQ(a.value_or_default(), T{});
        }
    }
    SUBCASE(".ptr_or_null") {
        opt::option a{v0};

        CHECK_EQ(*(a.ptr_or_null()), v0);
        a = opt::none;
        CHECK_EQ(a.ptr_or_null(), nullptr);
        a = v1;
        CHECK_EQ(*(as_const(a).ptr_or_null()), v1);
    }
    SUBCASE(".filter") {
        if constexpr (std::is_same_v<T, int>) {
            const auto is_even = [](int x) { return x % 2 == 0; };

            opt::option a{1};
            CHECK_EQ(a.filter(is_even), opt::none);
            a = 2;
            CHECK_EQ(a.filter(is_even), 2);
            a = opt::none;
            CHECK_EQ(a.filter(is_even), opt::none);
        }
    }
    SUBCASE(".flatten") {
        auto a = opt::option{opt::option{v0}};
        CHECK_EQ(**a, v0);
        auto b = a.flatten();
        CHECK_EQ(*b, v0);

        a = opt::option{opt::option<T>{opt::none}};
        b = a.flatten();
        CHECK_UNARY_FALSE(b.has_value());
        a = opt::option<T>{opt::none};
        b = a.flatten();
        CHECK_UNARY_FALSE(b.has_value());
        a = opt::none;
        b = a.flatten();
        CHECK_UNARY_FALSE(b.has_value());
    }
    SUBCASE(".map_or") {
        if constexpr (std::is_same_v<T, int>) {
            const auto add_one = [](int x) { return x + 1; };
            opt::option a{1};
            CHECK_EQ(a.map_or(10, add_one), 2);
            a = opt::none;
            CHECK_EQ(a.map_or(11, add_one), 11);

            const auto add_two = [](int x) { return float(x) + 2.f; };
            a = 2;
            CHECK_EQ(a.map_or(0.f, add_two), 4.f);
            a = opt::none;
            CHECK_EQ(a.map_or(5.f, add_two), 5.f);
        }
    }
    SUBCASE(".map_or_else") {
        if constexpr (std::is_same_v<T, int>) {
            const auto default_fn = []() { return 2; };
            const auto do_fn = [](int x) { return x - 1; };

            opt::option a{1};
            CHECK_EQ(a.map_or_else(default_fn, do_fn), 0);
            a = opt::none;
            CHECK_EQ(a.map_or_else(default_fn, do_fn), 2);
        }
    }
    SUBCASE(".take_if") {
        if constexpr (std::is_same_v<T, int>) {
            opt::option a{1};
            auto b = a.take_if([](int) { return false; });
            CHECK_UNARY_FALSE(b.has_value());
            CHECK_UNARY(a.has_value());

            b = a.take_if([](int& x) {
                return ++x == 2;
            });
            CHECK_UNARY(b.has_value());
            CHECK_EQ(*b, 2);
            CHECK_UNARY_FALSE(a.has_value());

            auto c = a.take_if([](int) { return false; });
            CHECK_UNARY_FALSE(c.has_value());
            CHECK_UNARY_FALSE(a.has_value());

            c = a.take_if([](int) { return true; });
            CHECK_UNARY_FALSE(c.has_value());
            CHECK_UNARY_FALSE(a.has_value());
        }
    }
    SUBCASE(".has_value_and") {
        opt::option a{v0};
        CHECK_UNARY(a.has_value_and([&](const T& x) { return x == v0; }));
        if (!(v0 == v1)) {
            CHECK_UNARY_FALSE(a.has_value_and([&](const T& x) { return x == v1; }));
        }
        a.reset();
        CHECK_UNARY_FALSE(a.has_value_and([&](const T& x) { return x == v0; }));
        CHECK_UNARY_FALSE(a.has_value_and([&](const T& x) { return x == v1; }));
    }
    SUBCASE(".insert") {
        opt::option a{v0};
        const T& x = a.insert(v1);
        CHECK_EQ(&x, a.ptr_or_null());
        CHECK_EQ(*a, v1);

        const T tmp = v2;
        const T& y = a.insert(tmp);

        CHECK_EQ(&y, a.ptr_or_null());
        CHECK_EQ(*a, v2);
    }
    SUBCASE(".inspect") {
        if constexpr (std::is_same_v<T, int>) {
            opt::option a{1};
            int x = 0;
            a.inspect([&](int y) { x += y; });
            CHECK_EQ(x, 1);
            opt::option<int>{a}
                .inspect([&](const int& y) { x += y; })
                .inspect([&](int& y) { return x += y * 2; });
            CHECK_EQ(x, 4);
        }
    }
    SUBCASE(".assume_has_value") {
        opt::option a{v0};
        a.assume_has_value();
        CHECK_EQ(*a, v0);
    }
    SUBCASE(".unzip") {
        SUBCASE("std::tuple") {
            opt::option a{std::tuple{v0, v1, v2, v3}};
            CHECK_UNARY(a.has_value());

            auto b = a.unzip();
            CHECK_UNARY(std::is_same_v<decltype(b), std::tuple<
                opt::option<T>, opt::option<T>, opt::option<T>, opt::option<T>
            >>);
            auto& [b1, b2, b3, b4] = b;

            CHECK_UNARY(b1.has_value());
            CHECK_EQ(*b1, v0);
            CHECK_UNARY(b2.has_value());
            CHECK_EQ(*b2, v1);
            CHECK_UNARY(b3.has_value());
            CHECK_EQ(*b3, v2);
            CHECK_UNARY(b4.has_value());
            CHECK_EQ(*b4, v3);

            a.reset();
            auto c = a.unzip();
            auto& [c1, c2, c3, c4] = c;
            CHECK_UNARY_FALSE(c1.has_value());
            CHECK_UNARY_FALSE(c2.has_value());
            CHECK_UNARY_FALSE(c3.has_value());
            CHECK_UNARY_FALSE(c4.has_value());
        }
        SUBCASE("std::pair") {
            opt::option a{std::pair{v0, v1}};
            CHECK_UNARY(a.has_value());

            auto b = a.unzip();
            CHECK_UNARY(std::is_same_v<decltype(b), std::pair<opt::option<T>, opt::option<T>>>);
            auto& [b1, b2] = b;

            CHECK_UNARY(b1.has_value());
            CHECK_EQ(*b1, v0);
            CHECK_UNARY(b2.has_value());
            CHECK_EQ(*b2, v1);

            a.reset();
            auto c = a.unzip();
            auto& [c1, c2] = c;
            CHECK_UNARY_FALSE(c1.has_value());
            CHECK_UNARY_FALSE(c2.has_value());
        }
        SUBCASE("std::array") {
            opt::option a{std::array{v0, v1, v2}};
            CHECK_UNARY(a.has_value());

            auto b = a.unzip();
            CHECK_UNARY(std::is_same_v<decltype(b), std::array<opt::option<T>, 3>>);
            auto& [b1, b2, b3] = b;

            CHECK_UNARY(b1.has_value());
            CHECK_EQ(*b1, v0);
            CHECK_UNARY(b2.has_value());
            CHECK_EQ(*b2, v1);
            CHECK_UNARY(b3.has_value());
            CHECK_EQ(*b3, v2);

            a.reset();
            auto c = a.unzip();
            auto& [c1, c2, c3] = c;
            CHECK_UNARY_FALSE(c1.has_value());
            CHECK_UNARY_FALSE(c2.has_value());
            CHECK_UNARY_FALSE(c3.has_value());
        }
    }
    SUBCASE("opt::zip") {
        opt::option<T> a{v0};
        opt::option<T> b{v1};

        auto c = opt::zip(a, b);
        CHECK_UNARY(c.has_value());

        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93147
        CHECK_EQ(std::tuple_size_v<typename decltype(c)::value_type>, 2);
        if constexpr (std::tuple_size_v<typename decltype(c)::value_type> == 2) {
            CHECK_EQ(std::get<0>(*c), v0);
            CHECK_EQ(std::get<1>(*c), v1);
        }

        a.reset();
        c = opt::zip(a, b);
        CHECK_UNARY_FALSE(c.has_value());

        b.reset();
        c = opt::zip(a, b);
        CHECK_UNARY_FALSE(c.has_value());
    }
    SUBCASE("opt::zip_with") {
        if constexpr (std::is_same_v<T, int>) {
            struct point { float x, y; };
            const auto construct_point = [](float x, float y) {
                return point{x, y};
            };

            const opt::option<int> a{1};
            const opt::option<float> b{2.f};

            auto c = opt::zip_with(construct_point, opt::option_cast<float>(a), b);

            CHECK_UNARY(c.has_value());
            CHECK_EQ(c->x, 1.f);
            CHECK_EQ(c->y, 2.f);

            c = opt::zip_with(construct_point, opt::option<float>{}, b);
            CHECK_UNARY_FALSE(c.has_value());

            c = opt::zip_with(construct_point, opt::option<float>{}, opt::option<float>{});
            CHECK_UNARY_FALSE(c.has_value());

            const auto do_something_else = [](int x, float y) {
                (void)x;
                (void)y;
            };
            opt::zip_with(do_something_else, a, b);
            opt::zip_with(do_something_else, opt::option<int>{}, b);
            opt::zip_with(do_something_else, a, opt::option<float>{});
        }
    }
    SUBCASE(".replace") {
        opt::option a{std::make_unique<T>(v0)};

        auto c = a.replace(std::make_unique<T>(v1));
        CHECK_UNARY(c.has_value());
        CHECK_EQ(**c, v0);
        CHECK_UNARY(a.has_value());
        CHECK_EQ(**a, v1);
    }
    SUBCASE("opt::from_nullable") {
        T a = v0;
        T* ptr = &a;

        const opt::option<T> b = opt::from_nullable(ptr);
        CHECK_EQ(b, v0);

        ptr = nullptr;
        const opt::option<T> c = opt::from_nullable(ptr);
        CHECK_UNARY_FALSE(c.has_value());
    }
}

}
