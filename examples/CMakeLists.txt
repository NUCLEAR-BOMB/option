
add_custom_target(option-examples)

function(add_example name)
    add_executable("example-${name}" EXCLUDE_FROM_ALL ${ARGN})
    target_link_libraries("example-${name}" PRIVATE "option")
    add_dependencies(option-examples "example-${name}")
endfunction()

add_example(methods methods.cpp)
add_example(operators operators.cpp)
add_example(functions functions.cpp)
add_example(option-flag option_flag.cpp)
