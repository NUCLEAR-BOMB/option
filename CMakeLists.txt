cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option LANGUAGES CXX)

add_library(option INTERFACE "include/option.hpp")
target_include_directories(option INTERFACE "include/")
target_compile_features(option INTERFACE cxx_std_17)

enable_testing()
add_subdirectory(test)
