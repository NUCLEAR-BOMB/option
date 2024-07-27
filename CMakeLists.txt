cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option VERSION 1.0 LANGUAGES CXX)

option(BUILD_TEST "Enable 'option-test' target" ${PROJECT_IS_TOP_LEVEL})
option(BUILD_EXAMPLES "Enable 'option-examples' target" ${PROJECT_IS_TOP_LEVEL})
option(USE_SANITIZER "Enable sanitizers for test target" TRUE)
option(USE_CLANG_TIDY "Enable clang-tidy for test target" FALSE)
option(INSTALL_OPTION "Enable --install for 'option' project" FALSE)
option(USE_NATVIS "Enable .natvis file for Visual Studio debugger" TRUE)

add_library(option INTERFACE "include/option.hpp")

if (NOT PROJECT_IS_TOP_LEVEL)
    set(include_as_system "SYSTEM")
endif()

target_include_directories(option ${include_as_system} INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(option INTERFACE cxx_std_17)

if (INSTALL_OPTION)
    include(GNUInstallDirs)
endif()

if (USE_NATVIS)
    target_sources(option INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/debugger/option.natvis>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_DATADIR}/debugger/option.natvis>"
    )
endif()

if (BUILD_TEST)
    add_subdirectory(test)
endif()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (INSTALL_OPTION)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(
        TARGETS option
        EXPORT option-targets
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    install(
        EXPORT option-targets
        FILE "option-targets.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/option"
    )
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/option-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
        ARCH_INDEPENDENT
    )
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/option-config.cmake.in"
        "${PROJECT_BINARY_DIR}/option-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/option"
    )
    install(
        FILES "${PROJECT_BINARY_DIR}/option-config.cmake"
              "${PROJECT_BINARY_DIR}/option-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/option"
    )
    install(FILES
        "${PROJECT_SOURCE_DIR}/include/option.hpp"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    if (CMAKE_GENERATOR MATCHES "Visual Studio")
        install(FILES
            "${PROJECT_SOURCE_DIR}/option_visualization.natvis"
            DESTINATION "${CMAKE_INSTALL_DATADIR}"
        )
    endif()
endif()
