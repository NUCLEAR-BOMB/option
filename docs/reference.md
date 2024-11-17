
# Reference

## Declaration

```cpp
template<class T>
class option;
```

## Template parameters

`T` - the type of the value to manage initialization state for.

`T` must be:
- not `std::in_place_t` (possibly cv-qualified).
- not `opt::none_t` (possibly cv-qualified).
- not `void` ([`std::is_void`][std::is_void]).
- destructible ([`std::is_destructible`][std::is_destructible]).
- not array ([`std::is_array`][std::is_array]).
- not function ([`std::is_function`][std::is_function]).

## Member types

| Member type    | Definition                            |
| -------------- | ------------------------------------- |  
| value_type     | T                                     |
| iterator       | *unspecified*, random access iterator |
| const_iterator | *unspecified*, random access iterator |

## Member functions

### Constructor

```cpp
constexpr option() noexcept;

constexpr option(opt::none_t) noexcept;
```
Default constructor. \
Constructs an `opt::option` object that does not contain a value.
- *Postcondition:* `has_value() == false`.

---

```cpp
constexpr option(const option& other) noexcept(/*see below*/);
```
Copy constructor.

Copy constructs an object of type `T` using *direct-list-initialization* with the expression `other.get()` if `other` contains a value. If `other` does not contain a value, construct an empty `opt::option` object instead.

Description in the code equivalent:
```cpp
if (other.has_value()) {
    {construct}(other.get());
}
```
Where `{construct}` is a function that constructs contained object in place.

- *`noexcept`* when `std::is_reference_v<T>` or `std::is_nothrow_copy_constructible_v<T>`.
- *Deleted* when `!std::is_reference_v<T>` and `!std::is_copy_constructible_v<T>`.
- *Trivial* when `std::is_reference_v<T>` or `std::is_trivially_copy_constructible_v<T>`.
- *Postcondition:* `has_value() == other.has_value()`.

---

```cpp
constexpr option(option&& other) noexcept(/*see below*/);
```
Move constructor.

Move constructs an object of type `T` using *direct-list-initialization* with the expression `std::move(other.get())` if `other` contains a value. If `other` does not contain a value, construct an empty `opt::option` object instead.

Description in the code equivalent:
```cpp
if (other.has_value()) {
    {construct}(std::move(other.get()));
}
```
Where `{construct}` is a function that constructs contained object in place.

- *`noexcept`* when `std::is_reference_v<T>` or `std::is_nothrow_move_constructible_v<T>`.
- *Deleted* when `!std::is_reference_v<T>` and `!std::is_move_constructible_v<T>`
- *Trivial* when `std::is_reference_v<T>` or `std::is_trivially_move_constructible_v<T>`.
- *Postcondition:* `has_value() == other.has_value()`.

!> After move, `other` still holds a value (if it had before), but the value itself is moved from.

---

```cpp
template<class U = T>
constexpr explicit(/*see below*/) option(U&& value);
```
Constructs an `opt::option` object that *contains a value*. Initializes a contained object of type `T` using *direct-list-initialization* with the expression `std::forward<U>(value)`.

