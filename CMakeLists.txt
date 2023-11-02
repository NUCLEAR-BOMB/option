cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option LANGUAGES CXX)

add_library(option INTERFACE "include/option.hpp")
target_include_directories(option INTERFACE "include/")
target_compile_features(option INTERFACE cxx_std_17)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    target_sources(option INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/option_visualization.natvis")
endif()

enable_testing()
add_subdirectory(test)
