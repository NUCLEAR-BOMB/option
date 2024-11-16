
# List of features

## Methods overview

| Name                                                    | Description                                                                                                                                                                                                         |
|---------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [`try_emplace`](reference.md#try_emplace)               | Constructs the contained value in-place if it's empty                                                                                                                                                               |
| [`has_value_and`](reference.md#has_value_and)           | Returns `true` if the option has value the value of it matches a predicate                                                                                                                                          |
| [`take`](reference.md#take)                             | Takes the value out of the option                                                                                                                                                                                   |
| [`take_if`](reference.md#take_if)                       | Takes the value out of the option, but only if the predicate evaluates to `true`                                                                                                                                    |
| [`inspect`](reference.md#inspect)                       | Calls a function with a reference to the contained value if has a value                                                                                                                                             |
| [`get`](reference.md#get)                               | More verbose version of `operator*`                                                                                                                                                                                 |
| [`get_unchecked`](reference.md#get_unchecked)           | Access the contained value, without checking for it existence                                                                                                                                                       |
| [`value_or_throw`](reference.md#value_or_throw)         | More verbose version of `value`                                                                                                                                                                                     |
| [`value_or`](reference.md#value_or)                     | Updated version of `std::optional`'s `value_or` ([P2218: More flexible `optional::value_or()`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2218r0))                                                   |
| [`value_or_construct`](reference.md#value_or_construct) | Returns the contained value or constructs a new one from the arguments ([P2218: More flexible `optional::value_or()`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2218r0))                            |
| [`map_or`](reference.md#map_or)                         | Returns default value if option does not contain a value, or applies a function to the contained value                                                                                                              |
| [`map_or_else`](reference.md#map_or_else)               | Calls default functions if option does not contain a value, or applies a function to the contained value                                                                                                            |
| [`ptr_or_null`](reference.md#ptr_or_null)               | Returns a pointer to the contained value or `nullptr` otherwise                                                                                                                                                     |
| [`filter`](reference.md#filter)                         | Returns the contained value if `fn` returns `true`; otherwise, an empty option                                                                                                                                      |
| [`and_then`](reference.md#and_then)                     | Calls `fn` with contained value and returns the result; otherwise, returns an empty option                                                                                                                          |
| [`map`](reference.md#map)                               | Applies function to the contained value, or returns an empty option                                                                                                                                                 |
| [`or_else`](refserence.md#or_else)                      | Returns option if it contains a value, otherwise calls `fn` and returns the result                                                                                                                                  |
| [`begin`](reference.md#begin)                           | If contains a value, returns an iterator to the contained value. Otherwise, a past-the-end iterator ([P3168: Give *std::optional* Range Support](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3168r2)) |
| [`end`](reference.md#end)                               | Returns a past-the-end iterator ([P3168: Give *std::optional* Range Support](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3168r2))                                                                     |

## Functions overview

| Name | Description
| ---- | -----------
[`opt::zip`](reference.md#optzip) | Zips `options...` into `opt::option<std::tuple<...>>`
[`opt::zip_with`](reference.md#optzip_with) | Zips `options...` into `opt::option<std::tuple<...>>` with function `fn`
[`opt::option_cast`](reference.md#optoption_cast) | Casts `opt::option<From>` to `opt::option<To>`
[`opt::from_nullable`](reference.md#optfrom_nullable) | Returns a reference option if pointer is not equal to `nullptr`; otherwise, returns an empty option.
[`opt::as_option`](reference.md#optas_option) | Converts value into `opt::option<T>` by assigning the underlying value to `value`
[`opt::get`](reference.md#optget) | Returns `std::get` if option contains a value; otherwise, an empty option (for *tuple-like* types). Returns a reference option to the held of `std::variant` (for `std::variant`)
[`opt::io`](reference.md#optio) | Write to/read from stream. Allows specify case for a default value
[`opt::at`](reference.md#optat) | Reference option to the held value at `index` of the `container` if `index` is a valid index
[`opt::at_front`](reference.md#optat_front) | Reference option to the first element of the `container` if is's available
[`opt::at_back`](reference.md#optat_back) | Reference option to the last element of the `container` if it's available
[`opt::flatten`](reference.md#optflatten) | Flattens option up to the first level
[`opt::unzip`](reference.md#optunzip) | Unzips option that contains a *tuple-like* type, into the *tuple-like* object that contains values that wrapped into option
[`opt::lookup`](reference.md#optlookup) | Finds an element with key equivalent to `key`. If no such element is found, returns an empty option (works on associative containers)
[`opt::operator\|`](reference.md#operator-1) | Returns the option if it contains a value, otherwise returns second argument
[`opt::operator\|=`](reference.md#operator-2) | Copy assigns `right` to `left` if the `left` does not contain a value
[`opt::operator&`](reference.md#operatoramp) | Returns an empty option if `left` does not contain a value, or if `left` does, returns `right`
[`opt::operator^`](reference.md#operator-4) | Returns option that contains a value if exactly one of `left` and `right` contains a value, otherwise, returns an empty option

## Smaller size

The library uses `opt::option_traits` type to find and use wasted states of the contained value to not store an additional `bool` flag.

Thanks to this, the size of `opt::option` type with implemented `opt::option_traits` in many situation decreases drastically, and if used properly could lead to performance improvement.

The `opt::option_traits` can recursively search thought members of *tuple-like* types and types that could be reflected like aggregate types (requires `boost.pfr` or `pfr` libraries).

Also, `opt::option_traits` can help reduce size of nested options (e.g. `opt::option<opt::option<...>>`, etc).

> The list of built-in `opt::option_traits` you can see [here](builtin_traits.md). \
> More information about how `opt::option_traits` is implemented is available [here](custom_traits_guide.md).

> [!NOTE]
> `opt::option_traits` can be specialized on any user-defined types.

## Supports reference options

Unlike `std::optional`, `opt::option` allows lvalue and rvalue references to be contained in it.

You can avoid using inconvenient `std::reference_wrapper` and dangerous nullable pointers and use `opt::option<T&>` instead.

## Constructing aggregate types

If you using C++17 standard, the `opt::option` will automatically use *direct-list-initialization* for aggregate types (otherwise will fallback to using *direct-initialization*).

For C++20 and later, aggregate types can already be constructed with *direct-initialization*, so there is no difference with `std::optional` for this.

## Extended `constexpr`

Slightly extended `constexpr` support. Note that most operations that `std::optional` don't allow will also don't work with `opt::option` but if you lucky this might work.

## Constructors without `std::in_place`

Has the multiargument constructors without `std::in_place`.

For example:
```cpp
opt::option<std::pair<int, float>> val{1, 2.f};
```
Will compile successfully.

