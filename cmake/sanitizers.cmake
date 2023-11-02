include_guard(GLOBAL)

function(get_target_libraries out target)
    get_target_property(raw_libraries ${target} LINK_LIBRARIES)
    set(result "")
    foreach(raw_lib IN LISTS raw_libraries)
        get_target_property(lib ${raw_lib} ALIASED_TARGET)

        if (lib STREQUAL "lib-NOTFOUND")
            list(APPEND result ${raw_lib})
        else()
            get_target_property(lib_type ${lib} TYPE)
            if (NOT ${lib_type} STREQUAL "INTERFACE_LIBRARY")
                list(APPEND result ${lib})
            endif()
        endif()
    endforeach()
    set(${out} ${result} PARENT_SCOPE)
endfunction()

function(target_add_sanitizer target)
    get_target_libraries(libraries ${target})
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            set_property(TARGET ${target} ${libraries} APPEND PROPERTY COMPILE_OPTIONS /fsanitize=address)
            target_link_options(${target} PRIVATE /INCREMENTAL:NO)
            set_target_properties(${target} ${libraries} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
                MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase"
            )
            return()
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set_property(TARGET ${target} ${libraries} APPEND PROPERTY COMPILE_OPTIONS -fsanitize=address,undefined)
            set_property(TARGET ${target} ${libraries} APPEND PROPERTY COMPILE_DEFINITIONS _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
            target_link_options(${target} PRIVATE -coverage -fsanitize=address,undefined)
            set_target_properties(${target} ${libraries} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
            return()
        endif()
    endif()
    target_compile_options(${target} PRIVATE -fsanitize=address,undefined)
    target_link_options(${target} PRIVATE -fsanitize=address,undefined)
endfunction()
