#include <gtest/gtest.h>

#include <option.hpp>

namespace {

struct operators : ::testing::Test {
    opt::option<int> A{1};
    opt::option<int> B{2};

    opt::option<int> E{opt::none};
};

TEST_F(operators, equal) {
    EXPECT_TRUE(A == A);
    EXPECT_FALSE(A == B);

    EXPECT_TRUE(E == E);
    EXPECT_FALSE(A == E);

    EXPECT_TRUE(A == 1);
    EXPECT_FALSE(E == 1);
}
TEST_F(operators, not_equal) {
    EXPECT_TRUE(A != B);
    EXPECT_FALSE(A != A);

    EXPECT_TRUE(A != E);
    EXPECT_FALSE(E != E);

    EXPECT_TRUE(B != 1);
    EXPECT_TRUE(E != 1);
}
TEST_F(operators, less) {
    EXPECT_TRUE(A < B);
    EXPECT_FALSE(A < A);

    EXPECT_TRUE(E < A);
    EXPECT_FALSE(E < E);

    EXPECT_TRUE(A < 2);
    EXPECT_FALSE(E > 1);
}
TEST_F(operators, less_equal) {
    EXPECT_TRUE(A <= A);
    EXPECT_FALSE(B <= A);

    EXPECT_TRUE(E <= A);
    EXPECT_FALSE(A <= E);

    EXPECT_TRUE(A <= 1);
    EXPECT_FALSE(1 <= E);
}
TEST_F(operators, greater) {
    EXPECT_TRUE(B > A);
    EXPECT_FALSE(A > A);

    EXPECT_TRUE(A > E);
    EXPECT_FALSE(E > A);

    EXPECT_TRUE(B > 1);
    EXPECT_FALSE(E > 1);
}
TEST_F(operators, greater_equal) {
    EXPECT_TRUE(A >= A);
    EXPECT_FALSE(A >= B);

    EXPECT_TRUE(A >= E);
    EXPECT_FALSE(E >= A);

    EXPECT_TRUE(B >= 1);
    EXPECT_FALSE(E >= 1);
}
TEST_F(operators, or) {
    EXPECT_EQ(A | 3, A);
    EXPECT_EQ(A | opt::none, A);
    EXPECT_EQ(opt::none | A, A);

    EXPECT_EQ(A | E, A);
    EXPECT_EQ(E | B, B);
    EXPECT_EQ(A | B, A);
    EXPECT_EQ(E | E, E);
}
TEST_F(operators, and) {
    EXPECT_EQ(A & B, B);
    EXPECT_EQ(E & B, E);
    EXPECT_EQ(A & E, E);
    EXPECT_EQ(E & E, E);
}
TEST_F(operators, xor) {
    EXPECT_EQ(A ^ B, E);
    EXPECT_EQ(A ^ E, A);
    EXPECT_EQ(E ^ B, B);
    EXPECT_EQ(E ^ E, E);
}
TEST_F(operators, or_assign) {
    opt::option<int> C{A};
    C |= B;
    EXPECT_EQ(C, A);
    C |= E;
    EXPECT_EQ(C, A);

    C = opt::none;
    C |= E;
    EXPECT_EQ(C, E);
    C |= B;
    EXPECT_EQ(C, B);

    C = opt::none;
    C |= 3;
    EXPECT_EQ(C, 3);
    C |= 4;
    EXPECT_EQ(C, 3);
}

}
