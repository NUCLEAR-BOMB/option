
# Size optimizations

- [Size optimizations](#size-optimizations)
  - [`bool`](#bool)
  - [`std::reference_wrapper`](#stdreference_wrapper)
  - [`std::pair`](#stdpair)
  - [`std::tuple`](#stdtuple)
  - [`std::array`](#stdarray)
  - [`std::array<T, 0>`](#stdarrayt-0)
  - [Empty types](#empty-types)
  - [Reflectable types](#reflectable-types)
  - [Polymorphic types](#polymorphic-types)
  - [Pointers](#pointers)
  - [Floating point](#floating-point)
  - [Reference](#reference)
  - [`std::basic_string_view`](#stdbasic_string_view)
  - [`std::unique_ptr<T, std::default_delete<T>>`](#stdunique_ptrt-stddefault_deletet)
  - [Pointers to members](#pointers-to-members)
  - [`opt::option`](#optoption)

| Type                                         | max_level | level range                             |
| :------------------------------------------- | :-------- | :-------------------------------------- |
| `bool`                                       | 254       | [2,255]                                 |
| `std::reference_wrapper`                     | 256       | [0,255]                                 |
| References                                   | 255       | [0,254]                                 |
| Pointer (8 bytes)                            | 512       | [0xF8E1B1825D5D6C67,0xF8E1B1825D5D6E66] |
| Pointer (4 bytes)                            | 256       | [0xFFFFFEE0,0xFFFFFFE0]                 |
| Pointer (2 bytes)                            | 256       | [0xFEFF,0xFFFF]                         |
| floating point (8 bytes, signaling NaN)      | 256       | [0xFFF6C79F55B0898F,0xFFF6C79F55B08A8E] |
| floating point (8 bytes, quite NaN)          | 256       | [0xFFFBF26430BB3557,0xFFFBF26430BB3656] |
| floating point (4 bytes, signaling NaN)      | 256       | [0xFFBF69AF,0xFFBF6AAE]                 |
| floating point (4 bytes, quite NaN)          | 256       | [0xFFC3EFB5,0xFFC3F0B4]                 |
| `std::array<T, 0>`                           | 255       | [0,254]                                 |
| Empty type                                   | 255       | [0,254]                                 |
| Polymorphic type                             | 255       | [-89152,-88898]                         |
| `std::basic_string_view`                     | 255       | data(): -32186, size(): [0, 254]        |
| `std::unique_ptr<T, std::default_delete<T>>` | 255       | [-46509,-46255]                         |
| Pointers to members (4 bytes)                | 255       | [0xFFFFFC17,0xFFFFFD15]                 |
| Pointers to members (8 bytes)                | 255       | [0xFFFFFFFFD3B2F9B2,0xFFFFFFFFD3B2FAB0] |

## `bool`

Stores level value in [2,255] range (`0` and `1` are corresponds to [`false`][bool literals] and [`true`][bool literals] representations).

Size of [`bool`][bool] type is required to be at least 1 byte, but only uses two state ([`true`][bool literals] and [`false`][bool literals]).

Option traits assumes that [`true`][bool literals] is represented as `1` and [`false`][bool literals] as `0` (C++ standard specifies these values as implementation defined [[basic.fundamental]/10][basic.fundamental/10]).

## `std::reference_wrapper`

Stores level value in [0,255] range.

Assumes that [`std::reference_wrapper`][std::reference_wrapper] is implemented as pointer and that [0,255] is very unlikely value to be represented as valid [`std::reference_wrapper`][std::reference_wrapper].

Only enables when size of [`std::reference_wrapper<T>`][std::reference_wrapper] is equal to size of `T*`.

## `std::pair`

Stores level value in [`std::pair`][std::pair] member with higher `max_level` value ([`.first`][std::pair members] or [`.second`][std::pair members]).

This option trait simply wraps `get_level`, `set_level`, `after_constructor`, `after_assignment` static methods around selected [`std::pair`][std::pair] member.

Selected [`std::pair`][std::pair] member's option traits static methods are called with an argument `std::addressof(std::get<{index}>(*{pair}))`. \
Where `{index}` - index of selected [`std::pair`][std::pair] member, `{pair}` - passed pointer to [`std::pair`][std::pair] argument.

Enables `after_constructor` and `after_assignment` static methods's option trait only if selected member containes them correspondingly. 

## `std::tuple`

Stores level value in one of [`std::tuple`][std::tuple] element with higher `max_level` one ([`std::get<0>({tuple})`][std::tuple get], [`std::get<1>({tuple})`][std::tuple get], ...).

This option trait simply wraps `get_level`, `set_level`, `after_constructor`, `after_assignment` static methods around selected [`std::tuple`][std::tuple] element.

Selected [`std::tuple`][std::tuple] element's option traits static methods are called with an argument `std::addressof(std::get<{index}>(*{tuple}))`. \
Where `{index}` - index of selected [`std::tuple`][std::tuple] element, `{tuple}` - passed pointer to [`std::tuple`][std::tuple] argument.

Enables `after_constructor` and `after_assignment` static methods only if selected element's option trait containes them correspondingly.

## `std::array`

Stores level value in first [`std::array`][std::array] element.

This option trait simply wraps `get_level`, `set_level`, `after_constructor`, `after_assignment` static methods around first [`std::array`][std::array] element.

First [`std::array`][std::tuple] element's option traits static methods are called with an argument `std::addressof((*{array})[0])`. \
Where `{array}` - passed pointer to [`std::array`][std::tuple] argument.

Enables `after_constructor` and `after_assignment` static methods only if first element's option trait containes them correspondingly.

## `std::array<T, 0>`

Stores level value in [0,254] range.

Different C++ standard library implements empty `std::array` differently:
- MSVC STL: if `T` is default constructible or is implicitly default constructible contains array of size 1 of type `T`. Otherwise, special type `_Empty_array_element`, which is effectively an empty type.
- Clang libc++: array of type `_EmptyType`, with alignment of `T` with size of `T`. `_EmptyType` is `__empty` or `const __empty` if `T` is `const`.
- GCC libstdc++: empty type.

So this requires separate option trait.

## Empty types

Stores level value in [0,254] range.

Since C++ standard requires that size of a type must be at least 1 byte, we're storing level in unused space and indicate "in use" state when the value is 255.

Compiler optimizations often not copy these empty types (because there nothing to copy), breaking `opt::option`'s copy/move assigning/consructors.
This is avoided with `after_constructor` and `after_assignment` static methods in option trait, to force the compiler to copy option.
To trivially copy `opt::option`, internally uses [`union`][union] that comes with dummy type with same size of `T`.
This is forces the compiler to fully trivially copy stores empty type.

## Reflectable types

Stores level value in one of the members with higher `max_level`.

Uses `boost.pfr` functions to search for maximum `max_level` value and correspondingly use that member.
The type is considered "reflectable" only if `::boost::pfr::is_implicitly_reflectable_v<T, opt::option_tag>`, where `opt::option_tag` is a special tag.

This option trait simply wraps `get_level`, `set_level`, `after_constructor`, `after_assignment` static methods around selected member element.

Selected member's option traits static methods are called with an argument `std::addressof(::boost::pfr::get<{index}>(*{value}))`. \
Where `{index}` - index of selected member, `{value}` - passed pointer to `T` argument.

Enables `after_constructor` and `after_assignment` static methods only if selected member's option trait containes them correspondingly.

## Polymorphic types

Stores level value in [vtable][vtable] pointer.

This option trait assumes that [vtable][vtable] pointer is located in first `sizeof(void*)` bytes inside `T`, and uses a range of level values inside [vtable][vtable] pointer.

The type is considered "polymorphic" when the [`std::is_polymorphic_v<T>`][std::is_polymorphic] trait is `true` and (for safety reasons) if it's size is at least pointer's size.

## Pointers

Stores level value in unused or very unlikely addresses.

When size of pointer is:
- 8 bytes, this option trait uses [noncanonical addresses][canonical address] (address that is unused by hardware implementations).
- 4 bytes, a little bit lower address than maximun address (top addresses are usually kernel mapped) to avoid colliding window's [pseudo handlers][pseudo handle].
- 2 bytes, just maximun address.
- 1 byte, not supported (will use external "has value" flag).

## Floating point

Stores level value in very unlikely floating point's states.

Exploits a huge range in the majority mostly unused NaN payload.
This option trait tries to use negative [signaling NaN][signaling NaN] payload range which is almost impossible to reproduce externally.
Otherwise, tries to use negative [quite NaN][quite NaN] payload range. If that is not possible too, not uses this trait entirely.

Under normal circumstances, it is impossible to recreate level values: manipulating with [signaling NaNs][signaling NaN] causes floating point exception, hardware only generate [quite NaN][quite NaN] with pre-set payload.

## Reference

Stores level value in [0,254] range.

Since `opt::option` uses a pointer to store references, this option trait stores level in around [`nullptr`][nullptr] range.

## `std::basic_string_view`

Stores level value inside exposition only length of the character sequence if pointer to character sequence is equal to the sentinel.

If exposition only [`std::string_view`][std::string_view]'s pointer is equal to special tag, one of the (possible nested) `opt::option`'s is empty.
The [`std::string_view`][std::string_view] length is a level.

This option trait assumes that [`std::string_view`][std::string_view] not checks if passed range (pointer and length) is valid.

## `std::unique_ptr<T, std::default_delete<T>>`

Stores level value in [`std::unique_ptr`][std::unique_ptr] exposition only pointer to an object.

Since "pointer" type is defined by `Deleter`, this option trait only used when `Deleter` is [`std::default_delete`][std::default_delete].
Also custom `Deleter` may be not trivially copyable and could lead to unexpected behaviour inside `get_level`, `set_level` static methods (invocation of [`std::unique_ptr`][std::unique_ptr] constructor).

## Pointers to members

Stores level value as sentinel offset.

Pointers to data member are usually implemented as offset to type's member:
- MSVC: `uint32_t` offset.
- Itanium C++ ABI (Clang, GCC): `ptrdiff_t` offset.

Pointer to member functions more diverse than pointers to data members.
For Itanium C++ ABI (Clang, GCC) they are stored as a pair of `fnptr_t` offset and `ptrdiff_t` adjustment.

For MSVC it is stored:
- When type has none or single inheritance: `uintptr_t` offset.
- When type has multiple inheritance: `uintptr_t` offset, `int` adjustment.
- When type has *virtual inheritance*: `uintptr_t` offset, `int` adjustment, `int` virtual table index. 
- When type has *unknown inheritance*: `uintptr_t` offset, `int` adjustment, `int` offset to vptr, `int` virtual table index.

More details: [MSVC][pmf msvc], [Itanium C++ ABI][pmf itanium abi].

## `opt::option`

Recursively stores level value.

Underlying `opt::option` uses option traits for it's value:
- If it's level is `std::uintmax_t(-1)` or `0`, propagates `std::uintmax_t(-1)`.
- Uses next unused level to indicate it's empty state.

Underlying `opt::option` uses seperate "has value" flag:
- Stores it's level inside that "has value" flag.

[basic.fundamental/10]: https://eel.is/c++draft/basic.fundamental#10
[std::pair]: https://en.cppreference.com/w/cpp/utility/pair
[std::pair members]: https://en.cppreference.com/w/cpp/utility/pair#Member_objects
[std::tuple]: https://en.cppreference.com/w/cpp/utility/tuple
[std::tuple get]: https://en.cppreference.com/w/cpp/utility/tuple/get
[std::array]: https://en.cppreference.com/w/cpp/container/array
[union]: https://en.cppreference.com/w/cpp/language/union
[vtable]: https://en.wikipedia.org/wiki/Virtual_method_table
[canonical address]: https://en.wikipedia.org/wiki/X86-64#Canonical_form_addresses
[pseudo handle]: https://learn.microsoft.com/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentprocess
[quite NaN]: https://en.wikipedia.org/wiki/NaN#Quiet_NaN
[signaling NaN]: https://en.wikipedia.org/wiki/NaN#Signaling_NaN
[std::string_view]: https://en.cppreference.com/w/cpp/string/basic_string_view
[std::unique_ptr]: https://en.cppreference.com/w/cpp/memory/unique_ptr
[std::default_delete]: https://en.cppreference.com/w/cpp/memory/default_delete
[pmf itanium abi]: https://itanium-cxx-abi.github.io/cxx-abi/abi.html#member-function-pointers
[pmf msvc]: https://rants.vastheman.com/2021/09/21/msvc/
[nullptr]: https://en.cppreference.com/w/cpp/language/nullptr
[std::is_polymorphic]: https://en.cppreference.com/w/cpp/types/is_polymorphic
[std::reference_wrapper]: https://en.cppreference.com/w/cpp/utility/functional/reference_wrapper
[bool]: https://en.cppreference.com/w/cpp/language/types#Boolean_type
[bool literals]: https://en.cppreference.com/w/cpp/language/bool_literal
