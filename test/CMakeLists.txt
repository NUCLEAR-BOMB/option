
if (DEFINED OPTION_DEV_CLANGCL_SANITIZERS AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTION_DEV_CLANGCL_SANITIZERS}")
    if (CMAKE_SIZEOF_VOID_P STREQUAL 8)
        link_libraries(
            "$ENV{ProgramFiles}/LLVM/lib/clang/18/lib/windows/clang_rt.asan-x86_64.lib"
            "$ENV{ProgramFiles}/LLVM/lib/clang/18/lib/windows/clang_rt.asan_cxx-x86_64.lib"
            clang_rt.ubsan_standalone_cxx-x86_64.lib
            clang_rt.ubsan_standalone-x86_64.lib
        )
    elseif(CMAKE_SIZEOF_VOID_P STREQUAL 4)
        link_libraries(
            "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/VC/Tools/Llvm/lib/clang/17/lib/windows/clang_rt.asan-i386.lib"
            "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/VC/Tools/Llvm/lib/clang/17/lib/windows/clang_rt.asan_cxx-i386.lib"
            clang_rt.ubsan_standalone_cxx-x86_64.lib
            clang_rt.ubsan_standalone-x86_64.lib
        )
    endif()
endif()

include("${PROJECT_SOURCE_DIR}/cmake/compiler_warnings.cmake")

add_executable(option-test
    "option.test.cpp"
    "operators.test.cpp"
    "special.test.cpp"
    "lifetime.test.cpp"
    "special_traits.test.cpp"
    "forward_std.test.cpp"
    "meta.test.cpp"
    "functions.cpp"
    "main.cpp"
    
    "utils.hpp"
)
target_add_warnings(option-test)
target_link_libraries(option-test PRIVATE option)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(option-test PRIVATE
        /Zc:preprocessor /Zc:gotoScope /Zc:__cplusplus /bigobj /MP /fp:strict
    )
endif()

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE CACHE BOOL "")
include(FetchContent)
FetchContent_Declare(
    doctest
    URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.zip
    URL_HASH SHA512=c4b0a7b88b577e2d95c8da45f19508ea615a4c6d0ccdc2c458922ac83b6f87631c0ae25fa5cb2052c90b2484917440fb83b6d1cdd5b1ee71780a4f7a632170ee
    FIND_PACKAGE_ARGS 2.4.11
)
FetchContent_MakeAvailable(doctest)

target_link_libraries(option-test PRIVATE doctest::doctest)
target_compile_definitions(option-test PRIVATE
    DOCTEST_CONFIG_NO_MULTITHREADING # https://github.com/doctest/doctest/blob/master/doc/markdown/configuration.md#doctest_config_no_multithreading
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS # https://github.com/doctest/doctest/blob/master/doc/markdown/configuration.md#doctest_config_super_fast_asserts
)

FetchContent_Declare(
    boost_pfr
    URL https://github.com/boostorg/pfr/archive/refs/tags/2.2.0.zip
    URL_HASH SHA256=becb093245149209b0442e88737495c99114c639dc2121d0e4c08157c034ed03
    FIND_PACKAGE_ARGS 2.2.0
)
FetchContent_MakeAvailable(boost_pfr)
set_target_properties(boost_pfr PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:boost_pfr,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(option-test PRIVATE Boost::pfr)

if (USE_LIBASSERT)
    FetchContent_Declare(
        libassert
        URL https://github.com/jeremy-rifkin/libassert/archive/refs/tags/v2.1.0.tar.gz
        URL_HASH SHA256=e42405b49cde017c44c78aacac35c6e03564532838709031e73d10ab71f5363d
        FIND_PACKAGE_ARGS 2.1.0
    )
    set(CMAKE_SKIP_INSTALL_RULES TRUE)
    FetchContent_MakeAvailable(libassert)
    # set_target_properties(libassert-lib PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:libassert-lib,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(option-test PRIVATE libassert::assert)
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        add_custom_command(TARGET option-test POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different $<TARGET_FILE:libassert::assert> $<TARGET_FILE_DIR:option-test>
        )
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        set_target_properties(libassert-lib cpptrace-lib PROPERTIES COMPILE_OPTIONS "-w")
    endif()
endif()

if (USE_CLANG_TIDY)
    include("${PROJECT_SOURCE_DIR}/cmake/clang_tidy.cmake")
    set_target_properties(option-test PROPERTIES EXPORT_COMPILE_COMMANDS TRUE)
    add_clang_tidy_target(NAME clang-tidy
        CONFIG "${PROJECT_SOURCE_DIR}/.clang-tidy"
        USE_COLOR QUIET SHOW_TIME
    )
endif()

add_custom_target(run-option-test VERBATIM
    COMMAND "$<TARGET_FILE:option-test>"
)
