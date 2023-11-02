
include("${PROJECT_SOURCE_DIR}/cmake/compiler_warnings.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/sanitizers.cmake")

add_executable(option-test "option.test.cpp" "operators.test.cpp" "special.test.cpp")
target_add_warnings(option-test)
target_link_libraries(option-test PRIVATE option)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
    URL_HASH SHA256=ffa17fbc5953900994e2deec164bb8949879ea09b411e07f215bfbb1f87f4632
    FIND_PACKAGE_ARGS NAMES GTest
)
set(gtest_force_shared_crt TRUE CACHE BOOL "" FORCE)
set(BUILD_GMOCK FALSE)
set(INSTALL_GTEST FALSE)
set(CMAKE_DISABLE_FIND_PACKAGE_Python TRUE)
set(gtest_disable_pthreads TRUE)
FetchContent_MakeAvailable(googletest)

target_link_libraries(option-test PRIVATE GTest::gtest GTest::gtest_main)

if (USE_SANITIZER)
    target_add_sanitizer(option-test)
endif()

include(GoogleTest)
gtest_discover_tests(option-test)
