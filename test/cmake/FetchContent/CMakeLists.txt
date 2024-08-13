cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option-FetchContent)

include(FetchContent)
FetchContent_Declare(
    option
    GIT_REPOSITORY ${GIT_REPOSITORY}
    GIT_TAG        ${GIT_TAG}
)
FetchContent_MakeAvailable(option)

add_executable(option-FetchContent-target "main.cpp")
target_link_libraries(option-FetchContent-target PRIVATE option)

add_custom_target(run VERBATIM COMMAND "$<TARGET_FILE:option-FetchContent-target>")
