cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option-find_package)

find_package(option REQUIRED)

add_executable(option-find_package-target "main.cpp")
target_link_libraries(option-find_package-target PRIVATE option)

add_custom_target(run VERBATIM COMMAND "$<TARGET_FILE:option-find_package-target>")
