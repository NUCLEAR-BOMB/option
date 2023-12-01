include_guard(GLOBAL)

function(add_clang_tidy_target)
    if (NOT CMAKE_GENERATOR MATCHES "Unix Makefiles|Ninja")
        return()
    endif()
    cmake_parse_arguments(ARG "USE_COLOR;QUIET;SHOW_TIME" "NAME;CONFIG;LINE_FILTER;HEADER_FILTER" "" ${ARGN})

    set(run_clang_tidy "${PROJECT_BINARY_DIR}/run-clang-tidy.py")

    file(DOWNLOAD https://raw.githubusercontent.com/llvm/llvm-project/main/clang-tools-extra/clang-tidy/tool/run-clang-tidy.py
        ${run_clang_tidy} STATUS download_status)

    list(GET download_status 0 status_code)
    if (NOT ${status_code} EQUAL 0)
        list(GET download_status 1 error_msg)
        message(WARNING "[clang-tidy] Error with code '${status_code}' occurred during downloading 'run-clang-tidy.py': ${error_msg}")
        return()
    endif()
    find_package(Python3 QUIET COMPONENTS Interpreter)
    if (NOT Python3_Interpreter_FOUND)
        message(WARNING "[clang-tidy] Python3 Interpreter is not found")
        return()
    endif()
    find_program(clang_tidy_binary clang-tidy)
    if (clang_tidy_binary STREQUAL "clang_tidy_binary-NOTFOUND")
        message(WARNING "[clang-tidy] clang-tidy is not found")
        return()
    endif()

    if (ARG_LINE_FILTER)
        set(line_filter_arg "-line-filter=${ARG_LINE_FILTER}")
    endif()
    if (ARG_HEADER_FILTER)
        set(header_filer_arg "-header-filter=${ARG_HEADER_FILTER}")
    endif()

    add_custom_target(${ARG_NAME}
        COMMAND "${Python3_EXECUTABLE}" "${run_clang_tidy}" -p "${PROJECT_BINARY_DIR}"
        -clang-tidy-binary "${clang_tidy_binary}" -config-file "${ARG_CONFIG}"
        $<$<BOOL:${ARG_USE_COLOR}>:-use-color> $<$<BOOL:${ARG_QUIET}>:-quiet>
        ${line_filter_arg} ${header_filer_arg}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" VERBATIM
        DEPENDS "${run_clang_tidy}"
        SOURCES "${clang_tidy_config}"
    )
    if (SHOW_TIME)
        set_target_properties(${ARG_NAME} PROPERTIES RULE_LAUNCH_CUSTOM "\"${CMAKE_COMMAND}\" -E time")
    endif()
endfunction()
