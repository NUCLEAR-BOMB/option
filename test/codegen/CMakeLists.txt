
add_library(option-codegen-test "sample.cpp")
target_link_libraries(option-codegen-test PRIVATE option)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(option-codegen-test PRIVATE "-Wno-missing-declarations")
endif()

find_package(Python3 COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(current_compiler "msvc")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(current_compiler "clang")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(current_compiler "gcc")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        set(current_compiler "icx")
    else()
        set(current_compiler "unknown")
    endif()

    add_custom_target(run-option-codegen-test VERBATIM
        COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/test/codegen/run.py"
            "$<TARGET_FILE:option-codegen-test>"
            "${CMAKE_CURRENT_SOURCE_DIR}/sample.cpp"
            "${current_compiler}"
    )
else()
    message(STATUS "Python3 is not found: \"run-option-codegen-test\" target was not created")
endif()
