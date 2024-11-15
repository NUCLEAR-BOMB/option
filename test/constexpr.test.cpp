
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <opt/option.hpp>
#include <doctest/doctest.h>

#include "utils.hpp"

#if OPTION_CLANG
    #pragma clang diagnostic ignored "-Wunused-variable"
#elif OPTION_GCC
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#elif OPTION_MSVC
    #pragma warning(disable : 4189) // 'identifier' : local variable is initialized but not referenced
#endif

// NOLINTBEGIN(misc-const-correctness)

namespace {

TEST_SUITE_BEGIN("constexpr");

TEST_CASE("constructors") {
    static_assert(([] {
        opt::option<int> a;
    }(), true));
    static_assert(([] {
        opt::option<int> a{opt::none};
    }(), true));
    static_assert(([] {
        const opt::option<int> a;
        opt::option<int> b{a};
    }(), true));
    static_assert(([] {
        const opt::option<int> a{1};
        opt::option<int> b{a};
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
    }(), true));
    static_assert(([] {
        opt::option<int> a{std::in_place, 1};
    }(), true));
    static_assert(([] {
        opt::option<int> a{};
        opt::option<long long> b{a};
    }(), true));
#if OPTION_IS_CXX20
    static_assert(([] {
        opt::option<int> a{1};
        opt::option<long long> b{a};
    }(), true));
#endif
}

TEST_CASE("operator=") {
    // operator=(opt::none_t)
    static_assert(([] {
        opt::option<int> a;
        a = opt::none;
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        a = opt::none;
    }(), true));
    // operator=(const option&)
    static_assert(([] {
        opt::option<int> a;
        const opt::option<int> b;
        a = b;
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        const opt::option<int> b;
        a = b;
    }(), true));
    static_assert(([] {
        opt::option<int> a;
        const opt::option<int> b{1};
        a = b;
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        const opt::option<int> b{1};
        a = b;
    }(), true));
    // operator=(option&&)
    static_assert(([] {
        opt::option<int> a;
        a = opt::option<int>{};
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        a = opt::option<int>{};
    }(), true));
    static_assert(([] {
        opt::option<int> a;
        a = opt::option<int>{1};
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        a = opt::option<int>{1};
    }(), true));
    // operator=(U&&)
#if OPTION_IS_CXX20
    static_assert(([] {
        opt::option<int> a;
        a = 1;
    }(), true));
#endif
    static_assert(([] {
        opt::option<int> a{1};
        a = 1;
    }(), true));
    // operator=(const option<U>&)
    static_assert(([] {
        opt::option<long long> a;
        const opt::option<int> b;
        a = b;
    }(), true));
#if OPTION_IS_CXX20
    static_assert(([] {
        opt::option<long long> a;
        const opt::option<int> b{1};
        a = b;
    }(), true));
#endif
    static_assert(([] {
        opt::option<long long> a{1};
        const opt::option<int> b;
        a = b;
    }(), true));
    static_assert(([] {
        opt::option<long long> a{1};
        const opt::option<int> b{1};
        a = b;
    }(), true));
    // operator=(option<U>&&)
    static_assert(([] {
        opt::option<long long> a;
        a = opt::option<int>{};
    }(), true));
#if OPTION_IS_CXX20
    static_assert(([] {
        opt::option<long long> a;
        a = opt::option<int>{1};
    }(), true));
#endif
    static_assert(([] {
        opt::option<long long> a{1};
        a = opt::option<int>{};
    }(), true));
    static_assert(([] {
        opt::option<long long> a{1};
        a = opt::option<int>{1};
    }(), true));
}

TEST_CASE("operator*") {
    static_assert(([] {
        opt::option<int> a{1};
        int b = *a;
    }(), true));
}

TEST_CASE("has_value") {
    static_assert(([] {
        opt::option<int> a;
        bool b = a.has_value();
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        bool b = a.has_value();
    }(), true));
}

TEST_CASE("reset") {
    static_assert(([] {
        opt::option<int> a;
        a.reset();
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        a.reset();
    }(), true));
}

TEST_CASE("emplace") {
#if OPTION_IS_CXX20
    static_assert(([] {
        opt::option<int> a;
        a.emplace(1);
    }(), true));
#endif
    static_assert(([] {
        opt::option<int> a{1};
        a.emplace(1);
    }(), true));
}

TEST_CASE("map") {
    static_assert(([] {
        opt::option<int> a;
        const opt::option<long long> b = a.map([](int x) -> long long { return x + 1; });
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        const opt::option<long long> b = a.map([](int x) -> long long { return x + 1; });
    }(), true));
}

TEST_CASE("inspect") {
    static_assert(([] {
        opt::option<int> a;
        a.inspect([](int) {});
    }(), true));
    static_assert(([] {
        opt::option<int> a{1};
        a.inspect([](int) {});
    }(), true));
}

TEST_SUITE_END();

}

// NOLINTEND(misc-const-correctness)
