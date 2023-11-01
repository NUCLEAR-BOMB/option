#include <gtest/gtest.h>

#include <option.hpp>

namespace {

struct operators : ::testing::Test {
    opt::option<int> A1{1};
    opt::option<int> B1{2};

    opt::option<int> E{opt::none};
};

TEST_F(operators, equal) {
    EXPECT_TRUE(A1 == A1);
    EXPECT_FALSE(A1 == B1);

    EXPECT_TRUE(E == E);
    EXPECT_FALSE(A1 == E);

    EXPECT_TRUE(A1 == 1);
    EXPECT_FALSE(E == 1);
}
TEST_F(operators, not_equal) {
    EXPECT_TRUE(A1 != B1);
    EXPECT_FALSE(A1 != A1);

    EXPECT_TRUE(A1 != E);
    EXPECT_FALSE(E != E);

    EXPECT_TRUE(B1 != 1);
    EXPECT_TRUE(E != 1);
}
TEST_F(operators, less) {
    EXPECT_TRUE(A1 < B1);
    EXPECT_FALSE(A1 < A1);

    EXPECT_TRUE(E < A1);
    EXPECT_FALSE(E < E);

    EXPECT_TRUE(A1 < 2);
    EXPECT_FALSE(E > 1);
}
TEST_F(operators, less_equal) {
    EXPECT_TRUE(A1 <= A1);
    EXPECT_FALSE(B1 <= A1);

    EXPECT_TRUE(E <= A1);
    EXPECT_FALSE(A1 <= E);

    EXPECT_TRUE(A1 <= 1);
    EXPECT_FALSE(1 <= E);
}
TEST_F(operators, greater) {
    EXPECT_TRUE(B1 > A1);
    EXPECT_FALSE(A1 > A1);

    EXPECT_TRUE(A1 > E);
    EXPECT_FALSE(E > A1);

    EXPECT_TRUE(B1 > 1);
    EXPECT_FALSE(E > 1);
}
TEST_F(operators, greater_equal) {
    EXPECT_TRUE(A1 >= A1);
    EXPECT_FALSE(A1 >= B1);

    EXPECT_TRUE(A1 >= E);
    EXPECT_FALSE(E >= A1);

    EXPECT_TRUE(B1 >= 1);
    EXPECT_FALSE(E >= 1);
}

}
