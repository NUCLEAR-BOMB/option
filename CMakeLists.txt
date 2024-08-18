cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option VERSION 1.0 LANGUAGES CXX)

if (PROJECT_IS_TOP_LEVEL)
    set(not_project_is_top_level FALSE)
else()
    set(not_project_is_top_level TRUE)
endif()

option(BUILD_TEST "Enable 'option-test' target" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_EXAMPLES "Enable 'option-examples' target" ${PROJECT_IS_TOP_LEVEL})
option(USE_SANITIZER "Enable sanitizers for test target" TRUE)
option(USE_CLANG_TIDY "Enable clang-tidy for test target" FALSE)
option(INSTALL_OPTION "Enable --install for 'option' project" FALSE)
option(USE_NATVIS "Enable .natvis file for Visual Studio debugger" TRUE)
option(USE_NATSTEPFILTER "Enable .natstepfilter file for Visual Studio debugger" ${not_project_is_top_level})
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

if (USE_NATVIS)
    target_sources(option INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/debugger/option.natvis>)
endif()
if (USE_NATSTEPFILTER)
    target_sources(option INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/debugger/option.natstepfilter>)
endif()

if(INSTALL_OPTION)
    set(exclude_from_all EXCLUDE_FROM_ALL)
endif()

if (BUILD_TEST)
    add_subdirectory(test ${exclude_from_all})
endif()

if (BUILD_EXAMPLES)
    add_subdirectory(examples ${exclude_from_all})
endif()

if (INSTALL_OPTION)
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
