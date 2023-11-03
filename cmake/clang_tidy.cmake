include_guard(GLOBAL)

function(add_clang_tidy_target name)
    if (NOT CMAKE_GENERATOR MATCHES "Unix Makefiles|Ninja")
        return()
    endif()

    set(run_clang_tidy "${PROJECT_BINARY_DIR}/run-clang-tidy.py")

    file(DOWNLOAD https://raw.githubusercontent.com/llvm/llvm-project/main/clang-tools-extra/clang-tidy/tool/run-clang-tidy.py
        ${run_clang_tidy} STATUS download_status)

    list(GET download_status 0 status_code)
    if (NOT ${status_code} EQUAL 0)
        list(GET download_status 1 error_msg)
        message(WARNING "[clang-tidy] Error with code '${status_code}' occurred during downloading 'run-clang-tidy.py': ${error_msg}")
        return()
    endif()
    find_package(Python 3 QUIET)
    if (NOT Python_FOUND)
        message(WARNING "[clang-tidy] Python3 not found")
        return()
    endif()
    find_program(clang_tidy_binary clang-tidy)
    if (clang_tidy_binary STREQUAL "clang_tidy_binary-NOTFOUND")
        message(WARNING "[clang-tidy] clang-tidy not found")
        return()
    endif()

    set(clang_tidy_config "${PROJECT_SOURCE_DIR}/.clang-tidy")
    add_custom_target(${name}
        COMMAND "${Python_EXECUTABLE}" "${run_clang_tidy}" -quiet -p "${PROJECT_BINARY_DIR}" -use-color
        -clang-tidy-binary "${clang_tidy_binary}" -config-file "${clang_tidy_config}"
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" VERBATIM
        DEPENDS "${run_clang_tidy}"
        SOURCES "${clang_tidy_config}"
    )
    set_target_properties(${name} PROPERTIES RULE_LAUNCH_CUSTOM "\"${CMAKE_COMMAND}\" -E time")
endfunction()
