#include <doctest/doctest.h>
#include <opt/option.hpp>
#include <cstdint>

namespace {

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
        CHECK_EQ(counters::default_ctor, default_ctor);
        CHECK_EQ(counters::value_ctor, value_ctor);
        CHECK_EQ(counters::destructor, destructor);
        CHECK_EQ(counters::copy_ctor, copy_ctor);
        CHECK_EQ(counters::move_ctor, move_ctor);
        CHECK_EQ(counters::copy_oper, copy_oper);
        CHECK_EQ(counters::move_oper, move_oper);
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

TEST_CASE("lifetime") {
    namespace ec = expected_counters;

    counters::reset();
    ec::reset();

    opt::option<lifetime_tester> tester;
    ec::test();

    tester = lifetime_tester{1};
    ec::value_ctor += 1;
    ec::destructor += 1;
    ec::move_ctor += 1;
    ec::test();

    tester.emplace();
    ec::default_ctor += 1;
    ec::destructor += 1;
    ec::test();

    tester.emplace(1);
    ec::value_ctor += 1;
    ec::destructor += 1;
    ec::test();

    tester = opt::option<lifetime_tester>{};
    ec::destructor += 1;
    ec::test();

    tester = opt::option<lifetime_tester>{{}};
    ec::default_ctor += 1;
    ec::move_ctor += 2;
    ec::destructor += 2;
    ec::test();

    opt::option<lifetime_tester> tester2{tester.take()};
    ec::move_ctor += 1;
    ec::destructor += 1;
    ec::test();

    [[maybe_unused]] const lifetime_tester& ref = tester2.insert(lifetime_tester{1});
    ec::value_ctor += 1;
    ec::move_ctor += 1;
    ec::destructor += 2;
    ec::test();

    [[maybe_unused]]
    const opt::option<lifetime_tester> tester3 = tester2.replace(lifetime_tester{1});
    ec::value_ctor += 1;
    ec::move_ctor += 2;
    ec::destructor += 2;
    ec::test();
}

}
