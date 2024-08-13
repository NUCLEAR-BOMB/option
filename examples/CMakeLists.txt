
set(file_list "")
set(target_list "")

macro(add_example name file)
    add_executable(example-${name} EXCLUDE_FROM_ALL ${file})
    target_link_libraries(example-${name} PRIVATE option)
    list(APPEND file_list "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    list(APPEND target_list "$<TARGET_FILE:example-${name}>")
endmacro()

add_example(methods methods.cpp)
add_example(operators operators.cpp)
add_example(functions functions.cpp)
add_example(option-traits option_traits.cpp)

find_package(Python3 COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
    add_custom_target(check-option-examples VERBATIM
        COMMAND ${Python3_EXECUTABLE} "./check.py" "${file_list}" "${target_list}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    message(STATUS "Python3 is not found: \"check-option-examples\" target is not created")
endif()
