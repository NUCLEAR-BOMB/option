
include("${PROJECT_SOURCE_DIR}/cmake/compiler_warnings.cmake")

add_executable(option-test
    "option.test.cpp" "operators.test.cpp" "special.test.cpp" "utils.hpp" "constexpr.test.cpp"
)
target_add_warnings(option-test)
target_link_libraries(option-test PRIVATE option)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
    URL_HASH SHA256=ffa17fbc5953900994e2deec164bb8949879ea09b411e07f215bfbb1f87f4632
    FIND_PACKAGE_ARGS NAMES GTest
)
set(BUILD_GMOCK FALSE)
set(INSTALL_GTEST FALSE)
set(CMAKE_DISABLE_FIND_PACKAGE_Python TRUE)
set(gtest_disable_pthreads TRUE)
FetchContent_MakeAvailable(googletest)

target_link_libraries(option-test PRIVATE GTest::gtest GTest::gtest_main)

FetchContent_Declare(
    magic_enum
    URL https://github.com/Neargye/magic_enum/archive/refs/tags/v0.9.5.zip
    URL_HASH SHA256=6259fe6629a6d865081ad2911a5bce8b687f8d086ae04560e2d7ce3883e9ea8d
    FIND_PACKAGE_ARGS NAMES magic_enum
)
FetchContent_MakeAvailable(magic_enum)
set_target_properties(magic_enum PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:magic_enum,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(option-test PRIVATE magic_enum::magic_enum)

FetchContent_Declare(
    boost_pfr
    URL https://github.com/boostorg/pfr/archive/refs/tags/2.2.0.zip
    URL_HASH SHA256=becb093245149209b0442e88737495c99114c639dc2121d0e4c08157c034ed03
)
FetchContent_MakeAvailable(boost_pfr)
set_target_properties(boost_pfr PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:boost_pfr,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(option-test PRIVATE Boost::pfr)

if (USE_SANITIZER)
    include("${PROJECT_SOURCE_DIR}/cmake/sanitizers.cmake")
    target_add_sanitizer(option-test)
endif()

if (USE_CLANG_TIDY)
    include("${PROJECT_SOURCE_DIR}/cmake/clang_tidy.cmake")
    set_target_properties(option-test PROPERTIES EXPORT_COMPILE_COMMANDS TRUE)
    add_clang_tidy_target(clang-tidy)
endif()

include(GoogleTest)
gtest_discover_tests(option-test)
