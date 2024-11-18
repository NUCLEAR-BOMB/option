
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>
#include <opt/option.hpp>
#include <cstdint>
#include <utility>

namespace {

namespace counters {
    inline std::int64_t default_ctor = 0;
    inline std::int64_t value_ctor = 0;
    inline std::int64_t destructor = 0;
    inline std::int64_t copy_ctor = 0;
    inline std::int64_t move_ctor = 0;
    inline std::int64_t copy_oper = 0;
    inline std::int64_t move_oper = 0;
}

class counters_check {
    std::int64_t expected_default_ctor;
    std::int64_t expected_value_ctor;
    std::int64_t expected_destructor;
    std::int64_t expected_copy_ctor;
    std::int64_t expected_move_ctor;
    std::int64_t expected_copy_oper;
    std::int64_t expected_move_oper;

    const char* file;
    int line;
public:
    counters_check(
        const std::int64_t expected_default_ctor_,
        const std::int64_t expected_value_ctor_,
        const std::int64_t expected_destructor_,
        const std::int64_t expected_copy_ctor_,
        const std::int64_t expected_move_ctor_,
        const std::int64_t expected_copy_oper_,
        const std::int64_t expected_move_oper_,
        const char* const file_, const int line_
    ) : expected_default_ctor{expected_default_ctor_},
        expected_value_ctor{expected_value_ctor_},
        expected_destructor{expected_destructor_},
        expected_copy_ctor{expected_copy_ctor_},
        expected_move_ctor{expected_move_ctor_},
        expected_copy_oper{expected_copy_oper_},
        expected_move_oper{expected_move_oper_},
        file{file_}, line{line_}
    {
        counters::default_ctor = 0;
        counters::value_ctor = 0;
        counters::destructor = 0;
        counters::copy_ctor = 0;
        counters::move_ctor = 0;
        counters::copy_oper = 0;
        counters::move_oper = 0;
    }

    ~counters_check() {
        if (counters::default_ctor != expected_default_ctor) {
            ADD_FAIL_CHECK_AT(file, line,
                "LIFETIME_CHECK [default_ctor]" <<
                "( received: " << counters::default_ctor << ", expected: " << expected_default_ctor << " )"
            );
        }
        if (counters::value_ctor != expected_value_ctor) {
            ADD_FAIL_CHECK_AT(file, line,
                "LIFETIME_CHECK [value_ctor]" <<
                "( received: " << counters::value_ctor << ", expected: " << expected_value_ctor << " )"
            );
        }
        if (counters::destructor != expected_destructor) {
            ADD_FAIL_CHECK_AT(file, line,
                "LIFETIME_CHECK [destructor]" <<
                "( received: " << counters::destructor << ", expected: " << expected_destructor << " )"
            );
        }
        if (counters::copy_ctor != expected_copy_ctor) {
            ADD_FAIL_CHECK_AT(file, line,
                "LIFETIME_CHECK [copy_ctor]" <<
                "( received: " << counters::copy_ctor << ", expected: " << expected_copy_ctor << " )"
            );
        }
        if (counters::move_ctor != expected_move_ctor) {
            ADD_FAIL_CHECK_AT(file, line,
                "LIFETIME_CHECK [move_ctor]" <<
                "( received: " << counters::move_ctor << ", expected: " << expected_move_ctor << " )"
            );
        }
        if (counters::copy_oper != expected_copy_oper) {
            ADD_FAIL_CHECK_AT(file, line,
                "LIFETIME_CHECK [copy_oper]" <<
                "( received: " << counters::copy_oper << ", expected: " << expected_copy_oper << " )"
            );
        }
        if (counters::move_oper != expected_move_oper) {
            ADD_FAIL_CHECK_AT(file, line,
                "LIFETIME_CHECK [move_oper]" <<
                "( received: " << counters::move_oper << ", expected: " << expected_move_oper << " )"
            );
        }
        counters::default_ctor = 0;
        counters::value_ctor = 0;
        counters::destructor = 0;
        counters::copy_ctor = 0;
        counters::move_ctor = 0;
        counters::copy_oper = 0;
        counters::move_oper = 0;
    }

