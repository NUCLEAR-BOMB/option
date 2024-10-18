
// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <doctest/doctest.h>
#include <opt/option.hpp>

namespace {

struct values {
    opt::option<int> A{1};
    opt::option<int> B{2};

    opt::option<int> E{opt::none};
};

TEST_CASE_FIXTURE(values, "==") {
    CHECK_UNARY(A == A);
    CHECK_UNARY_FALSE(A == B);

    CHECK_UNARY(E == E);
    CHECK_UNARY_FALSE(A == E);

    CHECK_UNARY(A == 1);
    CHECK_UNARY_FALSE(E == 1);
}
TEST_CASE_FIXTURE(values, "!=") {
    CHECK_UNARY(A != B);
    CHECK_UNARY_FALSE(A != A);

    CHECK_UNARY(A != E);
    CHECK_UNARY_FALSE(E != E);

    CHECK_UNARY(B != 1);
    CHECK_UNARY(E != 1);
}
TEST_CASE_FIXTURE(values, "<") {
    CHECK_UNARY(A < B);
    CHECK_UNARY_FALSE(A < A);

    CHECK_UNARY(E < A);
    CHECK_UNARY_FALSE(E < E);

    CHECK_UNARY(A < 2);
    CHECK_UNARY_FALSE(E > 1);
}
TEST_CASE_FIXTURE(values, "<=") {
    CHECK_UNARY(A <= A);
    CHECK_UNARY_FALSE(B <= A);

    CHECK_UNARY(E <= A);
    CHECK_UNARY_FALSE(A <= E);

    CHECK_UNARY(A <= 1);
    CHECK_UNARY_FALSE(1 <= E);
}
TEST_CASE_FIXTURE(values, ">") {
    CHECK_UNARY(B > A);
    CHECK_UNARY_FALSE(A > A);

    CHECK_UNARY(A > E);
    CHECK_UNARY_FALSE(E > A);

    CHECK_UNARY(B > 1);
    CHECK_UNARY_FALSE(E > 1);
}
TEST_CASE_FIXTURE(values, ">=") {
    CHECK_UNARY(A >= A);
    CHECK_UNARY_FALSE(A >= B);

    CHECK_UNARY(A >= E);
    CHECK_UNARY_FALSE(E >= A);

    CHECK_UNARY(B >= 1);
    CHECK_UNARY_FALSE(E >= 1);
}
TEST_CASE_FIXTURE(values, "|") {
    CHECK_EQ(A | 3, A);

    CHECK_EQ(A | E, A);
    CHECK_EQ(E | B, B);
    CHECK_EQ(A | B, A);
    CHECK_EQ(E | E, E);
}
TEST_CASE_FIXTURE(values, "&") {
    CHECK_EQ(A & B, B);
    CHECK_EQ(E & B, E);
    CHECK_EQ(A & E, E);
    CHECK_EQ(E & E, E);
}
TEST_CASE_FIXTURE(values, "^") {
    CHECK_EQ(A ^ B, E);
    CHECK_EQ(A ^ E, A);
    CHECK_EQ(E ^ B, B);
    CHECK_EQ(E ^ E, E);
}
TEST_CASE_FIXTURE(values, "|=") {
    opt::option<int> C{A};
    C |= B;
    CHECK_EQ(C, A);
    C |= E;
    CHECK_EQ(C, A);

    C = opt::none;
    C |= E;
    CHECK_EQ(C, E);
    C |= B;
    CHECK_EQ(C, B);

    C = opt::none;
    C |= 3;
    CHECK_EQ(C, 3);
    C |= 4;
    CHECK_EQ(C, 3);
}

}
