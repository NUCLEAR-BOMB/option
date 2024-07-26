#include <gtest/gtest.h>

#include "utils.hpp"

#include <option.hpp>

namespace {

struct lifetime : ::testing::Test {};

namespace counters {
    inline std::uint64_t default_ctor = 0;
    inline std::uint64_t value_ctor = 0;
    inline std::uint64_t destructor = 0;
    inline std::uint64_t copy_ctor = 0;
    inline std::uint64_t move_ctor = 0;
    inline std::uint64_t copy_oper = 0;
    inline std::uint64_t move_oper = 0;

    void reset() {
        default_ctor = 0;
        value_ctor = 0;
        destructor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_oper = 0;
        move_oper = 0;
    }
}

namespace expected_counters {
    inline std::uint64_t default_ctor = 0;
    inline std::uint64_t value_ctor = 0;
    inline std::uint64_t destructor = 0;
    inline std::uint64_t copy_ctor = 0;
    inline std::uint64_t move_ctor = 0;
    inline std::uint64_t copy_oper = 0;
    inline std::uint64_t move_oper = 0;

    void test() {
        EXPECT_EQ(counters::default_ctor, default_ctor);
        EXPECT_EQ(counters::value_ctor, value_ctor);
        EXPECT_EQ(counters::destructor, destructor);
        EXPECT_EQ(counters::copy_ctor, copy_ctor);
        EXPECT_EQ(counters::move_ctor, move_ctor);
        EXPECT_EQ(counters::copy_oper, copy_oper);
        EXPECT_EQ(counters::move_oper, move_oper);
    }
    void reset() {
        default_ctor = 0;
        value_ctor = 0;
        destructor = 0;
        copy_ctor = 0;
        move_ctor = 0;
        copy_oper = 0;
        move_oper = 0;
    }
}

struct lifetime_tester {
    // NOLINTBEGIN(cert-oop54-cpp, performance-noexcept-move-constructor)
    lifetime_tester() { counters::default_ctor += 1; }
    lifetime_tester(int) { counters::value_ctor += 1; }
    lifetime_tester(const lifetime_tester&) { counters::copy_ctor += 1; }
    lifetime_tester(lifetime_tester&&) { counters::move_ctor += 1; }

    ~lifetime_tester() { counters::destructor += 1; }

    lifetime_tester& operator=(const lifetime_tester&) {
        counters::copy_oper += 1;
        return *this;
    }
    lifetime_tester& operator=(lifetime_tester&&) {
        counters::move_oper += 1;
        return *this;
    }
    // NOLINTEND(cert-oop54-cpp, performance-noexcept-move-constructor)
};

TEST_F(lifetime, default_ctor) {
    using namespace expected_counters;

    counters::reset();
    expected_counters::reset();

    opt::option<lifetime_tester> tester;
    expected_counters::test();

    tester = lifetime_tester{1};
    value_ctor += 1;
    destructor += 1;
    move_ctor += 1;
    expected_counters::test();

    tester.emplace();
    default_ctor += 1;
    destructor += 1;
    expected_counters::test();

    tester.emplace(1);
    value_ctor += 1;
    destructor += 1;
    expected_counters::test();

    tester = opt::option<lifetime_tester>{};
    destructor += 1;
    expected_counters::test();

    tester = opt::option<lifetime_tester>{{}};
    default_ctor += 1;
    move_ctor += 2;
    destructor += 2;
    expected_counters::test();

    opt::option<lifetime_tester> tester2{tester.take()};
    move_ctor += 1;
    destructor += 1;
    expected_counters::test();

    [[maybe_unused]] lifetime_tester& ref = tester2.insert(lifetime_tester{1});
    value_ctor += 1;
    move_ctor += 1;
    destructor += 2;
    expected_counters::test();

    [[maybe_unused]]
    opt::option<lifetime_tester> tester3 = tester2.replace(lifetime_tester{1});
    value_ctor += 1;
    move_ctor += 2;
    destructor += 2;
    expected_counters::test();
}


}
