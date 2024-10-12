
option(OLD_LIBSTDCPP "" FALSE)

set_directory_properties(PROPERTIES COMPILE_OPTIONS "")

add_library(option-codegen-test "sample.cpp")
target_link_libraries(option-codegen-test PRIVATE option)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(option-codegen-test PRIVATE -w)
endif()

find_package(Python3 COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
    set(current_conditions "")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        list(APPEND current_conditions "msvc")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            list(APPEND current_conditions "clang-cl")
        else()
            list(APPEND current_conditions "clang")
            include(CheckCXXSymbolExists)
            check_cxx_symbol_exists(_LIBCPP_VERSION "ciso646" LIBCPP)
            if (LIBCPP)
                list(APPEND current_conditions "libcpp")
            elseif (OLD_LIBSTDCPP)
                list(APPEND current_conditions "old-libstdcpp")
            else()
                list(APPEND current_conditions "libstdcpp")
            endif()
        endif()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND current_conditions "gcc")
        if (OLD_LIBSTDCPP)
            list(APPEND current_conditions "old-libstdcpp")
        endif()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
        list(APPEND current_conditions "icx")
    else()
        list(APPEND current_conditions "unknown")
    endif()

    find_program(llvm_objdump_path
        NAMES llvm-objdump llvm-objdump-18 llvm-objdump-17 llvm-objdump-16 llvm-objdump-15 llvm-objdump-14 llvm-objdump-13 llvm-objdump-12 llvm-objdump-11 llvm-objdump-10 llvm-objdump-9
    )
    if (llvm_objdump_path)
        add_custom_target(run-option-codegen-test VERBATIM
            COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/test/codegen/run.py"
                ${llvm_objdump_path}
                "$<TARGET_FILE:option-codegen-test>"
                "${CMAKE_CURRENT_SOURCE_DIR}/sample.cpp"
                "${current_conditions}"
                "${CMAKE_CXX_COMPILER_VERSION}"
        )
    else()
        message(STATUS "llvm-objdump is not found: \"run-option-codegen-test\" target was not created")
    endif()
else()
    message(STATUS "Python3 is not found: \"run-option-codegen-test\" target was not created")
endif()
