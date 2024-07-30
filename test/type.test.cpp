#include <gtest/gtest.h>
#include <option.hpp>
#include <type_traits>
#include <optional>

#include "utils.hpp"

namespace {

struct type : ::testing::Test {};

TEST_F(type, delete_move_assignment) {
    struct deleted_move_assignment {
        int x;

        deleted_move_assignment() = default;
        deleted_move_assignment(const deleted_move_assignment&) = default;
        deleted_move_assignment(deleted_move_assignment&&) = default;
        deleted_move_assignment& operator=(const deleted_move_assignment&) = default;
    
        deleted_move_assignment& operator=(deleted_move_assignment&&) = delete;
    };
    EXPECT_TRUE(std::is_trivially_copyable_v<deleted_move_assignment>);
    EXPECT_TRUE(std::is_trivially_default_constructible_v<deleted_move_assignment>);
    EXPECT_TRUE(std::is_trivially_copy_constructible_v<deleted_move_assignment>);
    EXPECT_TRUE(std::is_trivially_move_constructible_v<deleted_move_assignment>);
    EXPECT_TRUE(std::is_trivially_copy_assignable_v<deleted_move_assignment>);
    EXPECT_TRUE(std::is_trivially_destructible_v<deleted_move_assignment>);
    EXPECT_FALSE(std::is_move_assignable_v<deleted_move_assignment>);

    // EXPECT_TRUE(std::is_trivially_copy_constructible_v<opt::option<deleted_move_assignment>>);
    // EXPECT_TRUE(std::is_trivially_move_constructible_v<opt::option<deleted_move_assignment>>);
    // EXPECT_TRUE(std::is_trivially_copy_assignable_v<opt::option<deleted_move_assignment>>);
    // EXPECT_TRUE(std::is_trivially_destructible_v<opt::option<deleted_move_assignment>>);
    // 
    using T = deleted_move_assignment;
    using type = opt::impl::enable_copy_move<true, true, true, false>;

    struct aaa : private type {
        T x;
    };

    EXPECT_FALSE(std::is_move_assignable_v<aaa>);

    // EXPECT_FALSE(std::is_move_assignable_v<std::optional<deleted_move_assignment>>);

    // opt::option<deleted_move_assignment> a{};
    // a = opt::option<deleted_move_assignment>{};

    // 
    // struct deleted_move_constructor {
    //     deleted_move_constructor() = default;
    //     deleted_move_constructor(const deleted_move_constructor&) = default;
    //     deleted_move_constructor& operator=(const deleted_move_constructor&) = default;
    //     deleted_move_constructor& operator=(deleted_move_constructor&&) = default;
    // 
    //     deleted_move_constructor(deleted_move_constructor&&) = delete;
    // };
    // 
    // // EXPECT_TRUE(std::is_move_assignable_v<deleted_move_constructor>);
    // // EXPECT_FALSE(std::is_move_constructible_v<deleted_move_constructor>);
    // EXPECT_FALSE(std::is_move_assignable_v<opt::option<deleted_move_constructor>>);
}

TEST_F(type, trivial_move_assignment) {
    // struct trivial {
    //     trivial(const trivial&) = default;
    //     trivial& operator=(trivial&&) = default;
    //     ~trivial() = default;
    // };
    // EXPECT_TRUE(std::is_trivially_move_assignable_v<trivial>);
    // EXPECT_TRUE(std::is_trivially_move_assignable_v<opt::option<trivial>>);
    // 
    // struct nontrivial_move_constructor {
    //     nontrivial_move_constructor() = default;
    //     nontrivial_move_constructor(const nontrivial_move_constructor&) = default;
    //     nontrivial_move_constructor& operator=(const nontrivial_move_constructor&) = default;
    //     nontrivial_move_constructor& operator=(nontrivial_move_constructor&&) = default;
    // 
    //     nontrivial_move_constructor(nontrivial_move_constructor&&) {}
    // };
    // EXPECT_FALSE(std::is_trivially_move_assignable_v<nontrivial_move_constructor>);
    // EXPECT_FALSE(std::is_trivially_move_assignable_v<opt::option<nontrivial_move_constructor>>);
    // 
    // struct nontrivial_move_assignment {
    //     nontrivial_move_assignment() = default;
    //     nontrivial_move_assignment(const nontrivial_move_assignment&) = default;
    //     nontrivial_move_assignment(nontrivial_move_assignment&&) = default;
    //     nontrivial_move_assignment& operator=(const nontrivial_move_assignment&) = default;
    // 
    //     nontrivial_move_assignment& operator=(nontrivial_move_assignment&&) { return *this; }
    // };
    // EXPECT_FALSE(std::is_trivially_move_assignable_v<nontrivial_move_assignment>);
    // EXPECT_FALSE(std::is_trivially_move_assignable_v<opt::option<nontrivial_move_assignment>>);
    // 
    // struct nontrivial_destructor {
    //     
    // };
}

}
