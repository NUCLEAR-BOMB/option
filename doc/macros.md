
# Macros

> [!WARNING]
> Defining macro that not listed in the following list will possible result code break between different versions.

- [OPTION_USE_QUIET_NAN](#option_use_quiet_nan)
- [OPTION_VERIFY](#option_verify)
- [**magic_enum** library related](#magic_enum-library-related)
    - [OPTION_MAGIC_ENUM_FILE](#option_magic_enum_file)
    - [OPTION_USE_MAGIC_ENUM](#option_use_magic_enum)
- [**boost.pfr** library related](#boostpfr-library-related)
    - [OPTION_BOOST_PFR_FILE](#option_boost_pfr_file)
    - [OPTION_USE_BOOST_PFR](#option_use_boost_pfr)

### OPTION_USE_QUIET_NAN
*expects:* `boolean`, *default:* `false`

If `true` use [quiet NaN][] as the sentinel value instead of [signaling NaN][] in `opt::option_traits` for floating point numbers (`float`, `double`); otherwise, use [signaling NaN][] as the sentinel in `opt::option_traits`. We using the [signaling NaN][] as default sentinel value for minimizing the chance of a collision between the contained value and the sentinel value. If on your platform the `opt::option` raises the invalid operation exception when using [signaling NaN][] recommended to define this macro to `true`.

### OPTION_VERIFY
*parameters:* `expression`, `message`

In the debug configuration (`NDEBUG` is not defined), by default, `std::fprintf` error message to `stderr` and causes a debug break if the `expression` evaluates to `false`. In the release configuration (`NDEBUG` is defined) will expand to `if (expression) {} else { [unreachable]; }`, where the `[unreachable]` is a specific point in the program that cannot be reached (`__assume(0)`, `__builtin_unreachable()`). **NOTE:** if this macro is defined by the user it will not provide switching logic for debug/release configurations; the user must himself provide it if he needs it. Used in `opt::option<T>::get`, `opt::option<T>::operator*` and `opt::option<T>::operator->`.

## **magic_enum** library related

### OPTION_MAGIC_ENUM_FILE
*expects:* `"path"` or `<path>`, *default:* `<magic_enum.hpp>`

Optionally specifies the `#include` path for **magic_enum** library. You can define this macro to specify a path to **magic_enum** library. If the **magic_enum** library was not found with `__has_include(OPTION_MAGIC_ENUM_FILE)` will silently continue without using the library features.

### OPTION_USE_MAGIC_ENUM
*expects:* `boolean`, *default:* `true`

Will make `#error` if this macro is defined by the user and it is evaluates to `true`, but `__has_include(OPTION_MAGIC_ENUM_FILE)` is `false`. If the **magic_enum** library is not supported by the compiler (not defined `MAGIC_ENUM_SUPPORTED`) will silently continue without actually using the library features.

[quiet NaN]: https://en.wikipedia.org/wiki/NaN#Quiet_NaN
[signaling NaN]: https://en.wikipedia.org/wiki/NaN#Signaling_NaN

## **boost.pfr** library related

### OPTION_BOOST_PFR_FILE
*expects:* `"path"` or `<path>`, *default:* `<boost/pfr.hpp>`

Optionally specifies the `#include` path for the **boost.pfr** library. You can define this macro to specify a path to the **boost.pfr** library. If the **boost.pfr** library was not found with `__has_include(OPTION_BOOST_PFR_FILE)`, will silently continue without using the library features.

### OPTION_USE_BOOST_PFR
*expects:* `boolean`, *default:* `true`

Will make `#error` if this macro is defined by the user and it is evaluates to `true`, but `__has_include(OPTION_BOOST_PFR_FILE)` is `false`. If the **boost.pfr** library is not supported by the compiler (defined `BOOST_PFR_NOT_SUPPORTED`), will silently continue without actually using the library features.
