
# CMake variables

CMake variables that controls build system. See [option][cmake-option].

You can define these variables by providing [`-D`][cmake-d] CMake argument.

## `OPTION_TEST`

**default:** [`${PROJECT_IS_TOP_LEVEL}`][is-project-top-level]

If `TRUE`, adds the `option-test` and `run-option-test` targets.
Use custom target `run-option-test` to run test.

[FetchContent][cmake-fetchcontent]'s [GoogleTest][googletest] and [Boost.pfr][boost-pfr] projects.

Uses [`USE_SANITIZER`](#use_sanitizer) and [`USE_CLANG_TIDY`](#use_clang_tidy) variables to enable sanitizers and clang-tidy testing on `option-test` target.

## `OPTION_EXAMPLES`

**default:** [`${PROJECT_IS_TOP_LEVEL}`][is-project-top-level]

If `TRUE`, adds the `option-examples` and `example-*` targets.
Use `option-examples` custom target to build `example-*` targets.

## `USE_SANITIZER`

**default:** `TRUE`

Enables available sanitizers on `option-test` target.

> [!NOTE]
> Cross-platform sanitizers are very hard to make right, so if you have problems with enabling them you can just disable them by setting `USE_SANITIZER` variable to `FALSE`.

## `USE_CLANG_TIDY`

**default:** `FALSE`

Adds [`clang-tidy`][clang-tidy] custom target which performs lint over `option-test` target.

The only supported generators are "Unix Makefiles" and "Ninja (Multi-Config)". See [CMAKE_EXPORT_COMPILE_COMMANDS][cmake-export-compile-commands].

If the generator is not supported, then the custom target [`clang-tidy`][clang-tidy] will not be added.

## `OPTION_INSTALL`

**default:** `FALSE`

Allows CMake to install this project with [`--install`][cmake-install] argument.

## `OPTION_USE_NATVIS`

**default:** `TRUE`

Adds the "debugger/option.natvis" file to depended targets, which enables Visual Studio natvis for them. See [natvis][natvis].

## `OPTION_USE_NATSTEPFILTER`

**default:** `TRUE`

Adds the "debugger/option.natstepfilter" file to depended targets, which enables Visual Studio natstepfilter for them. See [natstepfilter][natstepfilter].

> [!NOTE]
> The natstepfilter just includes everything in `opt` namespace. If you think this is redundant you can disable it by setting `USE_NATSTEPFILTER` to `FALSE`.

## `USE_LIBASSERT`

**default:** `TRUE`

Adds dependency to [**libassert**][libassert] library to `option-test` target.

> [!NOTE]
> For Visual Studio LLVM toolset when assertation fails address sanitizer produces error. Consider setting `USE_LIBASSERT` to `FALSE` if you have this problem.

[is-project-top-level]: https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
[clang-tidy]: https://clang.llvm.org/extra/clang-tidy
[cmake-export-compile-commands]: https://cmake.org/cmake/help/latest/variable/CMAKE_EXPORT_COMPILE_COMMANDS.html
[cmake-install]: https://cmake.org/cmake/help/latest/manual/cmake.1.html#install-a-project
[natvis]: https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects
[natstepfilter]: https://learn.microsoft.com/ru-ru/visualstudio/debugger/just-my-code
[cmake-option]: https://cmake.org/cmake/help/latest/command/option.html
[cmake-fetchcontent]: https://cmake.org/cmake/help/latest/module/FetchContent.html
[googletest]: https://github.com/google/googletest
[boost-pfr]: https://github.com/boostorg/pfr
[cmake-d]: https://cmake.org/cmake/help/latest/manual/cmake.1.html#cmdoption-cmake-D
[libassert]: https://github.com/jeremy-rifkin/libassert
