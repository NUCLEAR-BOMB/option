cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option-ExternalProject)

include(ExternalProject)
ExternalProject_Add(
    option
    PREFIX "${CMAKE_BINARY_DIR}/option"
    GIT_REPOSITORY ${GIT_REPOSITORY}
    GIT_TAG ${GIT_TAG}
    TIMEOUT 10
    CMAKE_COMMAND ${CMAKE_COMMAND} -E env --unset=CFLAGS --unset=CXXFLAGS -- ${CMAKE_COMMAND}
    INSTALL_COMMAND ""
    TEST_COMMAND ${CMAKE_COMMAND} --build . --target run-option-test
)
ExternalProject_Get_Property(option source_dir)

add_executable(option-ExternalProject-target "main.cpp")
target_compile_features(option-ExternalProject-target PRIVATE cxx_std_17)
target_include_directories(option-ExternalProject-target SYSTEM PRIVATE "${source_dir}/include")

add_custom_target(run VERBATIM COMMAND "$<TARGET_FILE:option-ExternalProject-target>")