- *`explicit`* when `!std::is_reference_v<T>` and `!std::is_convertible_v<U&&, T>`.
- *Enabled* when the following conditions are true:
    - If `T` is a non-reference type:
      - `std::is_constructible_v<T, U&&>`.
      - `!std::is_same_v<remove_cvref<U>, opt::option<T>>`. ([`remove_cvref`](#remove_cvrefx))
      - `!std::is_same_v<remove_cvref<U>, std::in_place_t`.
      - `(!std::is_same_v<remove_cvref<T>, bool> || !opt::is_option_v<remove_cvref<U>>)`.
    - Otherwise, if `T` is a reference type:
      - `!std::is_same_v<remove_cvref<U>, opt::option<T>>`.
      - `!std::is_same_v<remove_cvref<U>, std::in_place_t>`.
      - `!opt::is_option_v<remove_cvref<U>>`.
      - [`can_bind_reference<T, U>`](#can_bind_referencex-y).

> [!NOTE]
> After construction, the `opt::option` will always contain a value regardless of `value` argument.

---

```cpp
template<class First, class... Args>
constexpr option(First&& first, Args&&... args);
```
Constructs an `opt::option` object that contains a value that is initialized using *direct-list-initialization* with the arguments `std::forward<First>(first), std::forward<Args>(args)...`.

Always disabled when `T` is a reference type.

- *Enabled* when the following conditions are true:
    - `!std::is_reference_v<T>`.
    - `std::is_constructible_v<T, First, Args...> || is_initializable_from<T, First, Args...>`. ([`is_initializable_from`](#is_initializable_fromx-y))
    - `!std::is_same_v<remove_cvref<First>, opt::option<T>>`. ([`remove_cvref`](#remove_cvrefx))

> [!NOTE]
> After construction, the `opt::option` will always contain a value regardless of `first` and `args...` arguments.

---

```cpp
template<class... Args>
constexpr explicit option(std::in_place_t, Args&&... args);
```
Constructs an `opt::option` object that contains a value that is initialized using *direct-list-initialization* with the arguments `std::forward<Args>(args)...`.

Always disabled when `T` is a reference type.

> [!NOTE]
> After construction, the `opt::option` will always contain a value regardless of `args...` arguments.

---

```cpp
template<class U, class... Args>
constexpr explicit option(std::in_place_t, std::initializer_list<U> ilist, Args&&... args);
```
Constructs an `opt::option` object that contains a value that is initialized using *direct-list-initialization* with the arguments `ilist, std::forward<Args>(args)...`.

Always disabled when `T` is a reference type.

> [!NOTE]
> After construction, the `opt::option` will always contain a value regardless of `ilist` and `args...` arguments.

---

```cpp
template<class U>
constexpr explicit(/*see below*/) option(const option<U>& other);
```
Converting copy constructor. \
Constructs an object of type `T` using *direct-list-initialization* with the expression `other.get()` if `other` contains a value. If `other` does not contain a value, construct an empty `opt::option` object instead.

Description in the code equivalent:
```cpp
if (other.has_value()) {
    {construct}(other.get());
}
```
Where `{construct}` is a function that constructs contained object in place.

- *`explicit`* when `!std::is_reference_v<T>` and `!std::is_convertible_v<const U&, T>`.
- *Enabled* when the following conditions are true:
    - If `T` is a non-reference type:
      - `std::is_constructible_v<T, const U&>`.
      - `!std::is_same_v<T, U>`.
      - `std::is_same_v<remove_cvref<T>, bool> || !is_constructible_from_option<T, U>`. ([`remove_cvref`](#remove_cvrefx), [`is_constructible_from_option`](#is_constructible_from_optionx-y)).
    - Otherwise, if `T` is a reference type:
      - `!std::is_same_v<T, U>`,
      - [`can_bind_reference<T, U>`](#can_bind_referencex-y).
- *Postcondition:* `has_value() == other.has_value()`.
---

```cpp
template<class U>
constexpr explicit(/*see below*/) option(option<U>&& other);
```
Converting move constructor. \
Constructs an object of type `T` using *direct-list-initialization* with the expression `std::move(other.get())` if `other` contains a value. If `other` does not contain a value, construct an empty `opt::option` object instead.

Description in the code equivalent:
```cpp
if (other.has_value()) {
    {construct}(std::move(other.get()));
}
```
Where `{construct}` is a function that constructs contained object in place.

- *`explicit`* when `!std::is_reference_v<T>` and `!std::is_convertible_v<U&&, T>`.
- *Enabled* when the following conditions are true:
    - If `T` is a non-reference type:
      - `std::is_convertible_v<T, U&&>`.
      - `!std::is_same_v<T, U>`.
      - `std::is_same_v<std::remove_cv_t<T>, bool> || !is_constructible_from_option<T, U>`. ([`is_constructible_from_option`](#is_constructible_from_optionx-y),  [`remove_cvref`](#remove_cvrefx))
    - Otherwise, if `T` is a reference type:
      - `!std::is_same_v<T, U>`,
      - [`can_bind_reference<T, U>`](#can_bind_referencex-y).
- *Postcondition:* `has_value() == other.has_value()`.

---

### Destructor

```cpp
~option();
```
Destructs the contained object of type `T` if the `opt::option` object contains it. If the `opt::option` object does not contain the value, do nothing. Always `noexcept`.
- *Trivial* when `std::is_trivially_destructible_v<T>`.
- *`constexpr`* for non-`std::is_trivially_destructible_v<T>` types when C++20.

---

### `operator=`

```cpp
constexpr option& operator=(opt::none_t) noexcept;
```
The contained value is destroyed if this `opt::option` contains a value. Same as [`.reset()`](#reset).
- *Postcondition:* `has_value() == false`.

---

```cpp
constexpr option& operator=(const option& other) noexcept(/*see below*/);
```
Copy assigns this `opt::option` from `other`.
- If this `opt::option` and `other` do not contain a value, the function do nothing.
- If this `opt::option` contains a value, but `other` does not, the contained value is destroyed.
- If this `opt::option` do not contain a value, but `other` does, the contained value of type `T` is constructed using *direct-list-initialization* with the expression `other.get()`.
- If this `opt::option` contains a value, and `other` does too, the contained value of type `T` is assigned with the expression `other.get()`.

Description in the code equivalent:
```cpp
if (other.has_value()) {
    if (has_value()) {
        get() = other.get();
    } else {
        {construct}(other.get());
    }
} else {
    reset();
}
```
Where `{construct}` is a function that constructs contained object in place.

- *`noexcept`* when `std::is_reference_v<T>` or `std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>`.
- *Deleted* when `!std::is_reference_v<T>` and `!std::is_copy_constructible_v<T> || !std::is_copy_assignable_v<T>`.
- *Trivial* when `std::is_reference_v<T>` or the following are all `true`:
    - `std::is_trivially_copy_assignable_v<T>`.
    - `std::is_trivially_copy_constructible_v<T>`.
    - `std::is_trivially_destructible_v<T>`.
- *Postcondition:* `has_value() == other.has_value()`.

---

```cpp
constexpr option& operator=(option&& other) noexcept(/*see below*/);
```
Move assigns this `opt::option` from `other`.
- If this `opt::option` and `other` do not contain a value, the function do nothing.
- If this `opt::option` contains a value, but `other` does not, the contained value is destroyed.
- If this `opt::option` do not contain a value, but `other` does, the contained value of type `T` is constructed using *direct-list-initialization* with the expression `std::move(other.get())`.
- If this `opt::option` contains a value, and `other` does too, the contained value of type `T` is assigned with the expression `std::move(other.get())`.

Description in the code equivalent:
```cpp
if (other.has_value()) {
    if (has_value()) {
        get() = std::move(other.get());
    } else {
        {construct}(std::move(other.get()));
    }
} else {
    reset();
}
```
Where `{construct}` is a function that constructs contained object in place.

!> After move, `other` still holds a value (if it had before), but the value itself is moved from.

- *`noexcept`* when `std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>`.
- *Deleted* when `!std::is_reference_v<T>` and `!std::is_move_constructible_v<T> || !std::is_move_assignable_v<T>`.
- *Trivial* when `std::is_reference_v<T>` or the following are all `true`:
    - `std::is_trivially_move_assignable_v<T>`.
    - `std::is_trivially_move_constructible_v<T>`.
    - `std::is_trivially_destructible_v<T>`.
- *Postcondition:* `has_value() == other.has_value()`.

---

```cpp
template<class U = T>
constexpr option& operator=(U&& value);
```
Perfect-forwarded assignment. \
If this `opt::option` contains a value before the call, it is assigned from the expression `std::forward<U>(value)`. If does not, the contained object of type `T` is constructed using *direct-list-initialization* with the expression `std::forward<U>(value)`.

Description in the code equivalent:
```cpp
if (has_value()) {
    get() = std::forward<U>(value);
} else {
    {construct}(std::forward<U>(value));
}
```
Where `{construct}` is a function that constructs contained object in place.

- *Enabled* when the following are all `true`:
    - If `T` is a non-reference type:
      - `!std::is_same_v<remove_cvref<U>, opt::option<T>>`. ([`remove_cvref`](#remove_cvrefx))
      - `is_initializable_from<T, U>` ([`is_initializable_from`](#is_initializable_fromx-y))
      - `std::is_assignable_v<T&, U>`.
    - Otherwise, if `T` is a reference type:
      - `!std::is_same_v<remove_cvref<U>, opt::option<T>>`.
      - [`can_bind_reference<T, U>`](#can_bind_referencex-y).

---

```cpp
template<class U>
constexpr option& operator=(const option<U>& other);
```
Assigns this `opt::option` from `other`. \
- If this `opt::option` and `other` do not contain a value, the function do nothing.
- If this `opt::option` contains a value, but `other` does not, the contained value is destroyed.
- If this `opt::option` do not contain a value, but `other` does, the contained value of type `T` is constructed using *direct-list-initialization* with the expression `other.get()`.
- If this `opt::option` contains a value, and `other` does too, the contained value of type `T` is assigned with the expression `other.get()`.

Description in the code equivalent:
```cpp
if (other.has_value()) {
    if (has_value()) {
        get() = other.get();
    } else {
        {construct}(other.get());
    }
} else {
    reset();
}
```
Where `{construct}` is a function that constructs contained object in place.

- *Enabled* when the following are all `true`:
    - If `T` is a non-reference type:
      - `!std::is_same_v<T, U>`.
      - `is_initializable_from<T, const U&>`. ([`is_initializable_from`](#is_initializable_fromx-y))
      - `std::is_assignable_v<T&, const U&>`.
      - `!is_constructible_from_option<T, U>`. ([`is_constructible_from_option`](#is_constructible_from_optionx-y))
      - `!is_assignable_from_option<T, U>`. ([`is_assignable_from_option`](#is_assignable_from_optionx-y))
  - Otherwise, if `T` is a reference type:
      - `!std::is_same_v<T, U>`.
      - `can_bind_reference<T, U>`. ([`can_bind_reference`](#can_bind_referencex-y))
- *Postcondition:* `has_value() == other.has_value()`.

---

```cpp
template<class U = T>
constexpr option& operator=(option<U>&& other);
```
Assigns this `opt::option` from `other`.
- If this `opt::option` and `other` do not contain a value, the function do nothing.
- If this `opt::option` contains a value, but `other` does not, the contained value is destroyed.
- If this `opt::option` do not contain a value, but `other` does, the contained value of type `T` is constructed using *direct-list-initialization* with the expression `std::move(other.get())`.
- If this `opt::option` contains a value, and `other` does too, the contained value of type `T` is assigned with the expression `std::move(other.get())`.

Description in the code equivalent:
```cpp
if (other.has_value()) {
    if (has_value()) {
        get() = std::move(other.get());
    } else {
        {construct}(std::move(other.get()));
    }
} else {
    reset();
}
```
Where `{construct}` is a function that constructs contained object in place.

!> After move, `other` still holds a value (if it had before), but the value itself is moved from.

- *Enabled* when the following are all `true`:
    - If `T` is a non-reference type:
      - `!std::is_same_v<T, U>`.
      - `is_initializable_from<T, U&&>`. ([`is_initializable_from`](#is_initializable_fromx-y))
      - `std::is_assignable_v<T&, U&&>`.
      - `!is_constructible_from_option<T, U>`. ([`is_constructible_from_option`](#is_constructible_from_optionx-y))
      - `!is_assignable_from_option<T, U>`. ([`is_assignable_from_option`](#is_assignable_from_optionx-y))
  - Otherwise, if `T` is a reference type:
      - `!std::is_same_v<T, U>`.
      - `can_bind_reference<T, U>`. ([`can_bind_reference`](#can_bind_referencex-y))
- *Postcondition:* `has_value() == other.has_value()`.

---

### `reset`

```cpp
constexpr void reset() noexcept;
```
Destroys the contained value. \
If this `opt::option` contains a value, destroy that contained value. If does not, do nothing.
- *Postcondition:* `has_value() == false`

---

### `emplace`

```cpp
template<class... Args>
constexpr T& emplace(Args&&... args) /*lifetimebound*/;
```
Constructs the contained from `args...`. \
If this `opt::option` already contains a value, the contained value is destroyed. Initializes the contained value of type `T` using *direct-list-initialization* with `std::forward<Args>(args)...` as parameters.

Returns a reference to the contained value.

Description in the code equivalent:
```cpp
reset();
{construct}(std::forward<Args>(args)...);
```
Where `{construct}` is a function that constructs contained object in place.

---

### `try_emplace`

```cpp
template<class... Args>
constexpr T& try_emplace(Args&&... args) /*lifetimebound*/;
```
Constructs the contained value with `args...` if it's empty, otherwise does nothing.

Returns a reference to the contained value.

Description in the code equivalent:
```cpp
if (!has_value()) {
    {construct}(std::forward<Args>(args)...);
}
```
Where `{construct}` is a function that constructs contained object in place.

---

### `has_value`, `operator bool`

```cpp
constexpr bool has_value() const noexcept;

constexpr explicit operator bool() const noexcept;
```
Checks if this `opt::option` contains a value. \
Returns `true` if this `opt::option` contains a value, otherwise if it does not, return `false`.

---

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

Description in the simplified code equivalent:
```cpp
if (has_value()) {
    return predicate(get());
}
return false;
```

**Example:**
```cpp
opt::option<int> a = 3;

std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; // true

a = 0;
std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; // false

a = opt::none;
std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; // false
```

---

### `take`

```cpp
constexpr option take();
```
Takes the value out of the `opt::option`. \
Copy construct temporary value using the contained value in the `opt::option`, destroy which is left in the contained value in the `opt::option`, and return that temporary value with copy elision.
- *Postcondition:* `has_value() == false`.

Description in the code equivalent:
```cpp
option tmp{std::move(*this)};
reset();
return tmp;
```

**Example:**
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

---

### `take_if`

```cpp
template<class P>
constexpr option take_if(P&& predicate);
```
Takes the value out of the `opt::option`, but only if the `predicate` evaluates to `true`. \
Returns an empty `opt::option` if this `opt::option` does not contain a value or `predicate` evaluates to `false` with the non-const contained value as an argument; otherwise, return the expression `take()`.

Description in the simplified code equivalent:
```cpp
if (has_value() && predicate(get())) {
    return take();
}
return opt::none;
```

**Example:**
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

---

### `inspect`

```cpp
template<class F>
constexpr option& inspect(F&& fn) &;
template<class F>
constexpr const option& inspect(F&& fn) const&;
template<class F>
constexpr option& inspect(F&& fn) &&;
template<class F>
constexpr const option& inspect(F&& fn) const&&;
```
Invokes `fn` with a reference with same ref-qualifiers to the contained value if the `opt::option` contains one. If it does not, there are no effects.
Returns a reference to the this `opt::option`.

Description in the simplified code equivalent:
```cpp
if (has_value()) {
    fn(get());
}
return *this;
```

**Example:**
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

---

### `get`, `operator*`, `operator->`

```cpp
constexpr T& get() & noexcept /*lifetimebound*/;
constexpr const T& get() const& noexcept /*lifetimebound*/;
constexpr std::remove_reference_t<T>&& get() && noexcept /*lifetimebound*/;
constexpr const std::remove_reference_t<T>&& get() const&& noexcept /*lifetimebound*/;
```
Access the contained value. \
Returns a reference to the contained value of the `opt::option`. Calls the [`OPTION_VERIFY`][option-verify] macro if the `opt::option` does not contain a value. Same as `operator*`.
- *Precondition:* `has_value() == true`

---

```cpp
constexpr std::add_pointer_t<const T> operator->() const noexcept /*lifetimebound*/;
constexpr std::add_pointer_t<T> operator->() noexcept /*lifetimebound*/;
```
Access the contained value members. \ 
Returns a pointer to the contained value (`std::addressof(get())`) of the `opt::option`. Calls the [`OPTION_VERIFY`][option-verify] macro if the `opt::option` does not contain a value.
- *Precondition:* `has_value() == true`

---

```cpp
constexpr T& operator*() & noexcept /*lifetimebound*/;
constexpr const T& operator*() const& noexcept /*lifetimebound*/;
constexpr std::remove_reference_t<T>&& operator*() && noexcept /*lifetimebound*/;
constexpr const std::remove_reference_t<T>&& operator*() const&& noexcept /*lifetimebound*/;
```
Access the contained value. \
Returns a reference to the contained value of the `opt::option`. Calls the [`OPTION_VERIFY`][option-verify] macro if the `opt::option` does not contain a value. Same as `get()`.
- *Precondition:* `has_value() == true`

---

### `get_unchecked`

```cpp
constexpr T& get_unchecked() & noexcept;
constexpr const T& get_unchecked() const& noexcept;
constexpr std::remove_reference_t<T>&& get_unchecked() && noexcept;
constexpr const std::remove_reference_t<T>&& get_unchecked() const&& noexcept;
```
Access the contained value, without checking for it existence. \
Returns a reference to the contained value. *Does not* calls the [`OPTION_VERIFY`][option-verify] macro if the `opt::option` does not contain a value. Note that this method does not have the `lifetimebound` attribute. Considered for accessing type, that specified in user defined `opt::option_traits`.

---

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

---

### `value_or`

```cpp
template<class U = std::remove_cv_t<T>>
constexpr std::remove_cv_t<T> value_or(U&& def) const&;

template<class U = std::remove_cv_t<T>>
constexpr std::remove_cv_t<T> value_or(U&& def) &&;
```
Returns the contained value if `opt::option` contains one or returns a provided `def` instead. \
- *Requirements:* `std::is_copy_constructible_v<T>` and `std::is_convertible_v<U&&, T>`.

Implements the [P2218: More flexible `optional::value_or()`][P2218] proposal.

Description in the simplified equivalent:
```cpp
return has_value() ? get() : T(std::forward<U>(def));
```

---

### `value_or_construct`

```cpp
template<class... Args>
constexpr std::remove_cv_t<T> value_or_construct(Args&&... args) const&;

template<class... Args>
constexpr std::remove_cv_t<T> value_or_construct(Args&&... args) &&;

template<class U, class... Args>
constexpr std::remove_cv_t<T> value_or_construct(std::initializer_list<U> ilist, Args&&... args) const&&;

template<class U, class... Args>
constexpr std::remove_cv_t<T> value_or_construct(std::initializer_list<U> ilist, Args&&... args) &&;
```
Returns the contained value or constructs a new one from the arguments `std::forward<Args>(args)...` (or `ilist, std::forward<Args>(args)...`).

Implements the [P2218: More flexible `optional::value_or()`][P2218] proposal.

Description in the simplified equivalent:
```cpp
return has_value() ? get() : T(std::forward<Args>(args)...);
```

---

### `map_or`

```cpp
template<class U, class Fn>
constexpr remove_cvref<U> map_or(U&& def, Fn&& fn) &;
template<class U, class Fn>
constexpr remove_cvref<U> map_or(U&& def, Fn&& fn) const&;
template<class U, class Fn>
constexpr remove_cvref<U> map_or(U&& def, Fn&& fn) &&;
template<class U, class Fn>
constexpr remove_cvref<U> map_or(U&& def, Fn&& fn) const&&;
```
Returns the provided `def` value if `opt::option` does not contain a value, or invokes the `fn` with the contained value as an argument. \
Where `remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type `X`.

Description in the simplified code equivalent:
```cpp
if (has_value()) {
    return fn(get());
}
return def;
```

**Example:**
```cpp
opt::option<int> a = 2;

std::cout << a.map_or(0, [](int x) { return x * 200; }) << '\n'; // 400

a = opt::none;
std::cout << a.map_or(0, [](int x) { return x * 2; }) << '\n'; // 0
```

---

### `map_or_else`

```cpp
template<class D, class Fn>
constexpr auto map_or_else(D&& def, Fn&& fn) &;
template<class D, class Fn>
constexpr auto map_or_else(D&& def, Fn&& fn) const&;
template<class D, class Fn>
constexpr auto map_or_else(D&& def, Fn&& fn) &&;
template<class D, class Fn>
constexpr auto map_or_else(D&& def, Fn&& fn) const&&;
```
Returns the result of `def` fn with no arguments if `opt::option` does not contain a value; otherwise, returns the result of `fn` function with the contained value as an first argument.
- *Requirements:* the return type of `def` function must be the same as the return type of `fn` function.

Description in the simplified code equivalent:
```cpp
if (has_value()) {
    return fn(get());
}
return def();
```

**Example:**
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

---

### `ptr_or_null`

```cpp
constexpr std::remove_reference_t<T&>* ptr_or_null() & noexcept /*lifetimebound*/;
constexpr std::remove_reference_t<const T&>* ptr_or_null() const& noexcept /*lifetimebound*/;
```
Returns a pointer to the contained value (`std::addressof(get())`) if `opt::option` contains one. If it does not, returns `nullptr` instead.

**Example:**
```cpp
int a = 2;

opt::option<int&> b = a;

std::cout << (b.ptr_or_null() == &a) << '\n'; // true

b = opt::none;
std::cout << b.ptr_or_null() << '\n'; // 0000000000000000 (nullptr)
```

---

### `filter`

```cpp
template<class Fn>
constexpr option filter(Fn&& fn) &;
template<class Fn>
constexpr option filter(Fn&& fn) const&;
template<class Fn>
constexpr option filter(Fn&& fn) &&;
template<class Fn>
constexpr option filter(Fn&& fn) const&&;
```
Returns an empty `opt::option` if this `opt::option` does not contain a value. If it does, returns the contained value if `fn` returns `true`, and an empty `opt::option` if `fn` returns `false`.

Description in the simplified code equivalent:
```cpp
if (has_value() && fn(get())) {
    return get();
}
return opt::none;
```

The function is called with reference (possible `const`), and the result of it converted to `bool`.
The returned value is constructed with forwarded contained value.

> [!NOTE]
> `opt::option` does not modify value inside function (despite having non-const reference). You can modify contained value inside function via passed reference to it.

**Example:**
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

---

### `and_then`

```cpp
template<class Fn>
constexpr option<U> and_then(Fn&& fn) &;
template<class Fn>
constexpr option<U> and_then(Fn&& fn) const&;
template<class Fn>
constexpr option<U> and_then(Fn&& fn) &&;
template<class Fn>
constexpr option<U> and_then(Fn&& fn) const&&;
```
Returns an empty `opt::option` if this `opt::option` does not contain a value. If it does, invokes `fn` function with the contained value as an first argument, and then returns the result of that invocation. \
This operation is also sometimes called *flatmap*.
- *Requirements:* the result type of `fn` must be a specialization of `opt::option`.

Description in the simplified code equivalent:
```cpp
if (has_value()) {
    return fn(get());
}
return opt::none;
```

**Example:**
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

---

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

Description in the simplified code equivalent:
```cpp
if (has_value()) {
    return opt::option{fn(get())};
}
return opt::none;
```

**Example:**
```cpp
const auto to_float = [](int x) {
    return float(x) / 2.f;
};

opt::option<int> a = 1;

std::cout << a.map(to_float).get() << '\n'; // 0.5

a = opt::none;
std::cout << a.map(to_float).has_value() << '\n'; // false
```

---

### `or_else`

```cpp
template<class Fn>
constexpr option or_else(Fn&& fn) const&;
template<class Fn>
constexpr option or_else(Fn&& fn) &&;
```
If `opt::option` contains a value, returns it. If does not, returns the result of `fn` function with *no arguments*. \
Similar to [`std::optional<T>::or_else`](https://en.cppreference.com/w/cpp/utility/optional/or_else).
- *Requirements:* the result type of `fn` (without any cv-qualifiers) must be the same as `opt::option<T>`.

Description in the simplified code equivalent:
```cpp
if (has_value()) {
    return *this;
}
return fn();
```

**Example:**
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

---

### `swap`

```cpp
template<class U>
constexpr void swap(option<U>& other) noexcept(/*see below*/);
```
Swaps the options.
- `!this->has_value()` and `!other.has_value()` are true, do nothing.
- `this->has_value()` and `other.has_value()` are true, swap the contained values by calling `using std::swap; swap(this->get(), other.get())`.
- `this->has_value()` and `!other.has_value()` are true, construct the `other` with `std::move(this->get())` and destruct the contained value of `*this`.
- `!this->has_value()` and `other.has_value()` are true, construct the `*this` with `std::move(other.get())` and destruct the contained value of `other`.

Description in the code equivalent:
```cpp
if (!has_value() && !other.has_value()) {
    return;
}
if (has_value() && other.has_value()) {
    swap(get(), other.get());
    return;
}
if (has_value()) {
    other.{construct}(std::move(get()));
    reset();
    return;
}
{construct}(std::move(other.get()));
other.reset();
```
Where `{construct}` is a function that constructs contained object in place.

- *`noexcept`* when `std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<U> && std::is_nothrow_swappable_with_v<T, U>`.
- 
---

### `begin`

```cpp
constexpr iterator begin() noexcept /*lifetimebound*/;
constexpr const_iterator begin() const noexcept /*lifetimebound*/;
```
If contains a value, returns an iterator to the contained value. Otherwise, a past-the-end iterator.

The returned iterator is pointing to the range with a single element, which is the contained value inside option.

Implements the [P3168: Give `std::optional` Range Support][P3168] proposal.

**Example:**
```cpp
opt::option<int> a = 1;

std::cout << *(a.begin()) << '\n'; // 1

for (int& x : a) {
    std::cout << x << '\n'; // 1
    x = 2;
    std::cout << x << '\n'; // 2
}

a = opt::none;
for (const int& x : a) {
    std::cout << x << '\n';
}

opt::option<std::array<int, 3>> b{{1, 2, 3}};
for (const auto& v : b) {
    for (const int& x : v) {
        std::cout << x << ' '; // 1 2 3
    }
    std::cout << '\n';
}
```

---

### `end`

```cpp
constexpr iterator end() noexcept /*lifetimebound*/;
constexpr const_iterator end() const noexcept /*lifetimebound*/;
```
Returns a past-the-end iterator. Equivalent to `begin() + has_value()`.

Implements the [P3168: Give `std::optional` Range Support][P3168] proposal.

**Example:**
```cpp
opt::option<int> a = 1;
std::cout << *(a.begin()) << '\n'; // 1
std::cout << *(--(a.end())) << '\n'; // 1
std::cout << (a.begin() == a.end()) << '\n'; // false
std::cout << (a.begin() + 1 == a.end()) << '\n'; // true

a = opt::none;
std::cout << (a.begin() == a.end()) << '\n'; // true
```

## Non-member functions

### `opt::zip`

```cpp
template<class Options>
constexpr /*see below*/ zip(Options&&... options);
```
Zips `options...` into `opt::option<std::tuple<...>>`. \
If every `options...` contains the values, returns the `std::tuple` wrapped in `opt::option` with the forwarded containing value from `options...`. If any `options...` does not contain the value, return an empty `opt::option`.
- *Enabled* when every `Options...` (possible with cv-qualifiers) is the specializations of `opt::option`.

The return type of `zip` is `opt::option<std::tuple<typename remove_cvref<Options>::value_type...>>`. \
Where `remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type `X`.

Description in the simplified code equivalent:
```cpp
if ((options.has_value() && ...)) {
    return std::tuple{options.get()...};
} else {
    return opt::none;
}
```

**Example:**
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

---

### `opt::zip_with`

```cpp
template<class Fn, class... Options>
constexpr auto zip_with(Fn&& fn, Options&&... options);
```
If every `options...` contains the values, returns the result of `fn` function with every `options...` containing values as the `fn` arguments. If any of `options...` does not contain a value, returns an empty `opt::option`.
- *Enabled* when every `Options...` (possible with cv-qualifiers) is the specializations of `opt::option`.

Description in the simplified code equivalent:
```cpp
if ((options.has_value() && ...)) {
    return fn(options.get()...);
} else {
    return opt::none;
}
```

**Example:**
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

---

### `opt::option_cast`

```cpp
template<class To, class From>
constexpr option<To> option_cast(const option<From>& value);
template<class To, class From>
constexpr option<To> option_cast(option<From>&& value);
```
Casts `opt::option<From>` to `opt::option<To>`. \
If `opt::option<From>` contains a value, `static_cast`s it to the type `To` and wraps it into `opt::option<To>`. If `opt::option<From>` does not contain a value, returns an empty `opt::option<To>`.

Description in the simplified code equivalent:
```cpp
return value.map([](const From& x) { return To(x); });
```

**Example:**
```cpp
opt::option<float> a{2.5f};
opt::option<int> b;

b = opt::option_cast<int>(a);
std::cout << *b << '\n'; // 2

a = opt::none;
b = opt::option_cast<int>(a);
std::cout << b.has_value() << '\n'; // false
```

---

### `opt::from_nullable`

```cpp
template<class T>
constexpr option<T&> from_nullable(T* const nullable_ptr) noexcept;
```
Returns a reference option pointed to the dereferenced value of provided pointer if it is not equal to `nullptr`; otherwise, returns empty option.

Description in the code equivalent:
```cpp
if (nullable_ptr == nullptr) {
    return opt::none;
} else {
    return *nullable_ptr;
}
```

---

### `opt::as_option`

```cpp
template<class T>
constexpr option<T> as_option(const T& value) noexcept;
```
Converts `value` into `opt::option<T>` by assigning the underlying value to `value`.
The returned option can exists in an empty state.

- *Requirements:* must be [trivially copyable][trivially-copyable] and `opt::option_traits<T>::max_level` greater than 0.

Description in the code equivalent:
```cpp
option<T> result;
result.get_unchecked() = value;
return result;
```

**Example:**
```cpp
struct my_type {
    int x;
};

template<>
struct opt::option_traits<my_type> {
    static constexpr std::uintmax_t max_level = 1;

    static constexpr std::uintmax_t get_level(const my_type* const value) noexcept {
        return value->x - 100;
    }
    static constexpr void set_level(my_type* const value, std::uintmax_t) noexcept {
        value->x = 100;
    }
};

opt::option<my_type> a = opt::as_option(my_type{1});
std::cout << opt::io(a.map(&my_type::x), "empty") << '\n'; //$ 1

a = opt::as_option(my_type{100});
std::cout << opt::io(a.map(&my_type::x), "empty") << '\n'; //$ empty
```

---

### `opt::swap`

```cpp
template<class T, class U>
constexpr void swap(option<T>& left, option<U>& right) noexcept(/*see below*/);
```
Exchanges the state of `left` with that of `right`. Calls `left.swap(right)`.
- *Enabled* when `std::is_move_constructible_v<T>`, `std::is_move_constructible_v<U>` and `std::is_swappable_with_v<T, U>` are true.
- *`noexcept`* when `std::is_nothrow_move_constructible_v<T>`, `std::is_nothrow_move_constructible_v<U>` and `std::is_nothrow_swappable_with_v<T, U>` are true.

Description in the code equivalent:
```cpp
left.swap(right);
```

---

### `opt::get`

```cpp
template<std::size_t I, class T>
constexpr auto get(opt::option<T>& x /*lifetimebound*/) noexcept;
template<std::size_t I, class T>
constexpr auto get(const opt::option<T>& x /*lifetimebound*/) noexcept;
template<std::size_t I, class T>
constexpr auto get(opt::option<T>&& x /*lifetimebound*/) noexcept;
template<std::size_t I, class T>
constexpr auto get(const opt::option<T>&& x /*lifetimebound*/) noexcept;
```
Returns the result of `using std::get; get<I>` (ADL) in an reference option (e.g. `opt::option<T&>`) if it does contain one; otherwise, returns `opt::none`.

In return type, a `T` in `opt::option<T>` has the same reference qualifiers as for the first parameter of these overloads.

Description in the simplified code equivalent:
```cpp
if (x.has_value()) {
    return get<I>(x.get());
}
return opt::none;
```

---

```cpp
template<class T, class OptT>
constexpr auto get(opt::option<OptT>& x /*lifetimebound*/) noexcept;
template<class T, class OptT>
constexpr auto get(const opt::option<OptT>& x /*lifetimebound*/) noexcept;
template<class T, class OptT>
constexpr auto get(opt::option<OptT>&& x /*lifetimebound*/) noexcept;
template<class T, class OptT>
constexpr auto get(const opt::option<OptT>&& x /*lifetimebound*/) noexcept;
```
Returns the result of `using std::get; get<T>` (ADL) in an reference option (e.g. `opt::option<T&>`) if it does contain one; otherwise, returns `opt::none`.

In return type, a `T` in `opt::option<T>` has the same reference qualifiers as for the first parameter of these overloads.

Description in the simplified code equivalent:
```cpp
if (x.has_value()) {
    return get<T>(x.get());
}
return opt::none;
```

---

```cpp
template<std::size_t I, class... Ts>
constexpr auto get(std::variant<Ts...>& v /*lifetimebound*/) noexcept;
template<std::size_t I, class... Ts>
constexpr auto get(const std::variant<Ts...>& v /*lifetimebound*/) noexcept;
template<std::size_t I, class... Ts>
constexpr auto get(std::variant<Ts...>&& v /*lifetimebound*/) noexcept;
template<std::size_t I, class... Ts>
constexpr auto get(const std::variant<Ts...>&& v /*lifetimebound*/) noexcept;
```
Returns the reference option (e.g. `opt::option<T&>`) to an holded value of the `std::variant` at index `I` if `std::variant` contain it.
Otherwise, returns `opt::none`.

In return type, a `T` in `opt::option<T>` has the same reference qualifiers as for the first parameter of these overloads.

Description in the simplified code equivalent:
```cpp
auto ptr = std::get_if<I>(v);
if (ptr == nullptr) {
    return opt::none;
}
return *ptr;
```

---

```cpp
template<class T, class... Ts>
constexpr auto get(std::variant<Ts...>& v /*lifetimebound*/) noexcept;
template<class T, class... Ts>
constexpr auto get(const std::variant<Ts...>& v /*lifetimebound*/) noexcept;
template<class T, class... Ts>
constexpr auto get(std::variant<Ts...>&& v /*lifetimebound*/) noexcept;
template<class T, class... Ts>
constexpr auto get(const std::variant<Ts...>&& v /*lifetimebound*/) noexcept;
```
Returns the reference option (e.g. `opt::option<T&>`) to an holded value of the `std::variant` with type `T` if `std::variant` contain it.
Otherwise, returns `opt::none`.

In return type, a `T` in `opt::option<T>` has the same reference qualifiers as for the first parameter of these overloads.

Description in the simplified code equivalent:
```cpp
auto ptr = std::get_if<T>(v);
if (ptr == nullptr) {
    return opt::none;
}
return *ptr;
```

---

**Example:**
```cpp
opt::option<std::tuple<int, float>> a{1, 2.f};

if (auto b = opt::get<0>(a)) {
    std::cout << *b << '\n'; // 1
    *b = 2;
}
std::cout << *opt::get<int>(a) << '\n'; // 2

opt::option<std::variant<int, float>> c{123};

if (auto d = opt::get<int>(c)) {
    std::cout << *d << '\n'; // 123
}
c = 2.f;
if (auto d = opt::get<float>(c)) {
    std::cout << *d << '\n'; // 2
}
```

---

### `opt::io`

```cpp
template<class T>
constexpr /*unspecified*/ io(const opt::option<T>& x /*lifetimebound*/) noexcept;
template<class T>
constexpr /*unspecified*/ io(opt::option<T>& x /*lifetimebound*/) noexcept;
```
Constructs the implementation detail return type with a reference to the `opt::option`.

The return type has `operator<<` to write to stream.
These overloads accept a template stream parameter and uses `operator<<` on them if option contains a value; otherwise, does nothing.
Returns a non-`const` reference to the passed stream argument.

---

```cpp
template<class T, class NoneCase>
constexpr /*unspecified*/ io(const opt::option<T>& x /*lifetimebound*/, const NoneCase& none_case /*lifetimebound*/) noexcept;
template<class T, class NoneCase>
constexpr /*unspecified*/ io(opt::option<T>& x /*lifetimebound*/, NoneCase& none_case /*lifetimebound*/) noexcept;
```
Constructs the implementation detail return type with a reference to the `opt::option` and a reference to the `none_case`.

The return type has `operator<<` and `operator>>` to write/read to/from stream.
These overloads accept a template stream parameter and uses `operator<<` and `operator>>` on them if option contains a value;
otherwise, uses `none_case` instead of option.
Returns a non-`const` reference to the passed stream argument.

---

**Example:**
```cpp
opt::option<int> a;
std::cout << opt::io(a) << '\n'; //
std::cout << opt::io(a, "empty") << '\n'; // empty

a = 1;
std::cout << opt::io(a) << '\n'; // 1
std::cout << opt::io(a, "empty") << '\n'; // 1
```

---

### `opt::at`

```cpp
template<class T>
constexpr auto at(T&& container /*lifetimebound*/, std::size_t index) noexcept;
```
Returns the reference option (e.g. `opt::option<T&>`) to an held value at `index` of the `container` if `index` is a valid index (bounds checking is performed).
Otherwise, returns `opt::none`.

In return type, a `T` in `opt::option<T>` has the same reference qualifiers as for the first parameter of these overloads.

Uses the `.size()` to check if index is in a valid range (`index < container.size()`)
and `.operator[]` to access specified element at `index` (`std::forward<T>(container)[index]`).

None of these function should throw any exceptions.

Description in the simplified code equivalent:
```cpp
if (index >= container.size()) {
    return opt::none;
}
return container[index];
```

**Example:**
```cpp
std::vector<int> a{{10, 11, 12, 13, 14}};

std::cout << (opt::at(a, 0) == 10) << '\n'; // true
std::cout << (opt::at(a, 5) == 15) << '\n'; // false

a = {1, 2};
std::cout << (opt::at(a, 0) == 1) << '\n'; // true
std::cout << (opt::at(a, 1) == 2) << '\n'; // true
std::cout << (opt::at(a, 2) == 3) << '\n'; // false
```

---

### `opt::at_front`

```cpp
template<class T>
constexpr auto at_front(T&& container /*lifetimebound*/) noexcept;
```
Returns the reference option (e.g. `opt::option<T&>`) to the first element in the `container` if it is not empty (bounds checking is performed).
Otherwise, returns `opt::none`.

In return type, a `T` in `opt::option<T>` has the same reference qualifiers as for the first parameter of these overloads.

Uses the `.size()` to check if container is empty (`container.size() == 0`)
and `.front()` to access first element (`std::forward<T>(container).front()`).

None of these function should throw any exceptions.

Description in the simplified code equivalent:
```cpp
if (container.size() == 0) {
    return opt::none;
}
return container.front();
```

> [!NOTE]
> Calling `opt::at_front` on an empty container **does not** causes undefined behavior.

---

### `opt::at_back`

```cpp
template<class T>
constexpr auto at_back(T&& container /*lifetimebound*/) noexcept;
```
Returns the reference option (e.g. `opt::option<T&>`) to the last element in the `container` if it is not empty (bounds checking is performed).
Otherwise, returns `opt::none`.

In return type, a `T` in `opt::option<T>` has the same reference qualifiers as for the first parameter of these overloads.

Uses the `.size()` to check if container is empty (`container.size() == 0`)
and `.back()` to access last element (`std::forward<T>(container).back()`).

None of these function should throw any exceptions.

Description in the simplified code equivalent:
```cpp
if (container.size() == 0) {
    return opt::none;
}
return container.back();
```

> [!NOTE]
> Calling `opt::at_back` on an empty container **does not** causes undefined behavior.

---

### `opt::flatten`

```cpp
template<class Option>
constexpr auto flatten(Option&& opt);
```
Flattens `opt::option` up to the first level (i.e. `opt::option<Y>`, where `Y` is not an `opt::option`).

- *Requirements:* the `Option` template parameter must be at least `opt::option<X>` (where `X` can be any type).

The type `Y` is defined to be a reference type to the contained value if `Option` is lvalue reference; otherwise, a prvalue type.
Examples:
- `flatten<opt::option<opt::option<int>>>(...)` -> `opt::option<int>`
- `flatten<opt::option<opt::option<int>>&>(...)` -> `opt::option<int&>`
- `flatten<opt::option<opt::option<int&>>>(...)` -> `opt::option<int&>`
- `flatten<opt::option<opt::option<int&>>&>(...)` -> `opt::option<int&>`
- `flatten<opt::option<opt::option<int&&>>&>(...)` -> `opt::option<int&&>`
- `flatten<opt::option<opt::option<int&>>&&>(...)` -> `opt::option<int&>`

> [!NOTE]
> Since there is no way to distinguish prvalue from xvalue in the function's arguments, the function always assumes that prvalue was passed.

Description in the simplified code equivalent to the second level of `opt::option` nesting:
```cpp
if (opt.has_value()) {
    return opt.get();
}
return opt::none;
```

**Example:**
```cpp
opt::option<opt::option<int>> a = 1;

opt::option<int> b = opt::flatten(a);
std::cout << *b << '\n'; // 1

*a = opt::none;
std::cout << opt::flatten(a).has_value() << '\n'; // false
a = opt::none;
std::cout << opt::flatten(a).has_value() << '\n'; // false

opt::option<opt::option<opt::option<float>>> c = 2.f;

opt::option<float> d = opt::flatten(c);
std::cout << *d << '\n'; // 2

c.get().get() = opt::none;
std::cout << opt::flatten(c).has_value() << '\n'; // false
c.get() = opt::none;
std::cout << opt::flatten(c).has_value() << '\n'; // false
c = opt::none;
std::cout << opt::flatten(c).has_value() << '\n'; // false
```

---

### `opt::unzip`

```cpp
template<class Option>
constexpr auto unzip(Option&& opt);
```
Unzips `opt::option` that contains a *tuple like* type, into the *tuple like* object that contains values that wrapped into `opt::option` (e.g. `opt::option<std::pair<int, float>>` -> `std::pair<opt::option<int>, opt::option<float>>`).

If `opt::option` contains the value, return *tuple like* object that contains `opt::option`s of the *tuple like* object contained types. If `opt::option` does not contain the value, return *tuple like* object that contains empty `opt::option`s.
- *Requirements:* the `opt` contained value (without cv-qualifiers) must be *tuple like* object.

*tuple like* type (without cv-qualifiers) is a specialization of `std::array`, `std::pair` or `std::tuple`.

Description in the simplified code equivalent for `std::pair`:
```cpp
if (opt.has_value()) {
    return std::pair{opt::option{opt->first}, opt::option{opt->second}};
} else {
    return std::pair{opt::option<First>{opt::none}, opt::option<Second>{opt::none}};
}
```

**Example:**
```cpp
opt::option<std::tuple<int, float>> a{1, 2.5f};

std::tuple<opt::option<int>, opt::option<float>> unzipped_a;
unzipped_a = opt::unzip(a);

std::cout << std::get<0>(unzipped_a).get() << '\n'; // 1
std::cout << std::get<1>(unzipped_a).get() << '\n'; // 2.5

opt::option<std::array<int, 3>> b = opt::none;

std::array<opt::option<int>, 3> unzipped_b = opt::unzip(b);

std::cout << (!unzipped_b[0] && !unzipped_b[1] && !unzipped_b[2]) << '\n'; // true
```

---

### `opt::lookup`

```cpp
template<class T, class K>
constexpr auto lookup(T& associative_container /*lifetimebound*/, K&& key);
```

Finds an element with key equivalent to `key`. If no such element is found, returns `opt::none`.

Returns a reference option (e.g. `opt::option<T&>`) to the found element in the `associative_container`.

In return type, a `T` in `opt::option<T>` has the same reference qualifiers as for the `associative_container` parameter of these overloads.

Uses the `.find(std::forward<K>(key))` to find an element
and `.end()` to get the past-the-end iterator (the one that gets returns when element is not found).

If type `T` (without cv-qualifiers) has the nested type `mapped_type` being defined
it is considered that method `.find` returns an iterator to `std::pair` in which `.second` is a reference to the requested element.

Otherwise, method `.find` simply returns an iterator to the requested element.

Description in the simplified code equivalent for `map`-like types:
```cpp
auto it = associative_container.find(key);
if (it == associative_container.end()) {
    return opt::none;
}
return it->second;
```

Description in the simplified code equivalent for `set`-like types:
```cpp
auto it = associative_container.find(key);
if (it == associative_container.end()) {
    return opt::none;
}
return *it;
```

---

### `operator|`

```cpp
template<class T, class U>
constexpr T operator|(const option<T>& left, U&& right);

template<class T, class U>
constexpr T operator|(option<T>&& left, U&& right);
```
If `left` does not contain a value, returns `right` value instead. If `left` does, just returns `left`.

- *Enabled* when `U` (without cv-qualifiers) is not `opt::option`.

Same as [`opt::option<T>::value_or`](#value_or).

---

```cpp
template<class T>
constexpr option<T> operator|(const option<T>& left, const option<T>& right);

template<class T>
constexpr option<T> operator|(option<T>&& left, const option<T>& right);

template<class T>
constexpr option<T> operator|(const option<T>& left, option<T>&& right);

template<class T>
constexpr option<T> operator|(option<T>&& left, option<T>&& right);
```
Returns `left` if it does contains a value, or returns `right` if `left` does not.

Description in the simplified code equivalent:
```cpp
if (left.has_value()) {
    return left;
}
return right;
```

**Example:**
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
template<class T, class U>
constexpr option<T>& operator|=(option<T>& left, U&& right);
```
Copy assigns `right` to `left` if the `left` does not contain a value. \
Returns a reference to `left`.

Description in the simplified code equivalent:
```cpp
if (!left.has_value()) {
    left = right;
}
return left;
```

---

### `operator&`

```cpp
template<class T, class U>
constexpr option<U> operator&(const option<T>& left, const option<U>& right);

template<class T, class U>
constexpr option<U> operator&(const option<T>& left, option<U>&& right);
```
Returns an empty `opt::option` if `left` does not contain a value, or if `left` does, returns `right`.

Description in the simplified code equivalent:
```cpp
if (left.has_value()) {
    return right;
}
return opt::none;
```

**Example:**
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

---

### `operator^`

```cpp
template<class T>
constexpr option<T> operator^(const option<T>& left, const option<T>& right);

template<class T>
constexpr option<T> operator^(option<T>&& left, const option<T>& right);

template<class T>
constexpr option<T> operator^(const option<T>& left, option<T>&& right);

template<class T>
constexpr option<T> operator^(option<T>&& left, option<T>&& right);
```
Returns `opt::option` that contains a value if exactly one of `left` and `right` contains a value, otherwise, returns an empty `opt::option`.

Description in the simplified code equivalent:
```cpp
if (left.has_value() && !right.has_value()) {
    return left;
}
if (!left.has_value() && right.has_value()) {
    return right;
}
return opt::none;
```

**Example:**
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

---

### `operator==`

```cpp
template<class T1, class T2>
constexpr bool operator==(const option<T1>& left, const option<T2>& right);
```
If `left` and `right` contains the values, then compare values using operator `==`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `==`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() == right.get()` is well-formed.

---

```cpp
template<class T>
constexpr bool operator==(const option<T>& left, none_t) noexcept;
```
Returns `!left.has_value()`.

---

```cpp
template<class T>
constexpr bool operator==(none_t, const option<T>& right) noexcept;
```
Returns `!right.has_value()`.

---

```cpp
template<class T1, class T2>
constexpr bool operator==(const option<T1>& left, const T2& right);
```
If `left` contains a value, then compare it with `right` using operator `==`; otherwise, return `false`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() == right` is well-formed.

---

```cpp
template<class T1, class T2>
constexpr bool operator==(const T1& left, const option<T2>& right);
```
If `right` contains a value, then compare it with `left` using operator `==`; otherwise, return `false`.
- *Enabled* when the implicit conversion to `bool` of expression `left == right.get()` is well-formed.

### `operator!=`
```cpp
template<class T1, class T2>
constexpr bool operator!=(const option<T1>& left, const option<T2>& right);
```
If `left` and `right` contains the values, then compare values using operator `!=`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `!=`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() != right.get()` is well-formed.

---

```cpp
template<class T>
constexpr bool operator!=(const option<T>& left, none_t) noexcept;
```
Returns `left.has_value()`.

---

```cpp
template<class T>
constexpr bool operator!=(none_t, const option<T>& right) noexcept;
```
Returns `right.has_value()`.

---

```cpp
template<class T1, class T2>
constexpr bool operator!=(const option<T1>& left, const T2& right);
```
If `left` contains a value, then compare it with `right` using operator `!=`; otherwise, return `true`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() != right` is well-formed.

---

```cpp
template<class T1, class T2>
constexpr bool operator!=(const T1& left, const option<T2>& right);
```
If `right` contains a value, then compare it with `left` using operator `!=`; otherwise, return `true`.
- *Enabled* when the implicit conversion to `bool` of expression `left != right.get()` is well-formed.

### `operator<`
```cpp
template<class T1, class T2>
constexpr bool operator<(const option<T1>& left, const option<T2>& right);
```
If `left` and `right` contains the values, then compare values using operator `<`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `<`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() < right.get()` is well-formed.

---

```cpp
template<class T>
constexpr bool operator<(const option<T>& left, none_t) noexcept;
```
Returns `false`.

---

```cpp
template<class T>
constexpr bool operator<(none_t, const option<T>& right) noexcept;
```
Returns `right.has_value()`.

---

```cpp
template<class T1, class T2>
constexpr bool operator<(const option<T1>& left, const T2& right);
```
If `left` contains a value, then compare it with `right` using operator `<`; otherwise, return `true`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() < right` is well-formed.

---

```cpp
template<class T1, class T2>
constexpr bool operator<(const T1& left, const option<T2>& right);
```
If `right` contains a value, then compare it with `left` using operator `<`; otherwise, return `false`.
- *Enabled* when the implicit conversion to `bool` of expression `left < right.get()` is well-formed.

### `operator<=`
```cpp
template<class T1, class T2>
constexpr bool operator<=(const option<T1>& left, const option<T2>& right);
```
If `left` and `right` contains the values, then compare values using operator `<=`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `<=`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() <= right.get()` is well-formed.

---

```cpp
template<class T>
constexpr bool operator<=(const option<T>& left, none_t) noexcept;
```
Returns `!left.has_value()`.

---

```cpp
template<class T>
constexpr bool operator<=(none_t, const option<T>& right) noexcept;
```
Returns `true`.

---

```cpp
template<class T1, class T2>
constexpr bool operator<=(const option<T1>& left, const T2& right);
```
If `left` contains a value, then compare it with `right` using operator `<=`; otherwise, return `true`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() <= right` is well-formed.

---

```cpp
template<class T1, class T2>
constexpr bool operator<=(const T1& left, const option<T2>& right);
```
If `right` contains a value, then compare it with `left` using operator `<=`; otherwise, return `false`.
- *Enabled* when the implicit conversion to `bool` of expression `left <= right.get()` is well-formed.

### `operator>`
```cpp
template<class T1, class T2>
constexpr bool operator>(const option<T1>& left, const option<T2>& right);
```
If `left` and `right` contains the values, then compare values using operator `>`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `>`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() > right.get()` is well-formed.

---

```cpp
template<class T>
constexpr bool operator>(const option<T>& left, none_t) noexcept;
```
Returns `left.has_value()`.

---

```cpp
template<class T>
constexpr bool operator>(none_t, const option<T>& right) noexcept;
```
Returns `false`.

---

```cpp
template<class T1, class T2>
constexpr bool operator>(const option<T1>& left, const T2& right);
```
If `left` contains a value, then compare it with `right` using operator `>`; otherwise, return `false`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() > right` is well-formed.

---

```cpp
template<class T1, class T2>
constexpr bool operator>(const T1& left, const option<T2>& right);
```
If `right` contains a value, then compare it with `left` using operator `>`; otherwise, return `true`.
- *Enabled* when the implicit conversion to `bool` of expression `left > right.get()` is well-formed.

### `operator>=`
```cpp
template<class T1, class T2>
constexpr bool operator>=(const option<T1>& left, const option<T2>& right);
```
If `left` and `right` contains the values, then compare values using operator `>=`; otherwise, compare `left.has_value()` and `right.has_value()` using operator `>=`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() >= right.get()` is well-formed.

---

```cpp
template<class T>
constexpr bool operator>=(const option<T>& left, none_t) noexcept;
```
Returns `true`.

---

```cpp
template<class T>
constexpr bool operator>=(none_t, const option<T>& right) noexcept;
```
Returns `!right.has_value()`.

---

```cpp
template<class T1, class T2>
constexpr bool operator>=(const option<T1>& left, const T2& right);
```
If `left` contains a value, then compare it with `right` using operator `=>`; otherwise, return `false`.
- *Enabled* when the implicit conversion to `bool` of expression `left.get() >= right` is well-formed.

---

```cpp
template<class T1, class T2>
constexpr bool operator>=(const T1& left, const option<T2>& right);
```
If `right` contains a value, then compare it with `left` using operator `=>`; otherwise, return `true`.
- *Enabled* when the implicit conversion to `bool` of expression `left >= right.get()` is well-formed.

## Helpers

### `std::hash<opt::option>` :id=stdhashoptoption

```cpp
template<class T>
struct std::hash<opt::option<T>>;
```
The template specialization of `std::hash` for the `opt::option` gives users a ability to calculate hash of the contained value. \
If `opt::option` contains the value, returns hash of that value. If `opt::option` does not, returns the expression `static_cast<std::size_t>(-96391)` as an empty value hash.
- *Enabled* when `std::is_default_constructible_v<std::hash<std::remove_const_t<T>>>` is `true`.
- *`noexcept`* when the expression `noexcept(std::hash<std::remove_const_t<T>>{}(std::declval<const T&>()))` is `true`.

Description in the code equivalent:
```cpp
if (value.has_value()) {
    return std::hash<T>{}(value.get());
}
return {disengaged_hash};
```
Where `{disengaged_hash}` is a magic hash value.

**Example:**
```cpp
opt::option<int> a{12345};

std::cout << std::hash<opt::option<int>>{}(a) << '\n'; // [some hash]

a = opt::none;
std::cout << std::hash<opt::option<int>>{}(a) << '\n'; // [some empty option hash]
```

---

### `opt::none_t`

```cpp
struct none_t;
```
The tag type used to indicate `opt::option` with the contained value in uninitialized state. \
`opt::none_t` do not have a default constructor.

---

### `opt::none`

```cpp
inline constexpr none_t none{/*special value*/};
```
The `opt::none` variable is a `constexpr` value of type [`opt::none_t`](#none_t) that is used to indicate `opt::option` with the contained value in uninitialized state.

---

### `opt::bad_access`

```cpp
class bad_access;
```
The exception type of an object to be thrown by [`opt::option<T>::value`, `opt::option<T>::value_or_throw`](#value-value_or_throw) methods, if `opt::option` does not contain a value inside it.

---

### `opt::option_traits`

```cpp
template<class T, class = void>
struct option_traits;
```

`opt::option_traits` allows `opt::option` to remove size overhead of having additional `bool` flag to indicate it's state.

Each complete specialization of `opt::option_traits` must have `max_level` static constexpr variable.

If `max_level` is equal to 0, `opt::option_traits` is disabled and `opt::option` will not use it.
Otherwise, `opt::option_traits` specialization must have at least these two static methods: `get_level`, `set_level`.

See [guide](./custom_traits_guide.md) for creating custom option traits.

---

#### `get_level`

```cpp
static *constexpr std::uintmax_t get_level(const T*) noexcept;
```
The `opt::option` uses the `get_level` to determine if the contained value is considered empty.

The passed `const T*` pointer to the underlying object must be non-`nullptr`.

If `get_level` returns `0`, the contained value is empty.
Otherwise, it is not empty.

Other non-`0` values are used for nested `opt::option`s.
Level 0 indicates that the latest `opt::option` (the one that holds a value) is empty.
Level 1 indicates that one after the latest `opt::option` (the one that holds `opt::option`, that holds a value) is empty and etc.
until `max_level` is reached.

This function is called when `opt::option` is needed to check if it contains a value.

---

#### `set_level`

```cpp
static *constexpr void set_level(T*, std::uintmax_t) noexcept;
```

The `opt::option` uses the `set_level` to set the level depth state inside `opt::option`.

The passed `std::uintmax_t` level argument must be strictly less than `max_level`,
and the provided `T*` pointer to the underlying object must be non-`nullptr`.

Usually `set_level` is called after the original object is destructed/uninitialized, but also `set_level` can be called multiple times in a row.

!> The `noexcept` specifier is required (without it the program will not compile).

---

### `opt::sentinel_option_traits`

```cpp
template<class T, class = void>
struct sentinel_option_traits;
```

Same as `opt::option_traits`, but the only requirement is to provide a `static` `constexpr` member with name `sentinel_value`.

The `sentinel_value` member denotes the value which will be used in `get_level` and `set_level` static methods. \

Requirements:
- `T` must be equality comparable with itself (e.g. `std::declval<const T&>() == std::declval<const T&>()` must be a valid expression).
- `T` must be assignable by itself (e.g. `std::declval<T&>() = std::declval<const T&>()` must be a valid expression).

Explanation:
`opt::option_traits` has a specialization that only enables when `opt::sentinel_option_traits` is a complete type. \
In this specialization it defines `max_level` to be `1`, `get_level` and `set_level` `constexpr` `static` methods that uses `sentinel_value` to identify/set the empty state of `opt::option`.

**Example:**
```cpp
struct my_type {
    unsigned x;

    bool operator==(const my_type& other) const { return x == other.x; }
};
template<>
struct opt::sentinel_option_traits<my_type> {
    static constexpr my_type sentinel_value{0u};
};

opt::option<my_type> a{5u};

std::cout << (sizeof(a) == sizeof(my_type)) << '\n'; // true
std::cout << a->x << '\n'; // 5

a.reset();
std::cout << a.get_unchecked().x << '\n'; // 0

a = my_type{1u};
std::cout << a->x << '\n'; // 1
a->x = 0u;
std::cout << a.has_value() << '\n'; // false
```

---

### `opt::make_option`

```cpp
template<class T>
constexpr opt::option<std::decay_t<T>> make_option(T&& value);
```
Creates `opt::option` from `value`. Returns `opt::option<std::decay_t<T>>{std::forward<T>(value)}`

---

```cpp
template<class T, class... Args>
constexpr opt::option<T> make_option(Args&&... args);
```
Creates `opt::option` from `args...`. Returns `opt::option<T>{std::in_place, std::forward<Args>(args)...}`.

---

```cpp
template<class T, class U, class... Args>
constexpr opt::option<T> make_option(std::initializer_list<U> ilist, Args&&... args);
```
Creates `opt::option` from `ilist` and `args...`. Returns `opt::option<T>{std::in_place, ilist, std::forward<Args>(args)...}`.

### `opt::is_option`

```cpp
template<class T>
struct is_option;

template<class T>
inline constexpr bool is_option_v = is_option<T>::value;
```
Checks whether `T` is a specialization of the `opt::option` type.

Provides the member constant `value` of type `bool` that is equal to `true`, if `T` is a specialization of the `opt::option` type. Otherwise, value is equal to `false`.

### `opt::option_tag`

```cpp
struct option_tag;
```
Tag used in [`boost::pfr::is_reflectable`][pfr is_reflectable] (`pfr::is_reflectable`) for second template parameter `WhatFor`.

Does not do anything when the `boost.pfr` or `pfr` library is not used.

### `opt::sentinel`

```cpp
template<class T, auto... Values>
struct sentinel;
```

Type wrapper to specify the unused values for `opt::option`.

It tries to mimic the underlying type (`T`) with convertion operators, constructors, assignment operators
and non-static data member `m` to explicitly access reference type and to access data members/member functions.

This type provides `opt::option_traits` that uses these values to specify level value.
The `max_level` of it's `opt::option_traits` is equal to `sizeof...(Values)`.

The option traits uses regular comparison operator (`==`) and assignment operator (`=`) to get and set level value.

**Example:**
```cpp
// if func1() returns -1, it returns an empty option; otherwise, just a regular integer.
opt::option<opt::sentinel<int, -1>> func1();

// if func2() returns -1, it returns an option with empty option inside,
// if func2() returns -2, it returns an empty option;
// otherwise, just a regular integer.
opt::option<opt::option<opt::sentinel<int, -1, -2>>> func2();
```

### `opt::sentinel_f`

```cpp
template<class T, class Compare, class Set, auto... Values>
struct sentinel_f;
```

Type wrapper to specify the unused values for `opt::option` using function object `Compare` to get and `Set` to set level values.

It tries to mimic the underlying type (`T`) with convertion operators, constructors, assignment operators
and non-static data member `m` to explicitly access reference type and to access data members/member functions.

This type provides `opt::option_traits` that uses these values to specify level value.
The `max_level` of it's `opt::option_traits` is equal to `sizeof...(Values)`.

The `Compare` function object:
- First argument, a `const` reference type to `T`.
- Second argument, one value from the `Values`.
- Returns `bool`.

The `Set` function object:
- First argument, a non-`const` reference type to `T`.
- Second argument, one value from the `Values`.

### `opt::member`

```cpp
template<class T, auto MemberPtr>
struct member;
```

Type wrapper to explicitly select the data member to use for `opt::option_traits` of the contained value for `opt::option`.

It tries to mimic the underlying type (`T`) with convertion operators, constructors, assignment operators
and non-static data member `m` to explicitly access reference type and to access data members/member functions.

`MemberPtr` must be a pointer to the data member.

This type provides `opt::option_traits` that clones the `opt::option_traits` of the specified `MemberPtr` pointer to data member value.

**Example:**
```cpp
struct type {
    float x;
    int y;

    // user-declared constructor, not an aggregate type
    type(float x, int y) : x{x}, y{y} {}
};

// uses type::x data member to store the level value.
opt::option<opt::member<type, &type::x>> a;
```

### `opt::enforce`

```cpp
template<class T>
struct enforce;
```

Type wrapper to ensure that the type `T` has avaliable `opt::option_traits` for `opt::option`.

It tries to mimic the underlying type (`T`) with convertion operators, constructors, assignment operators
and non-static data member `m` to explicitly access reference type and to access data members/member functions.

If type `T` does not have a `opt::option_traits`, invokes `static_assert`.

This type provides `opt::option_traits` that clones the `opt::option_traits` of the type `T`.

**Example:**
```cpp
opt::option<opt::enforce<float>> a; // ok
opt::option<opt::enforce<int>> b; // error
opt::option<opt::enforce<std::pair<int, long>>> c; // error
opt::option<opt::enforce<std::pair<int, float>>> d; // ok
```

## Deduction guides

```cpp
template<class T>
option(T) -> option<T>;
```

**Example:**
```cpp
opt::option a{1};
static_assert(std::is_same_v<decltype(a), opt::option<int>>);

opt::option b{2.f};
static_assert(std::is_same_v<decltype(b), opt::option<float>>);

auto c = opt::option{opt::option{3.}};
static_assert(std::is_same_v<decltype(c), opt::option<double>>);
```

## Exposition-only

### `remove_cvref<X>`

`remove_cvref<X>` is a metafunction, that removes cv-qualifiers from type `X` (e.g. `remove_cvref<const int&>` is `int`).

### `can_bind_reference<X, Y>`

`can_bind_reference<X, Y>` is a metafunction, it is defined as:
```
std::is_same_v<std::remove_reference_t<Y>, std::reference_wrapper<std::remove_const_t<std::remove_reference_t<X>>>>
|| std::is_same_v<std::remove_reference_t<Y>, std::reference_wrapper<std::remove_reference_t<X>>>
|| (std::is_convertible_v<std::remove_reference_t<Y>*, std::remove_reference_t<X>*>
 && std::is_lvalue_reference_v<X> ? std::is_lvalue_reference_v<Y> : !std::is_lvalue_reference_v<Y>)
```

### `is_initializable_from<X, Y...>`

`is_initializable_from<X, Y...>` is a metafunction, that checks if a type `X` can be *direct-list-initialized* if `X` is an aggregate type with the arguments of types `Y...`; otherwise, if `X` can be *list-initialized* with the arguments of types `Y...`.

### `is_constructible_from_option<X, Y>`

`is_constructible_from_option<X, Y>` is a metafunction, that checks if type `X` is constructible or convertible from any expression of type `opt::option<Y>` (possibly `const`), i.e., in the following there is at least one `true`:
- `std::is_constructible_v<X, opt::option<Y>&>`.
- `std::is_constructible_v<X, const opt::option<Y>&>`.
- `std::is_constructible_v<X, opt::option<Y>&&>`.
- `std::is_constructible_v<X, const opt::option<Y>&&>`.
- `std::is_convertible_v<opt::option<Y>&, X>`.
- `std::is_convertible_v<const opt::option<Y>&, X>`.
- `std::is_convertible_v<opt::option<Y>&&, X>`.
- `std::is_convertible_v<const opt::option<Y>&&, X>`.

### `is_assignable_from_option<X, Y>`

`is_assignable_from_option<X, Y>` is a metafunction, that checks if type `X` is assignable from any expression of type `opt::option<Y>` (possibly `const`), i.e., in the following conditions there is at least one `true`:
- `std::is_assignable_v<X&, opt::option<Y>&>`.
- `std::is_assignable_v<X&, const opt::option<Y>&>`.
- `std::is_assignable_v<X&, opt::option<Y>&&>`.
- `std::is_assignable_v<X&, const opt::option<Y>&&>`.

### `/*lifetimebound*/`

Attribute `[[msvc::lifetimebound]]` (see [C26815][msvc-C26815] and [C26816][msvc-C26816]) or [`[[clang::lifetimebound]]`][clang-lifetimebound] if one is available.

[UB]: https://en.cppreference.com/w/cpp/language/ub
[option-verify]: ./macros.md#option_verify
[pfr is_reflectable]: https://www.boost.org/doc/libs/1_86_0/doc/html/doxygen/reference_section_of_pfr/structboost_1_1pfr_1_1is__reflectable.html
[std::is_array]: https://en.cppreference.com/w/cpp/types/is_array
[std::is_destructible]: https://en.cppreference.com/w/cpp/types/is_destructible
[std::is_void]: https://en.cppreference.com/w/cpp/types/is_void
[std::is_function]: https://en.cppreference.com/w/cpp/types/is_function
[P3168]: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3168r2
[P2218]: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2218r0
[clang-lifetimebound]: https://clang.llvm.org/docs/AttributeReference.html#lifetimebound
[msvc-C26815]: https://learn.microsoft.com/cpp/code-quality/c26815
[msvc-C26816]: https://learn.microsoft.com/cpp/code-quality/c26816
[trivially-copyable]: https://en.cppreference.com/w/cpp/named_req/TriviallyCopyable
