cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option-add_subdirectory)

add_executable(option-add_subdirectory-target "main.cpp")

add_subdirectory(${ROOT} "${CMAKE_BINARY_DIR}/option")

target_link_libraries(option-add_subdirectory-target PRIVATE option)

add_custom_target(run VERBATIM COMMAND "$<TARGET_FILE:option-add_subdirectory-target>")