    operator bool() const { return true; }
};

#define LIFETIME_CHECK(default_ctor, value_ctor, destructor, copy_ctor, move_ctor, copy_oper, move_oper) \
    if (const counters_check LIFETIME_CHECK_OBJECT_{default_ctor, value_ctor, destructor, copy_ctor, move_ctor, copy_oper, move_oper, __FILE__, __LINE__})

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

TEST_SUITE_BEGIN("lifetime");

// NOLINTBEGIN(misc-const-correctness, clang-analyzer-cplusplus.Move, bugprone-use-after-move)

TEST_CASE("on opt::option") {
    LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
        opt::option<lifetime_tester> a;
    }
    LIFETIME_CHECK(0, 1, 1, 0, 0, 0, 0) {
        opt::option<lifetime_tester> a{1};
    }
    LIFETIME_CHECK(1, 0, 2, 0, 1, 0, 0) {
        opt::option<lifetime_tester> a{{}};
    }
    LIFETIME_CHECK(1, 0, 1, 0, 0, 0, 0) {
        opt::option<lifetime_tester> a{std::in_place};
    }
    opt::option<lifetime_tester> var;

    var.reset();
    LIFETIME_CHECK(0, 1, 1, 0, 1, 0, 0) {
        var = lifetime_tester{1};
    }
    var.emplace();
    LIFETIME_CHECK(0, 1, 1, 0, 0, 0, 1) {
        var = lifetime_tester{1};
    }
    var.reset();
    LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
        var = opt::option<lifetime_tester>{};
    }
    var.reset();
    LIFETIME_CHECK(0, 1, 1, 0, 1, 0, 0) {
        var = opt::option<lifetime_tester>{1};
    }
    var.emplace();
    LIFETIME_CHECK(0, 0, 1, 0, 0, 0, 0) {
        var = opt::option<lifetime_tester>{};
    }
    var.emplace();
    LIFETIME_CHECK(0, 1, 1, 0, 0, 0, 1) {
        var = opt::option<lifetime_tester>{1};
    }

    lifetime_tester val;
    var.reset();
    LIFETIME_CHECK(0, 0, 0, 1, 0, 0, 0) {
        var = val;
    }
    var.emplace();
    LIFETIME_CHECK(0, 0, 0, 0, 0, 1, 0) {
        var = val;
    }
}

// TEST_CASE(".take") {
//     opt::option<lifetime_tester> var0;
//     opt::option<lifetime_tester> var1;
// 
//     var0.reset();
//     var1.reset();
//     LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
//         var1 = var0.take();
//     }
//     var0.emplace();
//     LIFETIME_CHECK(0, 0, 2, 0, 1, 0, 0) {
//         (void)var0.take();
//     }
//     var0.emplace();
//     var1.reset();
//     LIFETIME_CHECK(0, 0, 2, 0, 2, 0, 0) {
//         var1 = var0.take();
//     }
//     var0.reset();
//     var1.emplace();
//     LIFETIME_CHECK(0, 0, 1, 0, 0, 0, 0) {
//         var1 = var0.take();
//     }
//     var0.emplace();
//     var1.emplace();
//     LIFETIME_CHECK(0, 0, 2, 0, 1, 0, 1) {
//         var1 = var0.take();
//     }
// 
//     // var0.reset();
//     // LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
//     //     (void)std::move(var0).take();
//     // }
//     // var0.emplace();
//     // LIFETIME_CHECK(0, 0, 1, 0, 1, 0, 0) {
//     //     (void)std::move(var0).take();
//     // }
//     // var0.reset();
//     // var1.reset();
//     // LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
//     //     var1 = std::move(var0).take();
//     // }
//     // var0.reset();
//     // var1.emplace();
//     // LIFETIME_CHECK(0, 0, 1, 0, 0, 0, 0) {
//     //     var1 = std::move(var0).take();
//     // }
// }

TEST_CASE(".reset") {
    opt::option<lifetime_tester> var;

    var.reset();
    LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
        var.reset();
    }
    var.emplace();
    LIFETIME_CHECK(0, 0, 1, 0, 0, 0, 0) {
        var.reset();
    }
}

