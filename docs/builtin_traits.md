
# Size optimizations

- [`bool`](#bool)
- [`std::reference_wrapper`](#stdreference_wrapper)
- [`std::pair`](#stdpair)
- [`std::tuple`](#stdtuple)
- [Tuple-like types](#tuple-like-types)
- [`std::array`](#stdarray)
- [Reflectable types](#reflectable-types)
- [Polymorphic types](#polymorphic-types)
- [Pointers](#pointers)
- [Floating point](#floating-point)
- [Reference](#reference)
- [`std::basic_string_view`](#stdbasic_string_view)
- [`std::unique_ptr<T, std::default_delete<T>>`](#stdunique_ptrt-stddefault_deletet)
- [`std::basic_string`](#stdbasic_string)
- [`std::vector`](#stdvector)
- [`SENTINEL` member](#sentinel-member)
- [Enumeration `SENTINEL`](#enumeration-sentinel)
- [Enumeration `SENTINEL_START`](#enumeration-sentinel_start)
- [Enumeration `SENTINEL_START` and `SENTINEL_END`](#enumeration-sentinel_start-and-sentinel_end)
- [Enumeration](#enumeration)
- [Pointers to members](#pointers-to-members)
- [`opt::option`](#optoption)

| Type                                            | max_level                        | level range                             |
| :---------------------------------------------- | :------------------------------- | :-------------------------------------- |
| `bool`                                          | 254                              | [2,255]                                 |
| `std::reference_wrapper`                        | 256                              | [0,255]                                 |
| References                                      | 255                              | [0,254]                                 |
| Pointer (8 bytes)                               | 512                              | [0xF8E1B1825D5D6C67,0xF8E1B1825D5D6E66] |
| Pointer (4 bytes)                               | 32                               | [0XFFFFFFC0,0XFFFFFFDF]                 |
| floating point (8 bytes, signaling NaN)         | 256                              | [0xFFF6C79F55B0898F,0xFFF6C79F55B08A8E] |
| floating point (8 bytes, quite NaN)             | 256                              | [0xFFFBF26430BB3557,0xFFFBF26430BB3656] |
| floating point (4 bytes, signaling NaN)         | 256                              | [0xFFBF69AF,0xFFBF6AAE]                 |
| floating point (4 bytes, quite NaN)             | 256                              | [0xFFC3EFB5,0xFFC3F0B4]                 |
| Polymorphic type                                | 255                              | [-89152,-88898]                         |
| `std::basic_string_view`                        | 255                              | data(): [-32185,-31931]                 |
| `std::unique_ptr<T, std::default_delete<T>>`    | 255                              | [-46509,-46255]                         |
| `std::basic_string`                             | 255                              | capacity(): 0, size(): [0,254]          |
| `std::vector`                                   | 255                              | data(): 1, {data() + size()}: [0,245]   |
| Pointers to members (4 bytes)                   | 255                              | [0xFFFFFC17,0xFFFFFD15]                 |
| Pointers to members (8 bytes)                   | 255                              | [0xFFFFFFFFD3B2F9B2,0xFFFFFFFFD3B2FAB0] |
| `SENTINEL` member                               | [*](#sentinel-member)            | [1,[*](#sentinel-member)]               |
| Enumeration `SENTINEL`                          | 1                                | `::SENTINEL`                            |
| Enumeration `SENTINEL_START`                    | [*](#enumeration-sentinel_start) | [`::SENTINEL_START`,-1]                 |
| Enumeration `SENTINEL_START` and `SENTINEL_END` | [*](#enumeration-sentinel_start-and-sentinel_end) | [`::SENTINEL_START`,`::SENTINEL_END`] |
| Enumeration `SENTINEL_START` and `SENTINEL_END` | [*](#enumeration)                | [*](#enumeration)                       |
| `opt::option`                                   | `max_level` - 1                  | [*](#optoption)                         |

## `bool`

Stores level value in [2,255] range (`0` and `1` are corresponds to [`false`][bool literals] and [`true`][bool literals] representations).

Size of [`bool`][bool] type is required to be at least 1 byte, but only uses two state ([`true`][bool literals] and [`false`][bool literals]).

Option traits assumes that [`true`][bool literals] is represented as `1` and [`false`][bool literals] as `0` (C++ standard specifies these values as implementation defined [[basic.fundamental]/10][basic.fundamental/10]).

## `std::reference_wrapper`

Assumes that [`std::reference_wrapper`][std::reference_wrapper] is implemented as pointer and that [0,255] is very unlikely value to be represented as valid [`std::reference_wrapper`][std::reference_wrapper].

Only enables when size of [`std::reference_wrapper<T>`][std::reference_wrapper] is equal to size of `T*`.

## `std::pair`

Stores level value in [`std::pair`][std::pair] member with higher `max_level` value ([`.first`][std::pair members] or [`.second`][std::pair members]).

This option trait simply wraps `get_level`, `set_level` static methods around selected [`std::pair`][std::pair] member.

Selected [`std::pair`][std::pair] member's option traits static methods are called with an argument `std::addressof(std::get<{index}>(*{pair}))`. \
Where `{index}` - index of selected [`std::pair`][std::pair] member, `{pair}` - passed pointer to [`std::pair`][std::pair] argument.

## `std::tuple`

Stores level value in one of [`std::tuple`][std::tuple] element with higher `max_level` one ([`std::get<0>({tuple})`][std::tuple get], [`std::get<1>({tuple})`][std::tuple get], ...).

This option trait simply wraps `get_level`, `set_level` static methods around selected [`std::tuple`][std::tuple] element.

Selected [`std::tuple`][std::tuple] element's option traits static methods are called with an argument `std::addressof(std::get<{index}>(*{tuple}))`. \
Where `{index}` - index of selected [`std::tuple`][std::tuple] element, `{tuple}` - passed pointer to [`std::tuple`][std::tuple] argument.

## Tuple-like types

Stores level value in one of the element that has higher `max_level` value.

The type is considered tuple-like if `std::tuple_size<T>::value` is a well-formed constant expression.
The elements types are obtained using `std::tuple_element<I, T>::type` type.
And references to them are from:
- `x.get<I>()`, if it is a valid expression.
- `get<I>(x)`, where `get` is looked up by argument-dependent lookup.

`T` is the type of the tuple-like type.
`x` is (possible `const`) reference to `T`, which is the contained value of `opt::option`.
`I` is `std:size_t` constant expression that denotes index of the tuple-like element.

This option trait wraps `get_level`, `set_level`, static methods around selected tuple-like type's element.

## `std::array`

Stores level value in first [`std::array`][std::array] element.

This option trait simply wraps `get_level`, `set_level` static methods around first [`std::array`][std::array] element.

First [`std::array`][std::array] element's option traits static methods are called with an argument `std::addressof((*{array})[0])`. \
Where `{array}` - passed pointer to [`std::array`][std::array] argument.

> [!NOTE]
> If `std::array`'s size is 0, this option trait is not used.

## Reflectable types

Uses `boost.pfr` functions to search for maximum `max_level` value and correspondingly use that member.
The type is considered "reflectable" only if `::boost::pfr::is_implicitly_reflectable_v<T, opt::option_tag>`, where `opt::option_tag` is a special tag.

This option trait simply wraps `get_level`, `set_level` static methods around selected member element.

Selected member's option traits static methods are called with an argument `std::addressof(::boost::pfr::get<{index}>(*{value}))`. \
Where `{index}` - index of selected member, `{value}` - passed pointer to `T` argument.

## Polymorphic types

Stores level value in [vtable][vtable] pointer.

This option trait assumes that [vtable][vtable] pointer is located in first `sizeof(void*)` bytes inside `T`, and uses a range of level values inside [vtable][vtable] pointer.

The type is considered "polymorphic" when the [`std::is_polymorphic_v<T>`][std::is_polymorphic] trait is `true` and (for safety reasons) if it's size is at least pointer's size.

## Pointers

Stores level value in unused or very unlikely addresses.

When size of pointer is:
- 8 bytes, this option trait uses [noncanonical addresses][canonical address] (address that is unused by hardware implementations).
- 4 bytes, a little bit lower address than maximun address (top addresses are usually kernel mapped) to avoid colliding window's [pseudo handlers][pseudo handle].
- Otherwise, not supported (will use external "has value" flag).

## Floating point

Stores level value in very unlikely floating point's states.

Exploits a range of values in the majority mostly unused NaN payload.
This option trait tries to use negative [signaling NaN][signaling NaN] payload range which is almost impossible to reproduce externally.
Otherwise, tries to use negative [quiet NaN][quiet NaN] payload range. If that is not possible too, not uses this trait entirely.

Under normal circumstances, it is impossible to recreate level values: manipulating with [signaling NaNs][signaling NaN] causes floating point exception, hardware only generate [quiet NaN][quiet NaN] with pre-set payload.

> [!NOTE]
> On x32, it is always uses quiet NaN to avoid producing floating point hardware exceptions (although not sure why).

These is also an assumption that copying NaN does preserve its bit representation (which is not guaranteed by the [IEEE 754][IEEE 754] standard).

## Reference

Since `opt::option` uses a pointer to store references, this option trait stores level in around [`nullptr`][nullptr] range.

## `std::basic_string_view`

Stores level value inside internal pointer to character sequence.

This option trait assumes that [`std::string_view`][std::string_view] not checks if passed range (pointer and length) is valid.

## `std::unique_ptr<T, std::default_delete<T>>`

Stores level value in [`std::unique_ptr`][std::unique_ptr] exposition only pointer to an object.

Since "pointer" type is defined by `Deleter`, this option trait only used when `Deleter` is [`std::default_delete`][std::default_delete].
Also custom `Deleter` may be not trivially copyable and could lead to unexpected behaviour inside `get_level`, `set_level` static methods (invocation of [`std::unique_ptr`][std::unique_ptr] constructor).

## `std::basic_string`

Stores level value in the internal (standard library defined) representation of the `std::basic_string`.

The template parameter `Allocator` must satisfy `std::is_empty_v` and `!std::is_final_v` in order to enable this option trait.

## `std::vector`

Stores level value in the internal (standard library defined) representation of the `std::vector`.

The template parameter `Allocator` must satisfy `std::is_empty_v` and `!std::is_final_v` in order to enable this option trait.

## `SENTINEL` member

Stores level value in the `.SENTINEL` data member inside provided type. 

Requires an data member `SENTINEL`.
It must be non-`const`, satisfy `std::is_unsigned_v` and be accessible outside the type (the expression `std::declval<T&>().SENTINEL` must be valid).

The `max_level` is defined by the expression `~T(0)`, where `T` is type of `SENTINEL` member.

## Enumeration `SENTINEL`

Uses the `SENTINEL` enumerator to indicate an "is empty" state.

Requires an enumerator named `SENTINEL`.

The `max_level` value is 1 (since it only uses that particular value).

## Enumeration `SENTINEL_START`

Uses range that starts at (including) `SENTINEL_START` and ends at the maximum value of an underlying type of the enumeration.

Requires an enumerator named `SENTINEL_START`.

The `max_level` value is `(underlying max) - (sentinel start) + 1`,
where `(underlying max)` is the maximum value of an underlying type of the enumeration,
`(sentinel start)` is the `SENTINEL_START` but as underlying type.

## Enumeration `SENTINEL_START` and `SENTINEL_END`

Uses range that starts at (including) `SENTINEL_START` and ends at (including) `SENTINEL_END`

Requires an enumerators named `SENTINEL_START` and `SENTINEL_END`.

The `max_level` value is `(sentinel end) - (sentinel start) + 1`,
where `(sentinel start)` is the `SENTINEL_START` but as underlying type,
`(sentinel end)` is the `SENTINEL_END` but as underlying type.

## Enumeration

Uses range of unused enumeration values.

Searches for the last defined enumerator:
- If found, round the value to the next power of 2 (to avoid collision with flag enumerations) and use it as range beginning.
- If not, assume that enumeration is used as an integral type (and therefore all the value are used), do not use it.

The search starts from the value:
- If size of enumeration is 1 byte, 255.
- Otherwise, 1023.

The `max_level` value is difference between search start and rounded the found value.

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

Underlying `opt::option` uses next available level value.

For example:
- `opt::option<X>` will use `0` level,
- In `opt::option<opt::option<X>>`, outer `opt::option` will use `1` level and inner `opt::option` will use `0` level.
- ...

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
[quiet NaN]: https://en.wikipedia.org/wiki/NaN#Quiet_NaN
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
[std::is_empty]: https://en.cppreference.com/w/cpp/types/is_empty
[IEEE 754]: https://en.wikipedia.org/wiki/IEEE_754
