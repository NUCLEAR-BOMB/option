
# Optimizations

### `bool`
Stores the sentinel value in a second unused bit. \
On most platforms size of `bool` is 1 byte, but used only single bit to store value.
So we can exploit this flow to indicate an empty state in `opt::option`.
This implementation uses bitwise AND `&` and bitwise OR `|` to manipulate an empty flag inside.
We using `&` and `|` to also implement [`opt::option<bool>`](#optoptionbool).
> [!NOTE]
> The sentinel value is `2`.

### `opt::option<bool>`
Stores the sentinel value in a second third bit.
> [!NOTE]
> The sentinel value is `4`.

### Pointers
Stores a (probably) unused addresses as the sentinel value.
- On x64 instruction set (`sizeof(void*) == 8`) uses a [noncanonical address][] to indicate an empty state.
- On x32 instruction set (`sizeof(void*) == 4`) uses a sligtly decreased 32-bit unsigned integer max to avoid Windows pseudo-handles collision.
- On 16-bit and 8-bit instruction set (`sizeof(void*) == 2` or `sizeof(void*) == 1`) uses a 16-bit or 8-bit unsigned integer max.
> [!NOTE]
> The sentinel value on x64 is `0x7FFFFFFFFFFFFFFF`, on x32 is `0xFFFFFFF3`, on 16-bit instruction set is `0xFFFF`, on 8-bit instruction set is `0xFF`.

[noncanonical address]: https://read.seas.harvard.edu/cs161/2023/doc/memory-layout/

### Enumerations
Stores the unused enumerator as the sentinel value.
- If the **magic_enum** library is available check if max/min value of underlying enumeration type (`std::numeric_limits<T>::man()`, `std::numeric_limits<T>::min()`) is not contained in enumeration using `magic_enum::enum_contains`, if is not, use this value as the sentinel; otherwise, ignore this optimization.
- If the **magic_enum** library is not available try to use enumerator with name "`OPTION_EXPLOIT_UNUSED_VALUE`"
as the sentinel value.

### Floating point
Stores the quiet/signaling NaN payload as the sentinel value. \
Quiet NaN (qNaN) and signaling NaN (sNaN) have a wide range of typically unused values that we can use to store sentinel value. For example, [single precision floating point][] sNaN can have [payload][NaN floating point] in 23 bits non-zero number (the value zero encodes infinities).

See also macro [`OPTION_USE_QUIET_NAN`](macros.md#option_use_quiet_nan)

> [!NOTE]
> The signaling NaN sentinel value for 32-bit floating point number is `7fBF69AF`, for 64-bit floating point number is `7FF6C79F55B0898F`. \
> The quiet NaN sentinel value for 32-bit floating point number is `7FC3EFB5`, for 64-bit floating point number is `7FFBF26430BB3557`.

[Single precision floating point]: https://en.wikipedia.org/wiki/Single-precision_floating-point_format
[NaN floating point]: https://en.wikipedia.org/wiki/NaN#Floating_point

### `std::tuple`
Stores the sentinel value in one of its members. \
Recursive search the `std::tuple` for types that satisfying `has_option_traits<T>`. If one of its elements that satisfying `has_option_traits<T>` was found, use this element to manipulate the "is empty" flag. The search begins from left to right in the `std::tuple` template parameters. If function member `unset_empty(T&)` does not exists in the selected type, `unset_empty(T&)` will be replaced with an empty function.

### `std::pair`
Stores the sentinel value in `T1` (first) or `T2` (second) type. \
If the `T1` type satisfies `has_option_traits<T>`, then it is selected as a flag manipulator type; otherwise, the `T2` type is selected.

### `std::array`
Stores the sentinel value in first element of the array. \
The `std::array` size must be at least be 1. Uses the first element of the `std::array` as a flag manipulator type.

### `std::unique_ptr`
Stores an unused address as the sentinel value. \
Uses a `opt::option_traits<T*>` type to manipulate the pointer inside `std::unique_ptr`.

See [Pointers](#pointers) section.

### `std::reference_wrapper`
Stores the `nullptr` (`0`) as the sentinel value. \
