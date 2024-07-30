#include <gtest/gtest.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define VC_EXTRALEAN
    #define NOMINMAX
    #include <Windows.h>
#endif

int main(int argc, char** argv) {
    GTEST_FLAG_SET(brief, true);
#ifdef _WIN32
    if (IsDebuggerPresent()) {
        GTEST_FLAG_SET(break_on_failure, true);
    }
#endif
    ::testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
}
