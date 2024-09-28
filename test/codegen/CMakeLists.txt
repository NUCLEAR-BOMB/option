
add_library(option-codegen-test "sample.cpp")
target_link_libraries(option-codegen-test PRIVATE option)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(option-codegen-test PRIVATE -w)
endif()

find_package(Python3 COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(current_compiler "msvc")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            set(current_compiler "clang-cl")
        else()
            set(current_compiler "clang")
        endif()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(current_compiler "gcc")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        set(current_compiler "icx")
    else()
        set(current_compiler "unknown")
    endif()

    find_program(llvm_objdump_path
        NAMES "llvm-objdump" "llvm-objdump-18" "llvm-objdump-17" "llvm-objdump-16" "llvm-objdump-15" "llvm-objdump-14" "llvm-objdump-13"
    )
    if (llvm_objdump_path)
        add_custom_target(run-option-codegen-test VERBATIM
            COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/test/codegen/run.py"
                ${llvm_objdump_path}
                "$<TARGET_FILE:option-codegen-test>"
                "${CMAKE_CURRENT_SOURCE_DIR}/sample.cpp"
                "${current_compiler}"
        )
    else()
        message(STATUS "llvm-objdump is not found: \"run-option-codegen-test\" target was not created")
    endif()
else()
    message(STATUS "Python3 is not found: \"run-option-codegen-test\" target was not created")
endif()
