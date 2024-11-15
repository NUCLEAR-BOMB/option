
# Macros

> [!WARNING]
> Defining macro that not listed in the following list will possible result code break between different versions.

### OPTION_USE_QUIET_NAN
*expects:* `boolean`, *default:* `false`

If `true` use quiet NaN as the sentinel value instead of signaling NaN in [`opt::option_traits`][option-traits] for floating point numbers (`float`, `double`); otherwise, use signaling NaN as the sentinel in [`opt::option_traits`][option-traits]. We using the signaling NaN as default sentinel value for minimizing the chance of a collision between the contained value and the sentinel value. If on your platform the `opt::option` raises the invalid operation exception when using signaling NaN recommended to define this macro to `true`.

### OPTION_VERIFY
*parameters:* `expression`, `message`

In the debug configuration (`NDEBUG` is not defined), by default, [`std::fprintf`][cpp-fprintf] error message to [`stderr`][cpp-stderr] and causes a debug break if the `expression` evaluates to `false`. In the release configuration (`NDEBUG` is defined) will expand to `if (expression) {} else { [unreachable]; }`, where the `[unreachable]` is a specific point in the program that cannot be reached ([`__assume(0)`][msvc-assume], [`__builtin_unreachable()`][gcc-unreachable]). **NOTE:** if this macro is defined by the user it will not provide switching logic for debug/release configurations; the user must himself provide it if he needs it. Used in `opt::option<T>::get`, `opt::option<T>::operator*` and `opt::option<T>::operator->`.

[msvc-assume]: https://learn.microsoft.com/en-us/cpp/intrinsics/assume
[gcc-unreachable]: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-_005f_005fbuiltin_005funreachable
[cpp-fprintf]: https://en.cppreference.com/w/cpp/io/c/fprintf
[cpp-stderr]: https://en.cppreference.com/w/cpp/io/c/std_streams

### OPTION_USE_BUILTIN_TRAITS
*expects:* `boolean`, *default:* `true`

If `true` enables [`opt::option_traits`][option-traits] to use library builtin optimizations; otherwise, only user defined [`opt::option_traits`][option-traits] will be used.


### OPTION_FORWARD_DECLARE_STD
*expects:* `boolean`, *default:* `true`

If `true` provides a forward declaration of some standard library header (like `<string>`, `<vector>`, etc.).
Otherwise just includes headers.

> [!NOTE]
> This kind of action is forbidden by the standard. 
> If you have compilation error related to a forward declaration of standard library, set this macro to `1`.

### OPTION_CONSUMED_ANNOTATION_CHECKING
*expects:* `boolean`, *default:* `false`

Enables support for Clang's [Consumed Annotation Checking][Consumed Annotation Checking] (experimental) for `opt::option`.

> [!NOTE]
> This is really not recommended to use it because [`-Wconsumed`][Wconsumed] gives too many false positives.

## **boost.pfr**/**pfr** library related

### OPTION_PFR_FILE
*expects:* `"path"` or `<path>`

Optionally specifies the [`#include`][cpp-include] path for the [**boost.pfr**][boost-pfr]/[**pfr**][pfr] library.
You can define this macro to specify a path to the [**boost.pfr**][boost-pfr]/[**pfr**][pfr] library.

If this macro is not defined, will try to check if the [**boost.pfr**][boost-pfr] or [**pfr**][pfr] libraries header is avaliable.
Otherwise, will silently continue without using these library features.

If this macro is defined by user, will check if `OPTION_PFR_FILE` is exists and corrent. If not, will [`#error`][cpp-error].


### OPTION_USE_PFR
*expects:* `boolean`, *default:* `true`

Will make [`#error`][cpp-error] if this macro is defined by the user and it is evaluates to `true`, but `__has_include(OPTION_PFR_FILE)` is `false`.
If the [**boost.pfr**][boost-pfr]/[**pfr**][pfr] library is not supported, will silently continue without actually using the library features.

## **libassert** library related

### OPTION_LIBASSERT_FILE
*expects:* `"path"` or `<path>`, *default:* `<libassert/assert.hpp>`

Defines [`#include`][cpp-include] path for the [**libassert**][libassert] library.

### OPTION_USE_LIBASSERT
*expects:* `boolean`, *default:* `true`

Define [`OPTION_VERIFY`](#option_verify) to `LIBASSERT_ASSUME` macro and don't use default.

[boost-pfr]: https://www.boost.org/doc/libs/1_83_0/doc/html/boost_pfr.html
[pfr]: https://github.com/apolukhin/pfr_non_boost/tree/master
[cpp-include]: https://en.cppreference.com/w/cpp/preprocessor/include
[cpp-error]: https://en.cppreference.com/w/cpp/preprocessor/error
[libassert]: https://github.com/jeremy-rifkin/libassert
[Consumed Annotation Checking]: https://clang.llvm.org/docs/AttributeReference.html#consumed-annotation-checking
[Wconsumed]: https://clang.llvm.org/docs/DiagnosticsReference.html#wconsumed
[option-traits]: ./reference.md#optoption_traits
