
# Optimizations

- [Pointers](#pointers)
- [Enumerations](#enumerations)
- [Floating point](#floating-point)
- [Aggregates](#aggregates)
- [Polymorphic](#polymorphic)
- [Empty object](#empty-object)
- [`bool`](#bool)
- [`opt::option<bool>`](#optoptionbool)
- [`std::tuple`](#stdtuple)
- [`std::pair`](#stdpair)
- [`std::array`](#stdarray)
- [`std::unique_ptr`](#stdunique_ptr)
- [`std::reference_wrapper`](#stdreference_wrapper)
- [`std::string`](#stdstring)
- [`std::string_view`](#stdstring_view)
- [`std::vector`](#stdvector)

# Sentinel values
| Name | Value |
|------|-------|
| 64-bit pointer | `0x7FFFFFFFFFFFFFFF` |
| 32-bit pointer | `0xFFFFFFF3` |
| 16-bit pointer | `0xFFFF` |
| 8-bit pointer | `0xFF` |
| 32-bit floating point signaling NaN | `7fBF69AF` |
| 32-bit floating point quiet NaN | `7FC3EFB5` |
| 64-bit floating point signaling NaN | `7FF6C79F55B0898F` | 
| 64-bit floating point quiet NaN | `7FFBF26430BB3557` |
| Reference | `0` |
| Polymorphic | `uintptr_t(-1) - 14` |
| Empty object | `245` |
| Filled empty object | `0` |
| `bool` | `2` |
| `opt::option<bool>` | `4` |
| `std::reference_wrapper` | `0` |
| `std::basic_string` | *byte-filled with `255`* | 
| `std::basic_string_view` | *byte-filled with `255`* |
| `std::vector` | *byte-filled with `0`* |

### References
Unlike `std::optional`, allows to store reference types.

### Pointers
Stores a (probably) unused addresses as the sentinel value.
- On x64 instruction set (`sizeof(void*) == 8`) uses a [noncanonical address][] to indicate an empty state.
- On x32 instruction set (`sizeof(void*) == 4`) uses a slightly decreased 32-bit unsigned integer max to avoid Windows pseudo-handles collision.
- On 16-bit and 8-bit instruction set (`sizeof(void*) == 2` or `sizeof(void*) == 1`) uses a 16-bit or 8-bit unsigned integer max.

[noncanonical address]: https://read.seas.harvard.edu/cs161/2023/doc/memory-layout/

### Enumerations
Stores the enumerator with name `OPTION_EXPLOIT_UNUSED_VALUE` as the sentinel value.

### Floating point
Stores the quiet/signaling NaN payload as the sentinel value. \
Quiet NaN (qNaN) and signaling NaN (sNaN) have a wide range of typically unused values that we can use to store sentinel value. For example, [single precision floating point][] sNaN can have [payload][NaN floating point] in 23 bits non-zero number (the value zero encodes infinities).

See macro [`OPTION_USE_QUIET_NAN`](macros.md#option_use_quiet_nan)

[Single precision floating point]: https://en.wikipedia.org/wiki/Single-precision_floating-point_format
[NaN floating point]: https://en.wikipedia.org/wiki/NaN#Floating_point

### Aggregates
Stores the sentinel value in one of the aggregates members. \
If the **boost.pfr** library is available, will recursive search the aggregate type for types that satisfying `has_option_traits<T>`. If that member was found, use the `opt::option_traits<T>` for found member to manipulate the "is empty" flag. The search begins from first to last declared member in the aggregate type. If the static member function `unset_empty(T&)` does not exists in the `opt::option_traits<T>` for found type, the `unset_empty(T&)` will be replaced with an empty function.

### Polymorphic
Stores the sentinel value in virtual method table (VMT). \
Assumes that the first `sizeof(void*)` bytes (sizeof pointer) are VMT, and stores/checks the sentinel value in them.

### Empty object
The C++ standard does not allow objects of size 0.
We're using this property to store sentinel value in that unused space. \
If an object doesn't have any members, we can freely manipulate its underlying representation, since its methods can't possible read/modify its state.

### `bool`
Stores the sentinel value in a second unused bit. \
On most platforms size of `bool` is 1 byte, but used only single bit to store value. So we can exploit this flow to indicate an empty state in `opt::option`. This implementation uses bitwise AND `&` and bitwise OR `|` to manipulate an empty flag inside and does not default construct the `bool` value inside it, so the contained value of the empty constructed `opt::option<bool>` is in an indeterminate state. \
We using `&` and `|` to also implement [`opt::option<bool>`](#optoptionbool).

### `opt::option<bool>`
Stores the sentinel value in the third bit.

### `std::tuple`
Stores the sentinel value in one of its members. \
Recursive search the `std::tuple` for types that satisfying `has_option_traits<T>`. If one of its elements that satisfying `has_option_traits<T>` was found, use this element to manipulate the "is empty" flag. The search begins from left to right in the `std::tuple` template parameters. If the static member function `unset_empty(T&)` does not exists in the selected type, `unset_empty(T&)` will be replaced with an empty function.

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
Stores the `nullptr` (`0`) as the sentinel value.

### `std::string`
Sets each bit in the `std::string` object to `1` to indicate an empty state.

### `std::string_view`
Sets each bit in the `std::string_view` object to `1` to indicate an empty state.

### `std::vector`
Sets each bit in the `std::vector` object to `0` to indicate an empty state.
