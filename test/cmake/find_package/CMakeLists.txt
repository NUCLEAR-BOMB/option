cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(option-find_package)

find_package(option REQUIRED)

add_executable(option-find_package-target "main.cpp")
target_link_libraries(option-find_package-target PRIVATE option)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(option-find_package-target PRIVATE
        /Wall /WX /permissive-
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(option-find_package-target PRIVATE
        -Weverything -Werror
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(option-find_package-target PRIVATE
        -Wall -Wextra -Wconversion -Werror -Wpedantic -Wold-style-cast -Wfloat-equal -Wlogical-op -Wundef
        -Wredundant-decls -Wshadow -Wwrite-strings -Wpointer-arith -Wcast-qual -Wswitch-default
        -Wmissing-include-dirs -Wcast-align -Wswitch-enum -Wctor-dtor-privacy
        -Wsign-conversion -Wdisabled-optimization -Weffc++ -Winvalid-pch -Wmissing-declarations
        -Woverloaded-virtual -Wunused-but-set-variable -Wunused-result -Wnoexcept -Wdouble-promotion
        -Wtrampolines -Wzero-as-null-pointer-constant -Wuseless-cast -Wvector-operation-performance
        -Wshift-overflow=2 -Wnull-dereference -Wduplicated-cond -Walloc-zero -Walloca -Wduplicated-branches
        -Wcast-align -Wunused -Wsign-conversion -Wold-style-cast
    )
endif()

add_custom_target(run VERBATIM COMMAND "$<TARGET_FILE:option-find_package-target>")
