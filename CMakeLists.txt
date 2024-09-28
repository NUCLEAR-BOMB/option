cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option VERSION 1.2.0 LANGUAGES CXX)

option(OPTION_TEST "Enable 'option-test' target" ${PROJECT_IS_TOP_LEVEL})
option(OPTION_EXAMPLES "Enable 'option-examples' target" ${PROJECT_IS_TOP_LEVEL})
option(OPTION_BENCHMARK "Enable benchmarks targets" FALSE)
option(OPTION_CODEGEN_TEST "Enable 'option-codegen-test' target" TRUE)
option(USE_SANITIZER "Enable sanitizers for test target" TRUE)
option(USE_CLANG_TIDY "Enable clang-tidy for test target" FALSE)
option(OPTION_INSTALL "Enable --install for 'option' project" FALSE)
option(OPTION_USE_NATVIS "Enable .natvis file for Visual Studio debugger" TRUE)
option(OPTION_USE_NATSTEPFILTER "Enable .natstepfilter file for Visual Studio debugger" FALSE)
option(USE_LIBASSERT "Enable libassert library integration" TRUE)

add_library(option INTERFACE
    "include/opt/option.hpp"
    "include/opt/option_fwd.hpp"
)

if (NOT PROJECT_IS_TOP_LEVEL)
    set(include_as_system "SYSTEM")
endif()

target_include_directories(option ${include_as_system} INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

target_compile_features(option INTERFACE cxx_std_17)

if (OPTION_USE_NATVIS)
    target_sources(option INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/debugger/option.natvis>)
endif()
if (OPTION_USE_NATSTEPFILTER)
    target_sources(option INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/debugger/option.natstepfilter>)
endif()

if(OPTION_INSTALL)
    set(exclude_from_all EXCLUDE_FROM_ALL)
endif()

if (OPTION_TEST)
    add_subdirectory(test ${exclude_from_all})
endif()

if (OPTION_EXAMPLES)
    add_subdirectory(examples ${exclude_from_all})
endif()

if (OPTION_BENCHMARK)
    add_subdirectory(benchmark ${exclude_from_all})
endif()

if (OPTION_CODEGEN_TEST)
    add_subdirectory(test/codegen ${exclude_from_all})
endif()

if (OPTION_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    # https://cmake.org/cmake/help/latest/command/find_package.html#search-modes
    # Generate config file
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/option-config.cmake.in"
        "${PROJECT_BINARY_DIR}/option-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/option"
    )
    # Generate version file
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/option-config-version.cmake"
        # VERSION is implicit
        COMPATIBILITY ExactVersion
        ARCH_INDEPENDENT
    )
    # Install cmake config files
    install(
        FILES
            "${PROJECT_BINARY_DIR}/option-config.cmake"
            "${PROJECT_BINARY_DIR}/option-config-version.cmake"
        DESTINATION
            "${CMAKE_INSTALL_LIBDIR}/cmake/option"
    )

    # Generate option targets
    install(
        TARGETS              option
        EXPORT               option-targets
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    # Install option targets
    install(
        EXPORT      option-targets
        FILE        "option-targets.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/option"
    )

    # Install headers
    install(
        DIRECTORY   "include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    # Install natvis and natstepfilter
    install(
        FILES "${PROJECT_SOURCE_DIR}/debugger/option.natvis"
              "${PROJECT_SOURCE_DIR}/debugger/option.natstepfilter"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/debugger"
    )
endif()