TEST_CASE(".emplace") {
    opt::option<lifetime_tester> var;

    var.reset();
    LIFETIME_CHECK(1, 0, 0, 0, 0, 0, 0) {
        var.emplace();
    }
    var.reset();
    LIFETIME_CHECK(0, 1, 0, 0, 0, 0, 0) {
        var.emplace(1);
    }
    lifetime_tester val;

    var.reset();
    LIFETIME_CHECK(0, 0, 0, 1, 0, 0, 0) {
        var.emplace(val);
    }
    var.reset();
    LIFETIME_CHECK(0, 0, 0, 0, 1, 0, 0) {
        var.emplace(std::move(val));
    }
    var.emplace();
    LIFETIME_CHECK(0, 0, 1, 1, 0, 0, 0) {
        var.emplace(val);
    }
    var.emplace();
    LIFETIME_CHECK(0, 0, 1, 0, 1, 0, 0) {
        var.emplace(std::move(val));
    }
}

TEST_CASE("opt::make_option") {
    LIFETIME_CHECK(1, 0, 1, 0, 0, 0, 0) {
        opt::option<lifetime_tester> a = opt::make_option<lifetime_tester>();
    }
}

// TEST_CASE("std::exchange") {
//     opt::option<lifetime_tester> var0;
// 
//     var0.emplace();
//     LIFETIME_CHECK(0, 1, 2, 0, 1, 0, 1) {
//         (void)std::exchange(var0, 1);
//     }
//     var0.reset();
//     LIFETIME_CHECK(0, 1, 0, 0, 0, 0, 0) {
//         (void)std::exchange(var0, 1);
//     }
// 
//     opt::option<lifetime_tester> var1;
//     var0.reset();
//     var1.reset();
//     LIFETIME_CHECK(0, 1, 0, 0, 0, 0, 0) {
//         var1 = std::exchange(var0, 1);
//     }
//     var0.reset();
//     var1.emplace();
//     LIFETIME_CHECK(0, 1, 1, 0, 0, 0, 0) {
//         var1 = std::exchange(var0, 1);
//     }
//     var0.emplace();
//     var1.reset();
//     LIFETIME_CHECK(0, 1, 2, 0, 2, 0, 1) {
//         var1 = std::exchange(var0, 1);
//     }
//     var0.emplace();
//     var1.emplace();
//     LIFETIME_CHECK(0, 1, 2, 0, 1, 0, 2) {
//         var1 = std::exchange(var0, 1);
//     }
// }

TEST_CASE(".map") {
    opt::option<int> ivar;

    ivar.emplace(1);
    LIFETIME_CHECK(0, 1, 1, 0, 0, 0, 0) {
        (void)ivar.map([](int x) { return lifetime_tester{x}; });
    }
    ivar.reset();
    LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
        (void)ivar.map([](int) { return lifetime_tester{}; });
    }
    opt::option<lifetime_tester> var0;
    const auto fn = [](const lifetime_tester&) { return 1; };

    var0.reset();
    LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
        (void)var0.map(fn);
    }
    var0.emplace();
    LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
        (void)var0.map(fn);
    }
    opt::option<lifetime_tester> var1;

    var0.reset();
    var1.reset();
    LIFETIME_CHECK(0, 0, 0, 0, 0, 0, 0) {
        var1 = var0.map(fn);
    }
    var0.reset();
    var1.emplace();
    LIFETIME_CHECK(0, 0, 1, 0, 0, 0, 0) {
        var1 = var0.map(fn);
    }
    var0.emplace();
    var1.reset();
    LIFETIME_CHECK(0, 1, 0, 0, 0, 0, 0) {
        var1 = var0.map(fn);
    }
    var0.emplace();
    var1.emplace();
    LIFETIME_CHECK(0, 1, 1, 0, 0, 0, 1) {
        var1 = var0.map(fn);
    }
}

// NOLINTEND(misc-const-correctness, clang-analyzer-cplusplus.Move, bugprone-use-after-move)

TEST_SUITE_END();

}
