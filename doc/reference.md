
# Reference

- [Member functions](#member-functions)
    - [Constructor](#constructor)
    - [Destructor](#destructor)
    - [`operator=`](#operator)
    - [`reset`](#reset)
    - [`emplace`](#emplace)
    - [`has_value`, `operator bool`](#has_value-operator-bool)
    - [`has_value_and`](#has_value_and)
    - [`take`](#take)
    - [`take_if`](#take_if)
    - [`insert`](#insert)
    - [`inspect`](#inspect)
    - [`get`, `operator*`, `operator->`](#get-operator-operator)
    - [`get_unchecked`](#get_unchecked)
    - [`value`, `value_or_throw`](#value-value_or_throw)
    - [`value_or`](#value_or)
    - [`value_or_default`](#value_or_default)
    - [`map_or`](#map_or)
    - [`map_or_else`](#map_or_else)
    - [`ptr_or_null`](#ptr_or_null)
    - [`filter`](#filter)
    - [`flatten`](#flatten)
    - [`and_then`](#and_then)
    - [`map`](#map)
    - [`or_else`](#or_else)
    - [`assume_has_value`](#assume_has_value)
    - [`unzip`](#unzip)
    - [`replace`](#replace)
- [Non-member functions](#non-member-functions)
    - [`zip`](#zip)
    - [`zip_with`](#zip_with)
    - [`option_cast`](#option_cast)
    - [`operator|`](#operator-1)
    - [`operator|=`](#operator-2)
    - [`operator&`](#operator-3)
    - [`operator^`](#operator-4)
    - [`operator==`](#operator-5)
    - [`operator!=`](#operator-6)
    - [`operator<`](#operator-7)
    - [`operator<=`](#operator-8)
    - [`operator>`](#operator-9)
    - [`operator>=`](#operator-10)
- [Helpers](#Helpers)
    - [`std::hash<opt::option>`](#stdhashoptoption)
    - [`none_t`](#none_t)
    - [`none`](#none)
    - [`bad_access`](#bad_access)
    - [`option_traits`](#option_traits)

## Member functions

### Constructor

```cpp
constexpr option() noexcept;
```
Default constructor. \
Constructs an `opt::option` object that does not contain a value.
- *Trivial* when the `opt::option` is specialized for reference types (`std::is_reference_v<T>`).
- *Postcondition:* `has_value() == false`.

```cpp
constexpr option(opt::none_t) noexcept;
```
Constructs an `opt::option` object that does not contain a value.
- *Postcondition:* `has_value() == false`.

```cpp
constexpr option(const option& other) noexcept(/*see below*/);
```
Copy constructor. \
Copy constructs an object of type `T` using *direct-list-initialization* with the expression `other.get()` if `other` contains a value. If `other` does not contain a value, construct an empty `opt::option` object instead.
- *`noexcept`* when `std::is_nothrow_copy_constructible_v<T>`.
- *Deleted* when `!std::is_copy_constructible_v<T>`.
- *Trivial* when `std::is_trivially_copy_constructible_v<T>`.
- *Postcondition:* `has_value() == other.has_value()`.

```cpp
constexpr option(option&& other) noexcept(/*see below*/);
```
Move constructor. \
Move constructs an object of type `T` using *direct-list-initialization* with the expression `std::move(other.get())` if `other` contains a value. If `other` does not contain a value, construct an empty `opt::option` object instead.
- *`noexcept`* when `std::is_nothrow_move_constructible_v<T>`.
- *Deleted* when `!std::is_move_constructible_v<T>`
- *Trivial* when `std::is_trivially_move_constructible_v<T>`.
- *Postcondition:* `has_value() == other.has_value()`.

> [!IMPORTANT]
> After move, `other` still holds a value (if it had before), but the value itself is moved from.

```cpp
template<class U = T>
constexpr explicit(/*see below*/) option(U&& value) noexcept(/*see below*/);
```
Constructs an `opt::option` object that *contains a value*. Initializes a contained object of type `T` using *direct-list-initialization* with the expression `std::forward<U>(value)`.
- *`noexcept`* when `std::is_nothrow_constructible_v<T, U>`.
- *`explicit`* when `!std::is_convertible_v<U&&, T>`.
- *Enabled* when the following conditions are true:
    - `std::is_constructible_v<T, U&&>`.
    - `!std::is_same_v<remove_cvref<U>, opt::option<T>>`.
    - `!(std::is_same_v<remove_cvref<T>, bool> && opt::is_option<remove_cvref<U>>)`. \
    Where `remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type `X`.

```cpp
template<class First, class... Args>
constexpr option(First&& first, Args&&... args) noexcept(/*see below*/);
```
Constructs an `opt::option` object that contains a value, initialized an object of type `T` using *direct-list-initialization* with the arguments `std::forward<First>(first), std::forward<Args>(args)...`.
- *`noexpect`* when `std::is_nothrow_constructible_v<T, First, Args...>`.
- *Enabled* when the following conditions are true:
    - `std::is_constructible_v<T, First, Args...> || is_direct_list_initializable<T, First, Args...>`.
    - `!std::is_same_v<remove_cvref<First>, opt::option<T>>`. \
    Where `remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type X. \
    Where `is_direct_list_initializable<X, XArgs...>` is a metafunction, that checks if a type `X` can be *direct-list-initialized* with the arguments of types `XArgs...`.

```cpp
template<class U>
constexpr explicit(/*see below*/) option(const option<U>& other) noexcept(/*see below*/);
```
Converting copy constructor. \
Constructs an object of type `T` using *direct-list-initialization* with the expression `other.get()` if `other` contains a value. If `other` does not contain a value, construct an empty `opt::option` object instead.
- *`noexpect`* when `std::is_nothrow_constructible_v<T, const U&>`.
- *`explicit`* when `!std::is_convertible_v<const U&, T>`.
- *Enabled* when the following conditions are true:
    - `std::is_constructible_v<T, const U&>`.
    - `!std::is_same_v<remove_cvref<T>, bool>`.
    - `!is_constructible_from_option<T, U>)`. \
    Where `remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type X. \
    Where `is_constructible_from_option<X, Y>` is a metafunction, that checks if type `X` is constructible or convertible from any expression of type `opt::option<Y>` (possibly `const`), i.e., in the following there is at least one `true`:
        - `std::is_constructible_v<X, opt::option<Y>&>`.
        - `std::is_constructible_v<X, const opt::option<Y>&>`.
        - `std::is_constructible_v<X, opt::option<Y>&&>`.
        - `std::is_constructible_v<X, const opt::option<Y>&&>`.
        - `std::is_convertible_v<opt::option<Y>&, X>`.
        - `std::is_convertible_v<const opt::option<Y>&, X>`.
        - `std::is_convertible_v<opt::option<Y>&&, X>`.
        - `std::is_convertible_v<const opt::option<Y>&&, X>`.
- *Postcondition:* `has_value() == other.has_value()`.

```cpp
template<class U>
constexpr explicit(/*see below*/) option(option<U>&& other) noexcept(/*see below*/);
```
Converting move constructor. \
Constructs an object of type `T` using *direct-list-initialization* with the expression `std::move(other.get())` if `other` contains a value. If `other` does not contain a value, construct an empty `opt::option` object instead.
- *`noexcept`* when `std::is_nothrow_constructible_v<T, U&&>`.
- *`explicit`* when `!std::is_convertible_v<U&&, T>`.
- *Enabled* when the following conditions are true:
    - `std::is_convertible_v<T, U&&>`.
    - `!std::is_same_v<remove_cvref<T>, bool>`.
    - `!is_constructible_from_option<T, U>`. \
    Where `remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type X. \
    Where `is_constructible_from_option<X, Y>` is a metafunction, that checks if type `X` is constructible or convertible from any expression of type `opt::option<Y>` (possibly `const`), i.e., in the following there is at least one `true`:
        - `std::is_constructible_v<X, opt::option<Y>&>`.
        - `std::is_constructible_v<X, const opt::option<Y>&>`.
        - `std::is_constructible_v<X, opt::option<Y>&&>`.
        - `std::is_constructible_v<X, const opt::option<Y>&&>`.
        - `std::is_convertible_v<opt::option<Y>&, X>`.
        - `std::is_convertible_v<const opt::option<Y>&, X>`.
        - `std::is_convertible_v<opt::option<Y>&&, X>`.
        - `std::is_convertible_v<const opt::option<Y>&&, X>`.
- *Postcondition:* `has_value() == other.has_value()`.

### Destructor

```cpp
~option() noexcept(/*see below*/);
```
Destructs the contained object of type `T` if the `opt::option` object contains it. If the `opt::option` object does not contain the value, do nothing.
- *`noexcept`* when `std::is_nothrow_destructible_v<T>`.
- *Trivial* when `std::is_trivially_destructible_v<T>`.
- *`constexpr`* when `std::is_trivially_destructible_v<T>`.

### `operator=`

```cpp
constexpr option& operator=(opt::none_t) noexcept(/*see below*/);
```
The contained value is destroyed if this `opt::option` contains a value.
- *`noexcept`* when `std::is_nothrow_destructible_v<T>`.
- *Postcondition:* `has_value() == false`.

```cpp
constexpr option& operator=(const option& other) noexcept(/*see below*/);
```
Copy assigns this `opt::option` from `other`.
- If this `opt::option` and `other` do not contain a value, the function do nothing.
- If this `opt::option` contains a value, but `other` does not, the contained value is destroyed.
- If this `opt::option` do not contain a value, but `other` does, the contained value of type `T` is constructed using *direct-list-initialization* with the expression `other.get()`.
- If this `opt::option` contains a value, and `other` does too, the contained value of type `T` is assigned with the expression `other.get()`.
<!-- -->
- *`noexcept`* when `std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>`.
- *Deleted* when `!std::is_reference_v<T>` and `!std::is_copy_constructible_v<T> || !std::is_copy_assignable_v<T>`.
- *Trivial* when `std::is_reference_v<T>` or the following are all `true`:
    - `std::is_trivially_copy_assignable_v<T>`.
    - `std::is_trivially_copy_constructible_v<T>`.
    - `std::is_trivially_destructible_v<T>`.
- *Postcondition:* `has_value() == other.has_value()`.

```cpp
constexpr option& operator=(option&& other) noexcept(/*see below*/);
```
Move assigns this `opt::option` from `other`.
- If this `opt::option` and `other` do not contain a value, the function do nothing.
- If this `opt::option` contains a value, but `other` does not, the contained value is destroyed.
- If this `opt::option` do not contain a value, but `other` does, the contained value of type `T` is constructed using *direct-list-initialization* with the expression `std::move(other.get())`.
- If this `opt::option` contains a value, and `other` does too, the contained value of type `T` is assigned with the expression `std::move(other.get())`.

> [!IMPORTANT]
> After move, `other` still holds a value (if it had before), but the value itself is moved from.

- *`noexcept`* when `std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>`.
- *Deleted* when `!std::is_reference_v<T>` and `!std::is_move_constructible_v<T> || !std::is_move_assignable_v<T>`.
- *Trivial* when `std::is_reference_v<T>` or the following are all `true`:
    - `std::is_trivially_move_assignable_v<T>`.
    - `std::is_trivially_move_constructible_v<T>`.
    - `std::is_trivially_destructible_v<T>`.
- *Postcondition:* `has_value() == other.has_value()`.

```cpp
template<class U = T>
constexpr option& operator=(U&& value) noexcept(/*see below*/);
```
Perfect-forwarded assignment. \
If this `opt::option` contains a value before the call, it is assigned from the expression `std::forward<U>(value)`. If does not, the contained object of type `T` is constructed using *direct-list-initialization* with the expression `std::forward<U>(value)`.
- *`noexcept`* when the following are all `true`:
    - `std::is_nothrow_assignable_v<T&, U&&>`.
    - `std::is_nothrow_constructible_v<T, U&&>`.
    - `std::is_nothrow_destructible_v<T>`.
- *Enabled* when the following are all `true`:
    - `!opt::is_option<U>`.
    - `!(std::is_scalar_v<T> && std::is_same_v<T, std::decay_t<U>>)`.
    - `std::is_constructible_v<T, U>`.
    - `std::is_assignable_v<T&, U>`.

```cpp
template<class U>
constexpr option& operator=(const option<U>& other);
```
Assigns this `opt::option` from `other`. \
- If this `opt::option` and `other` do not contain a value, the function do nothing.
- If this `opt::option` contains a value, but `other` does not, the contained value is destroyed.
- If this `opt::option` do not contain a value, but `other` does, the contained value of type `T` is constructed using *direct-list-initialization* with the expression `other.get()`.
- If this `opt::option` contains a value, and `other` does too, the contained value of type `T` is assigned with the expression `other.get()`.
<!-- -->
- *Enabled* when `!is_constructible_from_option<T, U> && !is_assignable_from_option<T, U>` and `std::is_reference_v<T> || (std::is_constructible_v<T, const U&> && std::is_assignable_v<T&, const U&>)`.
- Where `is_constructible_from_option<X, Y>` is a metafunction, that checks if type `X` is constructible or convertible from any expression of type `opt::option<Y>` (possibly `const`), i.e., in the following there is at least one `true`:
    - `std::is_constructible_v<X, opt::option<Y>&>`.
    - `std::is_constructible_v<X, const opt::option<Y>&>`.
    - `std::is_constructible_v<X, opt::option<Y>&&>`.
    - `std::is_constructible_v<X, const opt::option<Y>&&>`.
    - `std::is_convertible_v<opt::option<Y>&, X>`.
    - `std::is_convertible_v<const opt::option<Y>&, X>`.
    - `std::is_convertible_v<opt::option<Y>&&, X>`.
    - `std::is_convertible_v<const opt::option<Y>&&, X>`.
- Where `is_assignable_from_option<X, Y>` is a metafunction, that checks if type `X` is assignable from any expression of type `opt::option<Y>` (possibly `const`), i.e., in the following conditions there is at least one `true`:
    - `std::is_assignable_v<X&, opt::option<Y>&>`.
    - `std::is_assignable_v<X&, const opt::option<Y>&>`.
    - `std::is_assignable_v<X&, opt::option<Y>&&>`.
    - `std::is_assignable_v<X&, const opt::option<Y>&&>`.
- *Postcondition:* `has_value() == other.has_value()`.

```cpp
template<class U = T>
constexpr option& operator=(option<U>&& other);
```
Assigns this `opt::option` from `other`.
- If this `opt::option` and `other` do not contain a value, the function do nothing.
- If this `opt::option` contains a value, but `other` does not, the contained value is destroyed.
- If this `opt::option` do not contain a value, but `other` does, the contained value of type `T` is constructed using *direct-list-initialization* with the expression `std::move(other.get())`.
- If this `opt::option` contains a value, and `other` does too, the contained value of type `T` is assigned with the expression `std::move(other.get())`.

> [!IMPORTANT]
> After move, `other` still holds a value (if it had before), but the value itself is moved from.

- *Enabled* when `!is_constructible_from_option<T, U> && !is_assignable_from_option<T, U>` and `std::is_reference_v<T> || (std::is_constructible_v<T, U&&> && std::is_assignable_v<T&, U&&>)`.
- Where `is_constructible_from_option<X, Y>` is a metafunction, that checks if type `X` is constructible or convertible from any expression of type `opt::option<Y>` (possibly `const`), i.e., in the following there is at least one `true`:
    - `std::is_constructible_v<X, opt::option<Y>&>`.
    - `std::is_constructible_v<X, const opt::option<Y>&>`.
    - `std::is_constructible_v<X, opt::option<Y>&&>`.
    - `std::is_constructible_v<X, const opt::option<Y>&&>`.
    - `std::is_convertible_v<opt::option<Y>&, X>`.
    - `std::is_convertible_v<const opt::option<Y>&, X>`.
    - `std::is_convertible_v<opt::option<Y>&&, X>`.
    - `std::is_convertible_v<const opt::option<Y>&&, X>`.
- Where `is_assignable_from_option<X, Y>` is a metafunction, that checks if type `X` is assignable from any expression of type `opt::option<Y>` (possibly `const`), i.e., in the following conditions there is at least one `true`:
    - `std::is_assignable_v<X&, opt::option<Y>&>`.
    - `std::is_assignable_v<X&, const opt::option<Y>&>`.
    - `std::is_assignable_v<X&, opt::option<Y>&&>`.
    - `std::is_assignable_v<X&, const opt::option<Y>&&>`.
- *Postcondition:* `has_value() == other.has_value()`.

### `reset`
```cpp
constexpr void reset() noexcept(/*see below*/);
```
Destroys the contained value. \
If this `opt::option` contains a value, destroy that contained value. If does not, do nothing.
- *`noexcept`* when `std::is_nothrow_destructible_v<T>`.
- *Postcondition:* `has_value() == false`

### `emplace`
```cpp
template<class... Args>
constexpr T& emplace(Args&&... args) noexcept(/*see below*/) /*lifetimebound*/;
```
Constructs the contained from `args...`. \
If this `opt::option` already contains a value, the contained value is destroyed. Initializes the contained value of type `T` using *direct-list-initialization* with `std::forward<Args>(args)...` as parameters.
- *`noexcept`* when `std::is_nothrow_constructible_v<T, Args...> && std::is_nothrow_destructible_v<T>`.
- *Enabled* when `std::is_constructible_v<T, Args...>`.

### `has_value`, `operator bool`
```cpp
constexpr bool has_value() const noexcept;

constexpr explicit operator bool() const noexcept;
```
Checks if this `opt::option` contains a value. \
Returns `true` if this `opt::option` contains a value, otherwise if it does not, return `false`.

### `has_value_and`
```cpp
template<class P>
constexpr bool has_value_and(P&& predicate) &;
template<class P>
constexpr bool has_value_and(P&& predicate) const&;
template<class P>
constexpr bool has_value_and(P&& predicate) &&;
template<class P>
constexpr bool has_value_and(P&& predicate) const&&;
```
Returns `true` if this `opt::option` contains a value, and the contained value matches a predicate. \
If this `opt::option` contains a value, return the result of invocation of `predicate` with the contained value as an argument. If this `opt::option` does not contain a value, return `false`.
- *Enabled* when `std::is_invocable_r_v<bool, P, T>`.

Example:
```cpp
opt::option<int> a = 3;

std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; // true

a = 0;
std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; // false

a = opt::none;
std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; // false
```

### `take`
```cpp
constexpr option take() &;
```
Takes the value out of the `opt::option`. \
Copy construct temporary value using the contained value in the `opt::option`, destroy which is left in the contained value in the `opt::option`, and return that temporary value with copy elision.
- *Postcondition:* `has_value() == false`.

```cpp
constexpr option take() &&;
```
Takes the value out of the `opt::option`. \
Returns this `opt::option` with the expression `std::move(*this)`, but *do not destroys the contained value*.

Example:
```cpp
opt::option<int> a = 1;

std::cout << a.has_value() << '\n'; // true

auto b = a.take();

std::cout << a.has_value() << '\n'; // false
std::cout << *b << '\n'; // 1

a = opt::none;
b = a.take();
std::cout << a.has_value() << '\n'; // false
std::cout << b.has_value() << '\n'; // false
```

### `take_if`
```cpp
template<class P>
constexpr option take_if(P&& predicate);
```
Takes the value out of the `opt::option`, but only if the `predicate` evaluates to `true` with a non-const contained value as an argument. \
Returns an empty `opt::option` if this `opt::option` does not contain a value or `predicate` evaluates to `false` with the non-const contained value as an argument; otherwise, return the expression `take()`.
- *Enabled* when `std::is_invocable_r_v<bool, P, T&>`.

Example:
```cpp
opt::option<int> a = 1;

auto b = a.take_if([](int& x) {
    x += 1;
    return x >= 3;
});
std::cout << *a << '\n'; // 2
std::cout << b.has_value() << '\n'; // false

b = a.take_if([](int& x) {
    x += 1;
    return x >= 3;
});
std::cout << a.has_value() << '\n'; // false
std::cout << *b << '\n'; // 3
```

### `insert`
```cpp
constexpr T& insert(const std::remove_reference_t<T>& value) /*lifetimebound*/;
```
Insert `value` into the `opt::option`, and then returns a non-const reference to the new contained value. \
If the `opt::option` contains a value, the contained value is destroyed, and copy constructs the new value in-place with `value` as parameters, and then returns a reference to the new contained value.
- *Postcondition:* `has_value() == true`.

```cpp
constexpr T& insert(std::remove_reference_t<T>&& value) /*lifetimebound*/;
```
Insert `value` into the `opt::option`, and then returns a non-const reference to the new contained value. \
If the `opt::option` contains a value, the contained value is destroyed, and move constructs the new value in-place with `std::move(value)` as parameters, and then returns a reference to the new contained value.
- *Postcondition:* `has_value() == true`.

### `inspect`
```cpp
template<class F>
constexpr option& inspect(F&& fn) &;
template<class F>
constexpr const option& inspect(F&& fn) const&;
template<class F>
constexpr option&& inspect(F&& fn) &&;
template<class F>
constexpr const option&& inspect(F&& fn) const&&;
```
Invokes `fn` with a reference (possible `const`) to the contained value if the `opt::option` contains one. If it does not, there are no effects. Returns a reference to the this `opt::option`.

Example:
```cpp
opt::option<int> a = 1;

a.inspect([](int x) { std::cout << x << '\n'; }); // 1

a.map([](int x) { return x * 2; })
 .inspect([](int x) { std::cout << x << '\n'; }) // 2
 .map([](int x) { return float(x * 2) + 0.5f; })
 .inspect([](float x) { std::cout << x << '\n'; }) // 4.5
 .and_then([](float x) { return x > 5.f ? opt::option<double>{x} : opt::none; })
 .inspect([](double x) { std::cout << x << '\n'; }); // will not print `x`
```

### `get`, `operator*`, `operator->`
```cpp
constexpr T& get() & noexcept /*lifetimebound*/;
constexpr const T& get() const& noexcept /*lifetimebound*/;
constexpr std::remove_reference_t<T>&& get() && noexcept /*lifetimebound*/;
constexpr const std::remove_reference_t<T>&& get() const&& noexcept /*lifetimebound*/;
```
Access the contained value. \
Returns a reference to the contained value of the `opt::option`. Calls the [`OPTION_VERIFY`](./macros.md#option_verify) macro if the `opt::option` does not contain a value. Same as `operator*`.
- *Precondition:* `has_value() == true`

```cpp
constexpr std::add_pointer_t<const T> operator->() const noexcept /*lifetimebound*/;
constexpr std::add_pointer_t<T> operator->() noexcept /*lifetimebound*/;
```
Access the contained value members. \ 
Returns a pointer to the contained value (`std::addressof(get())`) of the `opt::option`. Calls the [`OPTION_VERIFY`](./macros.md#option_verify) macro if the `opt::option` does not contain a value.
- *Precondition:* `has_value() == true`

```cpp
constexpr T& operator*() & noexcept /*lifetimebound*/;
constexpr const T& operator*() const& noexcept /*lifetimebound*/;
constexpr std::remove_reference_t<T>&& operator*() && noexcept /*lifetimebound*/;
constexpr const std::remove_reference_t<T>&& operator*() const&& /*lifetimebound*/;
```
Access the contained value. \
Returns a reference to the contained value of the `opt::option`. Calls the [`OPTION_VERIFY`](./macros.md#option_verify) macro if the `opt::option` does not contain a value. Same as `get()`.
- *Precondition:* `has_value() == true`

### `get_unchecked`
```cpp
constexpr T& get_unchecked() & noexcept;
constexpr const T& get_unchecked() const& noexcept;
constexpr std::remove_reference_t<T>&& get_unchecked() && noexcept;
constexpr const std::remove_reference_t<T>&& get_unchecked() const&& noexcept;
```
Access the contained value, without checking for it existence. \
Returns a reference to the contained value. *Does not* calls the [`OPTION_VERIFY`](./macros.md#option_verify) macro if the `opt::option` does not contain a value. Note that this method does not have the `lifetimebound` attribute. Considered for accessing type, that specified in user defined `opt::option_traits`.

> [!CAUTION]
> Using this method on an empty `opt::option` will cause [Undefined Behavior][UB].

### `value`, `value_or_throw`
```cpp
constexpr T& value_or_throw() & /*lifetimebound*/;
constexpr const T& value_or_throw() const& /*lifetimebound*/;
constexpr std::remove_reference_t<T>&& value_or_throw() const& /*lifetimebound*/;
constexpr const std::remove_reference_t<T>&& value_or_throw() const&& /*lifetimebound*/;

constexpr T& value() & /*lifetimebound*/;
constexpr const T& value() const& /*lifetimebound*/;
constexpr std::remove_reference_t<T>&& value() && /*lifetimebound*/;
constexpr const std::remove_reference_t<T>&& value() const&& /*lifetimebound*/;
```
Returns a reference to the contained value. \
Throws a [`opt::bad_access`](#bad_access) exception if `opt::option` does not contain the value. \
The `value_or_throw()` method is a more explicit version of the `value()` method.

### `value_or`
```cpp
template<class U>
constexpr T value_or(U&& default) const& noexcept(/*see below*/) /*lifetimebound*/;
```
Returns the contained value if `opt::option` contains one or returns a provided `default` instead. \
- *`noexcept`* when `std::is_nothrow_copy_constructible_v<T>` and ` std::is_nothrow_constructible_v<T, U&&>`.
- *Requirements:* `std::is_copy_constructible_v<T>` and `std::is_convertible_v<U&&, T>`.

```cpp
template<class U>
constexpr T value_or(U&& default) && noexcept(/*see below*/) /*lifetimebound*/;
```
Returns the contained value if `opt::option` contains one or returns a provided `default` instead. \
- *`noexcept`* when `std::is_nothrow_move_constructible_v<T>` and ` std::is_nothrow_constructible_v<T, U&&>`.
- *Requirements:* `std::is_move_constructible_v<T>` and `std::is_convertible_v<U&&, T>`.

### `value_or_default`
```cpp
constexpr T value_or_default() const& noexcept(/*see below*/);
```
Returns the contained value if `opt::option` contains one, otherwise returns a default constructed of type `T` (expression `T{}`).
- *`noexcept`* when `std::is_nothrow_copy_constructible_v<T>` and `std::is_nothrow_default_constructible_v<T>`.
- *Requirements:* `std::is_default_constructible_v<T>`, `std::is_copy_constructible_v<T>` and `std::is_move_constructible_v<T>`.

```cpp
constexpr T value_or_default() const& noexcept(/*see below*/);
```
Returns the contained value if `opt::option` contains one, otherwise returns a default constructed of type `T` (expression `T{}`).
- *`noexcept`* when `std::is_nothrow_move_constructible_v<T>` and `std::is_nothrow_default_constructible_v<T>`.
- *Requirements:* `std::is_default_constructible_v<T>` and `std::is_move_constructible_v<T>`.

### `map_or`
```cpp
template<class U, class Fn>
constexpr remove_cvref<U> map_or(U&& default, Fn&& function) &;
template<class U, class Fn>
constexpr remove_cvref<U> map_or(U&& default, Fn&& function) const&;
template<class U, class Fn>
constexpr remove_cvref<U> map_or(U&& default, Fn&& function) &&;
template<class U, class Fn>
constexpr remove_cvref<U> map_or(U&& default, Fn&& function) const&&;
```
Returns the provided `default` value if `opt::option` does not contain a value, or invokes the `function` with the contained value as an argument. \
Where `remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type `X`.
- *Enabled* when `std::is_invocable_r_v<remove_cvref<U>, Fn, T>` is `true`.

Example:
```cpp
opt::option<int> a = 2;

std::cout << a.map_or(0, [](int x) { return x * 200; }) << '\n'; // 400

a = opt::none;
std::cout << a.map_or(0, [](int x) { return x * 2; }) << '\n'; // 0
```

### `map_or_else`
```cpp
template<class D, class Fn>
constexpr auto map_or_else(D&& default, Fn&& function) &;
template<class D, class Fn>
constexpr auto map_or_else(D&& default, Fn&& function) const&;
template<class D, class Fn>
constexpr auto map_or_else(D&& default, Fn&& function) &&;
template<class D, class Fn>
constexpr auto map_or_else(D&& default, Fn&& function) const&&;
```
Returns the result of `default` function with no arguments if `opt::option` does not contain a value; otherwise, returns the result of `function` function with the contained value as an first argument.
- *Enabled* when `std::is_invocable_v<D>` and `std::is_invocable_v<Fn, T>` are `true`.
- *Requirements:* the return type of `default` function must be the same as the return type of `function` function.

Example:
```cpp
opt::option<int> a = 3;

std::cout << a.map_or_else(
    [] { std::cout << "will not print"; return 0; },
    [](int x) { return x + 1; }
) << '\n'; // 4

a = opt::none;
std::cout << a.map_or_else(
    [] { std::cout << "will print "; return -100; },
    [](int x) { return x - 2; }
) << '\n'; // will print -100
```

### `ptr_or_null`
```cpp
constexpr std::remove_reference_t<T>* ptr_or_null() & noexcept /*lifetimebound*/;
constexpr const std::remove_reference_t<T>* ptr_or_null() const& noexcept /*lifetimebound*/;
constexpr void ptr_or_null() && = delete;
constexpr void ptr_or_null() const&& = delete;
```
Returns a pointer to the contained value (`std::addressof(get())`) if `opt::option` contains one. If it does not, returns `nullptr` instead.

Example:
```cpp
int a = 2;

opt::option<int&> b = a;

std::cout << (b.ptr_or_null() == &a) << '\n'; // true

b = opt::none;
std::cout << b.ptr_or_null() << '\n'; // 0000000000000000 (nullptr)
```

### `filter`
```cpp
template<class Fn>
constexpr option filter(Fn&& function) const;
```
Returns an empty `opt::option` if this `opt::option` does not contain a value. If it does, returns the contained value if `function` returns `true`, and an empty `opt::option` if `function` returns `false`.
- *Enabled* when `std::is_invocable_r_v<bool, Fn, const T&>` is `true`.

Example:
```cpp
const auto is_odd = [](auto x) {
    return x % 2 != 0;
};
opt::option<int> a = 1;

std::cout << *a.filter(is_odd) << '\n'; // 1

a = 2;
std::cout << a.filter(is_odd).has_value() << '\n'; // false

a = opt::none;
std::cout << a.filter(is_odd).has_value() << '\n'; // false
```

### `flatten`
```cpp
constexpr typename T::value_type flatten() const&;
constexpr typename T::value_type flatten() &&;
```
Converts `opt::option<opt::option<X>>` to `opt::option<X>`. \
If this `opt::option` contains a value and the contained `opt::option` contains the values, return the underlying `opt::option` that contains a value. If they does not, returns an empty `opt::option`.
- *Requirements:* the `opt::option` must contain a value of specialization of `opt::option`.

```cpp
opt::option<opt::option<int>> a = 1;

opt::option<int> b = a.flatten();
std::cout << *b << '\n'; // 1

*a = opt::none;
std::cout << a.flatten().has_value() << '\n'; // false

a = opt::none;
std::cout << a.flatten().has_value() << '\n'; // false
```

### `and_then`
```cpp
template<class Fn>
constexpr option<U> and_then(Fn&& function) &;
template<class Fn>
constexpr option<U> and_then(Fn&& function) const&;
template<class Fn>
constexpr option<U> and_then(Fn&& function) &&;
template<class Fn>
constexpr option<U> and_then(Fn&& function) const&&;
```
Returns an empty `opt::option` if this `opt::option` does not contain a value. If it does, invokes `function` function with the contained value as an first argument, and then returns the result of that invocation. \
This operation is also sometimes called *flatmap*.
- *Enabled* when `std::is_invocable_v<Fn, T>`.
- *Requirements:* the result type of `function` must be a specialization of `opt::option`.

Example:
```cpp
const auto do_something = [](int x) {
    return x == 0 ? opt::option<float>{1.5f} : opt::none;
};

opt::option<int> a = 0;

std::cout << a.and_then(do_something).get() << '\n'; // 1.5

a = 1;
std::cout << a.and_then(do_something).has_value() << '\n'; // false

a = opt::none;
std::cout << a.and_then(do_something).has_value() << '\n'; // false
```

### `map`
```cpp
template<class Fn>
constexpr option<U> map(Fn&& function) &;
template<class Fn>
constexpr option<U> map(Fn&& function) const&;
template<class Fn>
constexpr option<U> map(Fn&& function) &&;
template<class Fn>
constexpr option<U> map(Fn&& function) const&&;
```
Maps the `opt::option` to `opt::option<U>` by applying a function to a contained value, or if `opt::option` does not contain the value, returns an empty `opt::option` otherwise. \
If `opt::option` contains a value, invokes `function` function with the contained value as an first argument, then wraps the function result into `opt::option<U>` and returns it. If `opt::option` does not contain the value, returns an empty `opt::option<U>`. \
Similar to [`std::optional<T>::transform`](https://en.cppreference.com/w/cpp/utility/optional/transform).
- *Enabled* when `std::is_invocable_v<Fn, T>` is `true`.

Example:
```cpp
const auto to_float = [](int x) {
    return float(x) / 2.f;
};

opt::option<int> a = 1;

std::cout << a.map(to_float).get() << '\n'; // 0.5

a = opt::none;
std::cout << a.map(to_float).has_value() << '\n'; // false
```

### `or_else`
```cpp
template<class Fn>
constexpr option or_else(Fn&& function) const&;
template<class Fn>
constexpr option or_else(Fn&& function) &&;
```
If `opt::option` contains a value, returns it. If does not, returns the result of `function` function with *no arguments*. \
Similar to [`std::optional<T>::or_else`](https://en.cppreference.com/w/cpp/utility/optional/or_else).
- *Enabled* when `std::is_invocable_v<Fn>`.
- *Requirements:* the result type of `function` (without any cv-qualifiers) must be the same as `opt::option<T>`.

Example:
```cpp
opt::option<int> a = 2;

std::cout << a.or_else(
    [] { std::cout << "will not print"; return opt::option{3}; }
).get() << '\n'; // 2

a = opt::none;
std::cout << a.or_else(
    [] { std::cout << "will print "; return opt::option{10}; }
).get() << '\n'; // will print 10

std::cout << a.or_else(
    [] { std::cout << "will print "; return opt::option<int>{}; }
).has_value() << '\n'; // will print false
```

### `assume_has_value`
```cpp
constexpr void assume_has_value() const noexcept;
```
Specifies that `opt::option` will always contain a value at a given point.

> [!CAUTION]
> Will cause [Undefined Behavior][UB] if `opt::option` does not contain a value.

### `unzip`
```cpp
constexpr auto unzip() &;
constexpr auto unzip() const&;
constexpr auto unzip() &&;
constexpr auto unzip() const&&;
```
Unzips `opt::option` that contains a *tuple like *type, into the *tuple like* object that contains values that wrapped into `opt::option`. \
If `opt::option` contains the value, return *tuple like* object that contains `opt::option`s of the *tuple like* object contained types. If `opt::option` does not contain the value, return *tuple like* object that contains empty `opt::option`s.
- *Requirements:* the contained value (without cv-qualifiers) must be a specialization of *tuple like* object.

Where *tuple like* object is a type of specialization of `std::array`, `std::pair` or `std::tuple`.

Example:
```cpp
opt::option<std::tuple<int, float>> a{1, 2.5f};

std::tuple<opt::option<int>, opt::option<float>> unzipped_a;
unzipped_a = a.unzip();

std::cout << std::get<0>(unzipped_a).get() << '\n'; // 1
std::cout << std::get<1>(unzipped_a).get() << '\n'; // 2.5

opt::option<std::array<int, 3>> b = opt::none;

std::array<opt::option<int>, 3> unzipped_b;
unzipped_b = b.unzip();

std::cout << (!unzipped_b[0] && !unzipped_b[1] && !unzipped_b[2]) << '\n'; // true
```

### `replace`
```cpp
template<class U>
constexpr option<T> replace(U&& value) &;
```
Replaces the contained value by a provided `value` and returns the old `opt::option` contained value.
- *Enabled* when `std::is_constructible_v<T, U&&>` is `true`.

Example:
```cpp
opt::option<int> a = 1;

opt::option<int> b = a.replace(2);

std::cout << *a << '\n'; // 2
std::cout << *b << '\n'; // 1

a = opt::none;
b = a.replace(3);

std::cout << *a << '\n'; // 3
std::cout << b.has_value() << '\n'; // false
```

## Non-member functions

### `zip`
```cpp
template<class Options>
constexpr /*see below*/ zip(Options&&... options);
```
Zips `options...` into `opt::option<std::tuple<...>>`. \
If every `options...` contains the values, returns the `std::tuple` wrapped in `opt::option` with the forwarded containing value from `options...`. If any `options...` does not contain the value, return an empty `opt::option`.
- *Enabled* when every `Options...` (without cv-qualifiers) is the specializations of `opt::option`.

The return type of `zip` is `opt::option<std::tuple<typename remove_cvref<Options>::value_type...>>`. \
Where `remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type `X`.

Example:
```cpp
opt::option<int> a{1};
opt::option<float> b{2.f};
opt::option<double> c{3.};

opt::option<std::tuple<int, float, double>> abc;
abc = opt::zip(a, b, c);
std::cout << abc.has_value() << '\n'; // true

a = opt::none;
abc = opt::zip(a, b, c);
std::cout << abc.has_value() << '\n'; // false
```

### `zip_with`
```cpp
template<class Fn, class... Options>
constexpr auto zip_with(Fn&& fn, Options&&... options);
```
If every `options...` contains the values, returns the result of `fn` function with every `options...` containing values as the `fn` arguments. If any of `options...` does not contain a value, returns an empty `opt::option`.
- *Enabled* when every `Options...` (without cv-qualifiers) is the specializations of `opt::option`.

Example:
```cpp
opt::option<int> a{10};
opt::option<float> b{5.f};

const auto add_and_print = [](int x, float y) {
    std::cout << (x + y) << '\n';
};

opt::zip_with(add_and_print, a, b); // 15

a = opt::none;
opt::zip_with(add_and_print, a, b); // will not call `add_and_print`
```

### `option_cast`
```cpp
template<class To, class From>
constexpr option<To> option_cast(const option<From>& value);
template<class To, class From>
constexpr option<To> option_cast(option<From>&& value);
```
Casts `opt::option<From>` to `opt::option<To>`. \
If `opt::option<From>` contains a value, `static_cast`s it to the type `To` and wraps it into `opt::option<To>`. If `opt::option<From>` does not contain a value, returns an empty `opt::option<To>`.

Example:
```cpp
opt::option<float> a{2.5f};
opt::option<int> b;

b = opt::option_cast<int>(a);
std::cout << *b << '\n'; // 2

a = opt::none;
b = opt::option_cast<int>(a);
std::cout << b.has_value() << '\n'; // false
```

### `operator|`
```cpp
template<class T>
constexpr T operator|(const option<T>& left, const T& right);
```
If `left` does not contain a value, returns `right` value instead. If `left` does, just returns `left`. \
Same as [`opt::option<T>::value_or`](#value_or).

```cpp
template<class T>
constexpr option<T> operator|(const option<T>& left, const option<T>& right);
```
Returns `left` if it does contains a value, or returns `right` if `left` does not.

```cpp
template<class T>
constexpr option<T> operator|(const option<T>& left, none_t);
```
Returns `left`.

```cpp
template<class T>
constexpr option<T> operator|(none_t, const option<T>& right);
```
Returns `right`.

Example:
```cpp
opt::option<int> a = 1;
opt::option<int> b = 2;

std::cout << (a | b).get() << '\n'; // 1

a = opt::none;
std::cout << (a | b).get() << '\n'; // 2

b = opt::none;
std::cout << (a | b).has_value() << '\n'; // false

// same as value_or(10)
std::cout << (a | 10) << '\n'; // 10

a |= 5;
std::cout << *a << '\n'; // 5
a |= 25;
std::cout << *a << '\n'; // 5

b |= a;
std::cout << *b << '\n'; // 5
```

### `operator|=`
```cpp
template<class T>
constexpr option<T>& operator|=(option<T>& left, const option<T>& right);

template<class T>
constexpr option<T>& operator|=(option<T>& left, const T& right);
```
Copy assigns `right` to `left` if the `left` does not contain a value. \
Returns a reference to `left`.

```cpp
template<class T>
constexpr option<T>& operator|=(option<T>& left, option<T>&& right);

template<class T>
constexpr option<T>& operator|=(option<T>& left, T&& right);
```
Move assigns `right` to `left` if the `left` does not contain a value, \
Returns a reference to `left`.

### `operator&`
```cpp
template<class T, class U>
constexpr option<U> operator&(const option<T>& left, const option<U>& right);
```
Returns an empty `opt::option` if `left` does not contain a value, or if `left` does, returns `right`.

Example:
```cpp
opt::option<int> a = 1;
opt::option<float> b = 2.5f;

std::cout << (a & b).get() << '\n'; // 2.5

a = opt::none;
std::cout << (a & b).has_value() << '\n'; // false

a = 1;
b = opt::none;
std::cout << (a & b).has_value() << '\n'; // false
```

### `operator^`
```cpp
template<class T>
constexpr option<T> operator^(const option<T>& left, const option<T>& right);
```
Returns `opt::option` that contains a value if exactly one of `left` and `right` contains a value, otherwise, returns an empty `opt::option`.

Example:
```cpp
opt::option<int> a = 2;
opt::option<int> b = 10;

std::cout << (a ^ b).has_value() << '\n'; // false

a = opt::none;
std::cout << (a ^ b).get() << '\n'; // 10

a = 5;
b = opt::none;
std::cout << (a ^ b).get() << '\n'; // 5

a = opt::none;
b = opt::none;
std::cout << (a ^ b).has_value() << '\n'; // false
```

### `operator==`
```cpp
template<class T1, class T2>
constexpr bool operator==(const option<T1>& left, const option<T2>& right) noexcept(/*see below*/);
```
If `left` and `right` contains the values, then compare values using operator `==`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `==`.
- *`noexcept`* when `noexcept(*left == *right)` is `true`.

```cpp
template<class T>
constexpr bool operator==(const option<T>& left, none_t) noexcept;
```
Returns `!left.has_value()`.

```cpp
template<class T>
constexpr bool operator==(none_t, const option<T>& right) noexcept;
```
Returns `!right.has_value()`.

```cpp
template<class T1, class T2>
constexpr bool operator==(const option<T1>& left, const T2& right) noexcept(/*see below*/);
```
If `left` contains a value, then compare it with `right` using operator `==`; otherwise, return `false`.
- *`noexcept`* when `noexcept(*left == right)` is `true`.

```cpp
template<class T1, class T2>
constexpr bool operator==(const T1& left, const option<T2>& right) noexcept(/*see below*/);
```
If `right` contains a value, then compare it with `left` using operator `==`; otherwise, return `false`.
- *`noexcept`* when `noexcept(left == *right)` is `true`.

### `operator!=`
```cpp
template<class T1, class T2>
constexpr bool operator!=(const option<T1>& left, const option<T2>& right) noexcept(/*see below*/);
```
If `left` and `right` contains the values, then compare values using operator `!=`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `!=`.
- *`noexcept`* when `noexcept(*left != *right)` is `true`.

```cpp
template<class T>
constexpr bool operator!=(const option<T>& left, none_t) noexcept;
```
Returns `left.has_value()`.

```cpp
template<class T>
constexpr bool operator!=(none_t, const option<T>& right) noexcept;
```
Returns `right.has_value()`.

```cpp
template<class T1, class T2>
constexpr bool operator!=(const option<T1>& left, const T2& right) noexcept(/*see below*/);
```
If `left` contains a value, then compare it with `right` using operator `!=`; otherwise, return `true`.
- *`noexcept`* when `noexcept(*left != right)` is `true`.

```cpp
template<class T1, class T2>
constexpr bool operator!=(const T1& left, const option<T2>& right) noexcept(/*see below*/);
```
If `right` contains a value, then compare it with `left` using operator `!=`; otherwise, return `true`.
- *`noexcept`* when `noexcept(left != *right)` is `true`.

### `operator<`
```cpp
template<class T1, class T2>
constexpr bool operator<(const option<T1>& left, const option<T2>& right) noexcept(/*see below*/);
```
If `left` and `right` contains the values, then compare values using operator `<`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `<`.
- *`noexcept`* when `noexcept(*left < *right)` is `true`.

```cpp
template<class T>
constexpr bool operator<(const option<T>& left, none_t) noexcept;
```
Returns `false`.

```cpp
template<class T>
constexpr bool operator<(none_t, const option<T>& right) noexcept;
```
Returns `right.has_value()`.

```cpp
template<class T1, class T2>
constexpr bool operator<(const option<T1>& left, const T2& right) noexcept(/*see below*/);
```
If `left` contains a value, then compare it with `right` using operator `<`; otherwise, return `true`.
- *`noexcept`* when `noexcept(*left < right)` is `true`.

```cpp
template<class T1, class T2>
constexpr bool operator<(const T1& left, const option<T2>& right) noexcept(/*see below*/);
```
If `right` contains a value, then compare it with `left` using operator `<`; otherwise, return `false`.
- *`noexcept`* when `noexcept(left < *right)` is `true`.

### `operator<=`
```cpp
template<class T1, class T2>
constexpr bool operator<=(const option<T1>& left, const option<T2>& right) noexcept(/*see below*/);
```
If `left` and `right` contains the values, then compare values using operator `<=`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `<=`.
- *`noexcept`* when `noexcept(*left <= *right)` is `true`.

```cpp
template<class T>
constexpr bool operator<=(const option<T>& left, none_t) noexcept;
```
Returns `!left.has_value()`.

```cpp
template<class T>
constexpr bool operator<=(none_t, const option<T>& right) noexcept;
```
Returns `true`.

```cpp
template<class T1, class T2>
constexpr bool operator<=(const option<T1>& left, const T2& right) noexcept(/*see below*/);
```
If `left` contains a value, then compare it with `right` using operator `<=`; otherwise, return `true`.
- *`noexcept`* when `noexcept(*left <= right)` is `true`.

```cpp
template<class T1, class T2>
constexpr bool operator<=(const T1& left, const option<T2>& right) noexcept(/*see below*/);
```
If `right` contains a value, then compare it with `left` using operator `<=`; otherwise, return `false`.
- *`noexcept`* when `noexcept(left <= *right)` is `true`.

### `operator>`
```cpp
template<class T1, class T2>
constexpr bool operator>(const option<T1>& left, const option<T2>& right) noexcept(/*see below*/);
```
If `left` and `right` contains the values, then compare values using operator `>`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `>`.
- *`noexcept`* when `noexcept(*left > *right)` is `true`.

```cpp
template<class T>
constexpr bool operator>(const option<T>& left, none_t) noexcept;
```
Returns `left.has_value()`.

```cpp
template<class T>
constexpr bool operator>(none_t, const option<T>& right) noexcept;
```
Returns `false`.

```cpp
template<class T1, class T2>
constexpr bool operator>(const option<T1>& left, const T2& right) noexcept(/*see below*/);
```
If `left` contains a value, then compare it with `right` using operator `>`; otherwise, return `false`.
- *`noexcept`* when `noexcept(*left < right)` is `true`.

```cpp
template<class T1, class T2>
constexpr bool operator>(const T1& left, const option<T2>& right) noexcept(/*see below*/);
```
If `right` contains a value, then compare it with `left` using operator `>`; otherwise, return `true`.
- *`noexcept`* when `noexcept(left < *right)` is `true`.

### `operator>=`
```cpp
template<class T1, class T2>
constexpr bool operator>=(const option<T1>& left, const option<T2>& right) noexcept(/*see below*/);
```
If `left` and `right` contains the values, then compare values using operator `>=`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `>=`.
- *`noexcept`* when `noexcept(*left >= *right)` is `true`.

```cpp
template<class T>
constexpr bool operator>=(const option<T>& left, none_t) noexcept;
```
Returns `true`.

```cpp
template<class T>
constexpr bool operator>=(none_t, const option<T>& right) noexcept;
```
Returns `!right.has_value()`.

```cpp
template<class T1, class T2>
constexpr bool operator>=(const option<T1>& left, const T2& right) noexcept(/*see below*/);
```
If `left` contains a value, then compare it with `right` using operator `=>`; otherwise, return `false`.
- *`noexcept`* when `noexcept(*left >= right)` is `true`.

```cpp
template<class T1, class T2>
constexpr bool operator>=(const T1& left, const option<T2>& right) noexcept(/*see below*/);
```
If `right` contains a value, then compare it with `left` using operator `=>`; otherwise, return `true`.
- *`noexcept`* when `noexcept(left >= *right)` is `true`.

## Helpers

### `std::hash<opt::option>`
```cpp
template<class T>
struct std::hash<opt::option<T>>;
```
The template specialization of `std::hash` for the `opt::option` gives users a ability to calculate hash of the contained value. \
If `opt::option` contains the value, returns hash of that value. If `opt::option` does not, returns the expression `static_cast<std::size_t>(-96391)` as an empty value hash.
- *Enabled* when `std::is_default_constructible_v<std::hash<std::remove_const_t<T>>>` is `true`.
- *`noexcept`* when the expression `noexcept(std::hash<std::remove_const_t<T>>{}(std::declval<const T&>()))` is `true`.

Example:
```cpp
opt::option<int> a{12345};

std::cout << std::hash<opt::option<int>>{}(a) << '\n'; // [some hash]

a = opt::none;
std::cout << std::hash<opt::option<int>>{}(a) << '\n'; // [some empty option hash]
```

### `none_t`
```cpp
struct none_t;
```
The tag type used to indicate `opt::option` with the contained value in uninitialized state. \
`opt::none_t` do not have a default constructor.

### `none`
```cpp
inline constexpr none_t none{/*special value*/};
```
The `opt::none` variable is a `constexpr` value of type [`opt::none_t`](#none_t) that is used to indicate `opt::option` with the contained value in uninitialized state.

### `bad_access`
```cpp
class bad_access;
```
The exception type of an object to be thrown by [`opt::option<T>::value`, `opt::option<T>::value_or_throw`](#value-value_or_throw) methods, if `opt::option` does not contain a value inside it.

### `option_traits`
```cpp
template<class T, class = void>
struct option_traits;
```
A template class type used for decreasing the size of `opt::option` for type `T`. \
You can also use a metafunction like `std::enable_if` in second template argument to leverage SFINAE. This allows to utilize `opt::option_traits` for more flexible specializations.

The `opt::option` uses `opt::option_traits` to not store separate `bool` variable to indicate an empty state, but instead use the contained value itself to store that flag. \
See [builtin `opt::option_traits` specializations](./sentinel.md).

You can declare without defining `opt::option_traits` to disable using the builtin `opt::option_traits` specializations if something goes wrong.

To define a custom specialization for `opt::option_traits`, user must specify the following 2 required static methods and 1 optional static method.

```cpp
static /*constexpr*/ bool is_empty(const T& value) noexcept;
```
Required. The `opt::option` uses the `is_empty` function to determine if the contained value is empty. Can be optionally marked `constexpr`.

> [!IMPORTANT]  
> After the contained value is constructed, the `is_empty` function must return `false`; otherwise, the `OPTION_VERIFY` macro is called.

```cpp
static /*constexpr*/ void set_empty(T& value) noexcept;
```
Required. The `opt::option` uses the `set_empty` function to set the contained value to an empty state. Called in default constructor and in [`reset`](#reset) method *after* the contained value is destroyed. Can be optionally marked `constexpr`.

```cpp
static /*constexpr*/ void unset_empty() noexcept;
```
Optional. The `opt::option` uses the `unset_empty` function to unset the contained value empty state. Called *before* the contained value is constructed, but not constructed in `opt::option` constructors. It is always called after the `set_empty` function. If user does not define the `unset_empty` function, it will be replaced with function that has no effect. Can be optionally marked `constexpr`.

If the above requirements are not met, the program will be ill-formed.

You can also optionally define `static constexpr` variable named `empty_value` to help Visual Studio debugger understand `opt::option` state.

Example:
```cpp
struct some_struct {
    int val;
};

template<>
struct opt::option_traits<some_struct> {
    static bool is_empty(const some_struct& value) noexcept {
        return value.val == -1;
    }
    static void set_empty(some_struct& value) noexcept {
        value = {-1};
    }
};

opt::option<some_struct> a{5};

std::cout << (sizeof(a) == sizeof(some_struct)) << '\n'; // true
std::cout << a->val << '\n'; // 5

a = opt::none;
std::cout << a.has_value() << '\n'; // false
std::cout << a.get_unchecked().val << '\n'; // -1
```

[UB]: https://en.cppreference.com/w/cpp/language/ub