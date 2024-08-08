include_guard(GLOBAL)

function(get_target_name out)
    set(out_tmp "")
    foreach(target IN LISTS ARGN)
        get_target_property(target_unaliased ${target} ALIASED_TARGET)
        if (target_unaliased STREQUAL "target_unaliased-NOTFOUND")
            list(APPEND out_tmp ${target})
        elseif (target_unaliased)
            list(APPEND out_tmp ${target_unaliased})
        else()
            list(APPEND out_tmp ${target})
        endif()
    endforeach()
    set(${out} ${out_tmp} PARENT_SCOPE)
endfunction()

function(target_add_sanitizer target)
    cmake_parse_arguments(ARG "" "" "SANITIZERS;LIBRARIES;OPTIONS" ${ARGN})
    if (NOT ARG_SANITIZERS)
        message(FATAL_ERROR "Please select sanitizers that will be enabled for target '${target}'")
        return()
    endif()

    cmake_parse_arguments(sanitizers "Address;Undefined;Fuzzer" "" "" ${ARG_SANITIZERS})

    get_target_name(libraries ${ARG_LIBRARIES})
    set(options ${ARG_OPTIONS})

    target_compile_options(${target} PRIVATE ${options})
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            if (sanitizers_Address)
                target_compile_definitions(${target} PRIVATE
                    $<$<COMPILE_LANGUAGE:CXX>:_DISABLE_VECTOR_ANNOTATION _DISABLE_STRING_ANNOTATION>
                )
                target_compile_options(${target} PRIVATE
                    $<$<COMPILE_LANGUAGE:CXX>:/fsanitize=address /Zi>
                )
                target_link_options(${target} PRIVATE
                    $<$<COMPILE_LANGUAGE:CXX>:/DEBUG>
                )
            endif()
            if (sanitizers_Fuzzer)
                target_compile_options(${target} PRIVATE /fsanitize=fuzzer)
            endif()
            if (sanitizers_Address OR sanitizers_Fuzzer)
                target_link_options(${target} PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL:NO>)
            endif()
            return()
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
            if (sanitizers_Address)
                # Address sanitizer is broken for Release config
                set_property(TARGET ${target} ${libraries} APPEND
                    PROPERTY COMPILE_OPTIONS $<$<NOT:$<CONFIG:Release>>:-fsanitize=address>
                )
                set_target_properties(${target} ${libraries} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")

                find_library(clang_rt_asan
                    NAMES "clang_rt.asan-x86_64.lib"
                    PATHS "$ENV{ProgramFiles}/LLVM/lib/clang/18/lib/windows"
                )
                if (NOT clang_rt_asan STREQUAL "clang_rt_asan-NOTFOUND")
                    cmake_path(REPLACE_FILENAME clang_rt_asan "clang_rt.asan_cxx-x86_64.lib" OUTPUT_VARIABLE clang_rt_asan_cxx)
                else()
                    set(clang_rt_asan "clang_rt.asan-x86_64.lib")
                    set(clang_rt_asan_cxx "clang_rt.asan_cxx-x86_64.lib")
                endif()
                target_link_libraries(${target} PRIVATE ${clang_rt_asan} ${clang_rt_asan_cxx})
            endif()
            if (sanitizers_Fuzzer)
                target_compile_options(${target} PRIVATE -fsanitize=fuzzer)
                if (sanitizers_Address OR sanitizers_Undefined)
                    target_link_libraries(${target} PRIVATE clang_rt.fuzzer_MT-x86_64.lib)
                else()
                    set_target_properties(${target} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
                    target_link_libraries(${target} PRIVATE
                        clang_rt.fuzzer_MT$<$<CONFIG:Debug>:d>-x86_64.lib
                    )
                endif()
            endif()
            if (sanitizers_Undefined)
                target_compile_options(${target} PRIVATE
                    -fsanitize=undefined,integer,alignment,bool,builtin,function,null,unreachable
                    -fno-sanitize=unsigned-integer-overflow,unsigned-shift-base
                    -fno-sanitize-recover=all
                )
                set_target_properties(${target} ${libraries} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
                target_link_libraries(${target} PRIVATE
                    clang_rt.ubsan_standalone_cxx-x86_64.lib
                    clang_rt.ubsan_standalone-x86_64.lib
                )
            endif()
            return()
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            
            set_property(TARGET ${target} ${libraries} APPEND PROPERTY COMPILE_DEFINITIONS _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
            target_link_options(${target} PRIVATE -coverage)
            if (sanitizers_Address)
                set_property(TARGET ${target} ${libraries} APPEND PROPERTY COMPILE_OPTIONS -fsanitize=address)
                target_link_options(${target} PRIVATE -fsanitize=address)
            endif()
            if (sanitizers_Undefined)
                set_property(TARGET ${target} ${libraries} APPEND PROPERTY COMPILE_OPTIONS -fsanitize=undefined)
                target_link_options(${target} PRIVATE -fsanitize=undefined)
            endif()
            set_target_properties(${target} ${libraries} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
            return()
        endif()
    endif()
    if (sanitizers_Address)
        target_compile_options(${target} PRIVATE -fsanitize=address)
        target_link_options(${target} PRIVATE -fsanitize=address)
    endif()
    if (sanitizers_Undefined)
        target_compile_options(${target} PRIVATE -fsanitize=undefined)
        target_link_options(${target} PRIVATE -fsanitize=undefined)
    endif()
endfunction()
