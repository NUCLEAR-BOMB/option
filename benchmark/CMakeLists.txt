
find_package(Python3 COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/benchmark_opt_option_src.cpp"
        COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/benchmark/generate.py"
            "opt::option"
            "${CMAKE_CURRENT_BINARY_DIR}/benchmark_opt_option_src.cpp"
            "1000"
        VERBATIM
    )
    add_library(build-benchmark-opt-option EXCLUDE_FROM_ALL "${CMAKE_CURRENT_BINARY_DIR}/benchmark_opt_option_src.cpp")
    target_link_libraries(build-benchmark-opt-option PRIVATE option)
    set_property(TARGET build-benchmark-opt-option PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/benchmark_std_optional_src.cpp"
        COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/benchmark/generate.py"
            "std::optional"
            "${CMAKE_CURRENT_BINARY_DIR}/benchmark_std_optional_src.cpp"
            "1000"
        VERBATIM
    )
    add_library(build-benchmark-std-optional EXCLUDE_FROM_ALL "${CMAKE_CURRENT_BINARY_DIR}/benchmark_std_optional_src.cpp")
    target_compile_features(build-benchmark-std-optional PRIVATE cxx_std_17)
    set_property(TARGET build-benchmark-std-optional PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
else()
    message(STATUS "Python3 is not found: \"build-benchmark\" target was not created")
endif()
