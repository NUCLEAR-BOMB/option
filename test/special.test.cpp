#include <gtest/gtest.h>

#include <option.hpp>

#define TypeParam T

namespace {

template<class T>
struct special : public ::testing::Test {
    const T A = T(0);
    const T B = T(1);
    const opt::option<T> E{opt::none};
};

using special_types = ::testing::Types<bool>;
TYPED_TEST_SUITE(special, special_types,);

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

}
