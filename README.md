
# option

Replacement for [`std::optional`][std::optional] with efficient memory usage and additional features.

- Functionality from [C++23 `std::optional`][std::optional monadic], Rust's [`std::option::Option`][Rust Option] and other `opt::option`'s own stuff. See [reference](./docs/markdown/reference.md).
- Zero memory overhead with types that have unused values. See [builtin traits](./docs/markdown/builtin_traits.md).
- Support for nested `opt::option`s with zero memory overhead.
- Simpler interface than `std::optional` (constructors without [`std::in_place`][std::in_place]), allows [direct list initialization][direct-list-initialization].
- Custom size optimizations for your own types (`opt::option_traits`). See [option traits guide](./docs/markdown/custom_traits_guide.md).
- Allows reference types.

**Table of contents**:

- [Overview](#overview)
- [Why `opt::option`?](#why-optoption)
- [Additional functionality](#additional-functionality)
- [Compiler support](#compiler-support)
- [CMake integration](#cmake-integration)
  - [find\_package](#find_package)
  - [FetchContent](#fetchcontent)
  - [ExternalProject](#externalproject)
- [How it works](#how-it-works)
- [Compatibility with `std::optional`](#compatibility-with-stdoptional)
- [About undefined behavior](#about-undefined-behavior)

# Overview

**Types with unused states.**

```cpp
opt::option<float> a = 123.456f;
// Has the same size as just float
static_assert(sizeof(a) == sizeof(float));

// Convert `opt::option<float>` into `opt::option<int>`, and print the value if it is not empty
a.map([](float x) { return int(x); }).inspect([](int x) {
    std::cout << x << '\n';
});
a = opt::none;
// Convert `opt::option<float>` to `opt::option<int>` if it contains a value; otherwise, invoke the 'else' condition.
int b = a.map_or_else([] { return 1; }, [](float x) { return int(x) + 5; });
std::cout << b << '\n';

int c = 2;
opt::option<int*> d = &c;
// Has the same size as just a pointer
static_assert(sizeof(d) == sizeof(int*));

// Print the dereferenced value
std::cout << **d << '\n';

// Empty opt::option is not nullptr for pointers!
d = nullptr;
// Print the pointer address or "empty option" if option does not contain one
std::cout << opt::io(d, "empty option") << '\n';
```

---

**Complex types that contain unused states.**

```cpp
opt::option<std::tuple<int, unsigned, float>> a;
// Uses `float` value in `std::tuple` to store "has value" state
static_assert(sizeof(a) == sizeof(std::tuple<int, unsigned, float>));

a.emplace(1, 2u, 3.f);

std::cout << std::get<0>(*a) << std::get<1>(*a) << std::get<2>(*a) << '\n';

struct S1 {
    unsigned x;
    char y;
    bool z;
};
opt::option<S1> b{5u, 'a', false};
// Uses `bool` value in `S1` to store "has value" state
static_assert(sizeof(b) == sizeof(S1));

b.reset();
std::cout << b.has_value() << '\n';

struct S2 {
    S1 x;
    std::tuple<int, int> y;
};
opt::option<S2> c{S2{S1{1, 'b', true}, {2, 3}}};
// Uses `bool` value in `x` data member inside `S1` type to store "has value" state
static_assert(sizeof(c) == sizeof(S2));

c->x.x = 100u;
std::cout << c->x.x << '\n';
```

---

**Nested `opt::option`s.**

```cpp
opt::option<opt::option<bool>> a{true};
// Uses `bool` value to store two "has value" states
static_assert(sizeof(a) == sizeof(bool));

a->reset();
a.reset();

opt::option<opt::option<opt::option<opt::option<opt::option<float>>>>> b;
// Uses `bool` value to store emptiness level value
static_assert(sizeof(b) == sizeof(float));

(*****b).reset();
(****b).reset();
(***b).reset();
(**b).reset();
b->reset();
b.reset();
```

# Why `opt::option`?

`opt::option` allows to minimize the type size to a minimum.

Minimizing the type size is always a good thing.
[Cache locality][cache locality] can often improve performance of the program even more than any other performed optimization.

It supports reference types, so you can avoid using inconvenient `std::reference_wrapper` and dangerous nullable pointers.

Features taken from Rust's [`std::option::Option`][Rust Option] (`.take`, `.map_or(_else)`, `.flatten`, `.unzip`, etc.), [monadic operations from C++23][std::optional monadic] (`.and_then`, `.map` (renamed `.transform`), `.or_else`) and custom ones (`.ptr_or_null`, `opt::option_cast`, `opt::from_nullable`, operators equivalent to methods, etc.).

# Additional functionality

The option library provides extended functionality over standard `std::optional`, which can lead to the use of more efficient and cleaner code.

Most methods/functions are inspired from Rust's [`std::option::Option`][Rust Option]:
- `opt::option<T>::has_value_and`
- `opt::option<T>::take`
- `opt::option<T>::take_if`
- `opt::option<T>::inspect`
- `opt::option<T>::get_unchecked`
- `opt::option<T>::value_or_default`
- `opt::option<T>::map_or`
- `opt::option<T>::map_or_else`
- `opt::option<T>::filter`
- `opt::option<T>::flatten`
- `opt::option<T>::unzip`
- `opt::option<T>::replace`
- `opt::zip`
- `opt::zip_with`
- `opt::operator|`
- `opt::operator|=`
- `opt::operator&`
- `opt::operator^`

But the option library has its own functionality:
- `opt::option<T>::value_or_throw` (explicit `opt::option<T>::value`)
- `opt::option<T>::ptr_or_null`
- `opt::option<T>::assume_has_value`
- `opt::option_cast`
- `opt::from_nullable`
- `opt::get` (from tuple-like or `std::variant`)
- `opt::io` (read from/write to stream)
- `opt::swap`

See [**reference**](./docs/markdown/reference.md) for more details.

# Compiler support

The library is tested with these compiler versions with sanitizers enabled[^1]:
- GCC: 14.0.1, 13.2.0, 12.3.0, 11.4.0
- Clang: 18.1.3, 17.0.6, 16.0.6, 15.0.7, 14.0.0, 13.0.1, 12.0.1, 11.1.0, 10.0.0, 9.0.1 ([`libc++`][libc++] and [`libstdc++`][libstdc++])
- MSVC: 19.40.33813.0 (VS v143)
- ClangCL: 17.0.3
- IntelLLVM: 2024.2.1

> [!IMPORTANT]
> The library could work with other versions of compilers, but some functionality may be broken.
> 
> Consider using sanitizers when using library using an untested version of the compiler to prevent unexpected behaviour.

The library has various tests that tries to cover every part of the library.

Uses [clang-tidy][clang-tidy] (18.1.8) to minimize the number of bugs and unexpected behavior.

# CMake integration

The `CMakeLists.txt` file in the project root directory provides `option` [`INTERFACE`][INTERFACE add_library] target, which adds:
- include directory `include/`.
- `cxx_std_17` compile feature.
- Includes `debugger/option.natvis` and `debugger/option.natstepfilter` with ability to disable them.

See [project cmake variables](./docs/markdown/cmake_variables.md) for more information.

## [find_package][find_package]

To use [`find_package`][find_package] command you need to firstly install the `option` project.

Generate the `option` project with the variable `INSTALL_OPTION` defined to `TRUE`:

```shell
cmake -B build -DINSTALL_OPTION=TRUE
```
> Generate project into `build` directory, and allow to install it.

```shell
sudo cmake --install build
```
> Install project from `build` directory. Requires administrative permissions

> [!NOTE]
> The version file for the project has the `COMPATIBILITY` mode set to `ExactVersion` and `ARCH_INDEPENDENT` argument.

Use [`find_package`][find_package] to find the `option` library and [`target_link_libraries`][target_link_libraries] to specify dependency on it.
```cmake
find_package(option REQUIRED)
...
target_link_libraries(<target> PRIVATE option)
```

Next, you can `#include` the library header to use it.

## [FetchContent][FetchContent]

You can use [`FetchContent`][FetchContent] either to clone the git repository or specify the archive URL to directly download it.

Download through git repository:

```cmake
include(FetchContent)
FetcnhContet_Declare(
    option
    GIT_REPOSITORY https://github.com/NUCLEAR-BOMB/option.git
    GIT_TAG        <commit/tag>
)
FetchContent_MakeAvailable(option)
...
target_link_libraries(<target> PRIVATE option)
```

> [!TIP]
> You could specify the `SYSTEM` (since CMake 3.25) and `EXCLUDE_FROM_ALL` (since CMake 3.28) arguments to `FetcnhContet_Declare` but the library already uses `target_include_directories` with `SYSTEM` and it is header-only library.

Using URL to the archive:

```cmake
include(FetchContent)
FetchContent_Declare(
    option
    URL https://github.com/NUCLEAR-BOMB/option/archive/<id>
    URL_HASH SHA256=<hash>
)
FetchContent_MakeAvailable(option)
...
target_link_libraries(<target> PRIVATE option)
```

You can create archive URL with [Source code archive URLs][Source code archive URLs].

`<hash>` is optional but recommended. With this integrity check you can more secure pin the library version and avoid possible  data corruptions and "changed file in transit" scenarios.

> [!TIP]
> Use `FIND_PACKAGE_ARGS` optional argument in `FetchContent_Declare` to make it firstly try a call to [`find_package`][find_package].

## [ExternalProject][ExternalProject]

Download through git repository:

```cmake
include(ExternalProject)
ExternalProject_Add(
    option
    PREFIX "${CMAKE_BINARY_DIR}/option"
    GIT_REPOSITORY https://github.com/NUCLEAR-BOMB/option.git
    GIT_TAG <commit/tag>
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
)
ExternalProject_Get_Property(option source_dir)

target_compile_features(<target> PRIVATE cxx_std_17)
target_include_directories(<target> SYSTEM PRIVATE "${source_dir}/include")
```

As it is a header-only library it doesn't requires building, configuring, installation or testing. 

If you want to run test target before library is used:

```cmake
include(ExternalProject)
ExternalProject_Add(
    option
    PREFIX "${CMAKE_BINARY_DIR}/option"
    GIT_REPOSITORY https://github.com/NUCLEAR-BOMB/option.git
    GIT_TAG <commit/tag>
    TIMEOUT 10
    INSTALL_COMMAND ""
    TEST_COMMAND ${CMAKE_COMMAND} --build . --target run-option-test
)
```
> This will build the tests and run them with `run-option-test` target. Note that this may take some time to build tests.

> [!NOTE]
> You can change the download method to download through URL instead of through git repository with [`URL` and `URL_HASH` arguments][ExternalProject URL].

## [add_subdirectory][add_subdirectory]

You can directly embed the project and add it through CMake's [`add_subdirectory`][add_subdirectory] command.

```cmake
add_subdirectory(<path>)

target_link_libraries(<target> PRIVATE option)
```
> `<path>` is path to the root directory of the project (contains `CMakeLists.txt`).
> 
# How it works

The `opt::option` internally uses `opt::option_traits` which contains static methods that manipulates underlying value inside `opt::option`.
That's provide a way to store an empty state in a `opt::option` without using additional `bool` flag variable.

`opt::option_traits` also defines recursively `opt::option` type optimization and allows nested `opt::option` to have same size as the contained value.

Quick list of built-in size optimizations:
- **`bool`**: `bool` only uses `false` and `true` values, the remaining ones are used.
- **References and `std::reference_wrapper`**: around zero values are used.
- **Pointers**: for x64 noncanonical addresses, for x32 slightly less than maximum address (16-bit also supported).
- **Floating point**: negative signaling NaN with some payload values are used (other values are supported).
- **Polymorphic types**: unused vtable pointer values are used.
- **Reflectable types** (aggregate types)[^2]: the member with maximum number of unused value are used.
- **Pointers to members** (`T U::*`): some special offset range is used.
- **`std::tuple`, `std::pair`, `std::array` and any other tuple-like type**: the member with maximum number of unused value are used.
- **`std::basic_string_view` and `std::unique_ptr<T, std::default_delete<T>>`**: special values are used.
- **`std::basic_string` and `std::vector`**: uses internal implementation of the containers (supports [`libc++`][libc++], [`libstdc++`][libstdc++] and [`MSVC STL`][MSVC STL]).
- **Enumeration reflection**[^3]: automatic finds unused values (empty enums and flag enums are taken into account).
- **Manual reflection**: sentinel non-static data member (`.SENTINEL`), enumeration sentinel (`::SENTINEL`, `::SENTINEL_START`, `::SENTINEL_END`).
- **`opt::sentinel`, `opt::sentinel_f`, `opt::member`**: user-defined unused values.

See [**built-in traits**](./docs/markdown/builtin_traits.md) for more information.

# Compatibility with `std::optional`

The library is fully compatible with `std::optional`[^4], except:
- [`std::optional<T>::transform`][std::optional<T>::transform] is called `opt::option<T>::map`.
- Size of `std::optional` is not always the same as `opt::option`.
- `opt::option` supports reference types in contrast of `std::optional`.
- Some operations on types are not always `constexpr` depending on the option traits.
- [`std::bad_optional_access`][std::bad_optional_access] is `opt::bad_access`.
- [`std::nullopt`][std::nullopt]/[`std::nullopt_t`][std::nullopt_t] is `opt::none`/`opt::none_t`.
- `opt::option` supports [direct list initialization][direct-list-initialization] in it's constructors.
- Some methods/functions may not be `noexcept`.

You can replace `std::optional` with `opt::option`, taking into account that there are these exceptions.

# About undefined behavior

The library actively uses platform-dependent behavior to exploit unused object states.

Recommended using sanitizers (*`AddressSanitizer`* and *`UndefinedBehaviorSanitizer`*) to catch unexpected behavior.

> [!NOTE]
> The library doesn't break the strict aliasing rules. It uses `std::memcpy` to copy object bits instead of `reinterpret_cast`.

You can disable individual built-in traits to avoid using platform specific behavior for specific types.
Or you can disable built-in traits entirely with a macro definition.

[^1]: When possible uses [*Address Sanitizers*][AddressSanitizer] and [*Undefined Behavior Sanitizer*][UndefinedSanitizer].
Note that some compilers/versions have unstable sanitizer support, so the CI tests are disables that options.

[^2]: Requires either [**`boost.pfr`**][boost.pfr] or [**`ptr`**][pfr] library.

[^3]: Requires identifier `__PRETTY_FUNCTION__` or compiler built-in `__builtin_FUNCSIG()`.

[^4]: Including: conditionally enabled constructors and methods, propagating trivial constructors and operators, propagating deleted constructors, operators.


[cache locality]: https://en.wikipedia.org/wiki/Locality_of_reference
[std::optional<T>::transform]: https://en.cppreference.com/w/cpp/utility/optional/transform
[std::bad_optional_access]: https://en.cppreference.com/w/cpp/utility/optional/bad_optional_access
[std::nullopt]: https://en.cppreference.com/w/cpp/utility/optional/nullopt
[std::nullopt_t]: https://en.cppreference.com/w/cpp/utility/optional/nullopt_t
[std::in_place]: https://en.cppreference.com/w/cpp/utility/in_place
[INTERFACE add_library]: https://cmake.org/cmake/help/latest/command/add_library.html#interface-libraries
[ExternalProject URL]: https://cmake.org/cmake/help/latest/module/ExternalProject.html#url
[ExternalProject]: https://cmake.org/cmake/help/latest/module/ExternalProject.html
[Source code archive URLs]: https://docs.github.com/en/repositories/working-with-files/using-files/downloading-source-code-archives#source-code-archive-urls
[FetchContent]: https://cmake.org/cmake/help/latest/module/FetchContent.html
[target_link_libraries]: https://cmake.org/cmake/help/latest/command/target_link_libraries.html
[find_package]: https://cmake.org/cmake/help/latest/command/find_package.html
[std::optional]: https://en.cppreference.com/w/cpp/utility/optional
[std::optional monadic]: https://en.cppreference.com/w/cpp/utility/optional#Monadic_operations
[Rust Option]: https://doc.rust-lang.org/std/option/enum.Option.html
[direct-list-initialization]: https://en.cppreference.com/w/cpp/language/list_initialization
[std::in_place]: https://en.cppreference.com/w/cpp/utility/in_place
[AddressSanitizer]: https://clang.llvm.org/docs/AddressSanitizer.html
[UndefinedSanitizer]: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
[clang-tidy]: https://clang.llvm.org/extra/clang-tidy/
[boost.pfr]: https://github.com/boostorg/pfr
[pfr]: https://github.com/apolukhin/pfr_non_boost
[libc++]: https://libcxx.llvm.org/
[libstdc++]: https://gcc.gnu.org/onlinedocs/libstdc++/
[add_subdirectory]: https://cmake.org/cmake/help/latest/command/add_subdirectory.html
[MSVC STL]: https://github.com/microsoft/STL
