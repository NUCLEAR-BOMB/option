
# Custom option traits guide

- [Custom option traits guide](#custom-option-traits-guide)
  - [How `opt::option_traits` affects `opt::option`](#how-optoption_traits-affects-optoption)
  - [Creating new `opt::option_traits` specialization](#creating-new-optoption_traits-specialization)
  - [Optional static methods](#optional-static-methods)
  - [SFINAE](#sfinae)
  - [Disabling specializations](#disabling-specializations)
  - [Accessing empty state](#accessing-empty-state)
  - [Overriding builtin option traits](#overriding-builtin-option-traits)

## How `opt::option_traits` affects `opt::option`

The main purpose of `opt::option_traits` is to redude size of the `opt::option` to absolute minimum.

`opt::option_traits` achieves it by defining static methods to manipulate underlying value without using external "has value" flag, which saves up space for `bool`.
This could drastically affect the size of `opt::option` because of storage alignment issues.

## Creating new `opt::option_traits` specialization

To define new option trait specialization you need to specialize the `opt::option_traits` type.

> [!NOTE]
> You must specialize `opt::option_traits` either in global namespace or in `opt` namespace.

```cpp
struct my_type {
    int x;
    int y;
};

template<>
struct opt::option_traits<my_type> {
    ...
};
```

`opt::option_traits` requires static constexpr `max_level` variable, `set_level` and `get_level` static methods to be implemented inside specialized `opt::option_traits`.

- `max_level`: Maximum avaliable level depth inside underlying value. 0 means that option trait is disabled and can't be used for `opt::option`.
- `get_level`: function that returns emptiness level of underlying value which is received by it's first parameter.
If there is actually a contained value, `std::uintmax_t(-1)` is returned.
- `set_level`: function that sets level of underlying value which is received by it's first parameter, and level value by second parameter.
Passed level is less than `max_level`.

> [!NOTE]
> Level means the depth of emptiness of nested `opt::option`s. \
> If level depth is 0 in `opt::option<opt::option<int>>`, the nested `opt::option<int>` is considered empty (can't access the `int` value), but outer `opt::option<opt::option<int>>` is not empty (can still access `opt::option<int>` ). \
> If level depth is 1, entire `opt::option<opt::option<int>>` is considered empty (can't access the `opt::option<int>` value and accordingly the `int` value). \
> If level depth is `std::uintmax_t(-1)` (last nested `opt::option` containes an underlying value), the `int` value is avaliable with every `opt::option` level correspondingly.

> [!IMPORTANT]
> The `noexcept` specifier is required. It is made to avoid similar to [`std::variant<Types...>::valueless_by_exception`][std::variant valueless_by_exception] state.

Correct `opt::option_traits` specialization.
```cpp
template<>
struct opt::option_traits<my_type> {
    static constexpr std::uintmax_t max_level = 100;

    static std::uintmax_t get_level(const my_type* value) noexcept {
        return value->x == -1 ? value->y : std::uintmax_t(-1);
    }
    static void set_level(my_type* value, std::uintmax_t level) noexcept {
        value->x = -1;
        value->y = level;
    }
};
```

When `my_type::x` is equal to -1, the `opt::option<my_type>` is considered empty (`.has_value()` returns `false`), and `my_type::y` identifies a level of emptiness depth of nested `opt::option`s.

```cpp
opt::option<my_type> a{1, 2};
// a.has_value() == true
// my_type::x == 1
// my_type::y == 2

a.reset();
// a.has_value() == false
// my_type::x == -1 (is empty)
// my_type::y == 0 (level)
```

```cpp
opt::option<opt::option<my_type>> b{10, 20};
// b.has_value() == true
// b.get().has_value() == true
// my_type::x == 10
// my_type::y == 20

b.get().reset();
// b.has_value() == true
// b.get().has_value() == false
// my_type::x == -1 (is empty)
// my_type::y == 0 (level)

b.reset()
// b.has_value() == false
// Can't access b.get() because there is no that object.
// my_type::x == -1 (is empty)
// my_type::y == 1 (level)
```

```cpp
opt::option<opt::option<opt::option<my_type>>> c{2, 3};
// c.has_value() == true
// c.get().has_value() == true
// c.get().get().has_value() == true
// my_type::x == 2
// my_type::y == 3

c.get().get().reset();
// c.has_value() == true
// c.get().has_value() == true
// c.get().get().has_value() == false
// my_type::x == -1 (is empty)
// my_type::y == 0 (level)

c.get().reset();
// c.has_value() == true
// c.get().has_value() == false
// Can't access c.get().get() because there is no that object.
// my_type::x == -1 (is empty)
// my_type::y == 1 (level)

c.reset();
// c.has_value() == false
// Can't access c.get() because there is no that object.
// Can't access c.get().get() because there is c.get() object.
// my_type::x == -1 (is empty)
// my_type::y == 2 (level)
```

Now the size of `opt::option<my_type>` (and nested ones) is same as size of `my_type` type. Success!

But what would happen if you modify `my_type::x` to be -1?
```cpp
opt::option<opt::option<my_type>> a{1, 1};
a->x = -1; // !!!
// Now a.has_value() == false!
// my_type::x == -1 (is empty)
// my_type::y == 1 (level)

a.get_unchecked().y = 0; // Modify unexisting value
// a.has_value() == true
// a.get().has_value() == false
// my_type::x == -1 (is empty)
// my_type::y == 0 (level)
```
Yes, you can do that, but beware that it could trigger `OPTION_VERIFY` if you do this in some places (`.emplace`, constructors, etc.).

## Optional static methods

`opt::option_traits` also supports 2 optional static methods: `after_constructor`, `after_assignment`.

- `after_constructor`: called after object is constructed. Used to prepare object for later use for `opt::option`.
- `after_assignment`: called after object is assigned. Used to prepare object for later use for `otp::option`.

> [!NOTE]
> Usually these static methods are used to force the compiler to complete object initialization for later use in option.

> [!IMPORTANT]
> These static methods are not called if `opt::option` uses direct copy/move assignment operators/constructors. It allows `opt::option` to have trivially copy/move assignment operators/constructors.

```cpp
struct my_type_2 {
    int x;
};

template<>
struct opt::option_traits<my_type_2> {
    static constexpr std::uintmax_t max_level = 10;

    static std::uintmax_t get_level(const my_type_2* value) noexcept {
        return value->x != 1 ? value->x - 2 : std::uintmax_t(-1);
    }
    static void set_level(my_type_2* value, std::uintmax_t level) noexcept {
        value->x = level + 2;
    }
    static void after_constructor(my_type_2* value) noexcept {
        value->x = 1;
    }
    static void after_assignment(my_type_2* value) noexcept {
        value->x = 1;
    }
};
```
Only when `my_type_2::x` is equal 1, `opt::option<my_type_2>` is not empty, otherwise, it is empty.
Level value is denoted by `my_type_2::x - 2`.

```cpp
opt::option<my_type_2> a;
// a.has_value() == false
// a.my_type_2::x == 2

a = my_type_2{5};
// Call perfect-forwarded assignment of `opt::option`, invoke `after_assignment`
// a.has_value() == true
// a.my_type_2::x == 1

opt::option<my_type_2> b = my_type_2{123};
// b.has_value() == true
// b.my_type_2::x == 1

b->x = 10;
// b.has_value() == false
// b.my_type_2::x == 10

a = b;
// Trivially copy, no `after_assignment` invocation
// a.has_value() == false
// a.my_type_2::x == 10

a.emplace(6);
// Call `emplace` method of `opt::option`, invoke `after_constructor`
// a.has_value() == true
// a.my_type_2::x == 1

a->x = 20;
// a.has_value() == false
// a.my_type_2::x == 20

opt::option<my_type_2> c{a};
// Trivially construct, no `after_constructor` invocation
// c.has_value() == false
// c.my_type_2::x == 20
```

## SFINAE

You can use [SFINAE][sfinae] with second template parameter `opt::option_traits`.
It is defaulted to have `void` type and it is recommended that you explicitly don't provide custom type.

```cpp
template<class T>
struct my_type_3 {
    T x;
};

template<class T>
struct opt::option_traits<my_type_3<T>, std::enable_if_t<!std::is_same_v<T, float>>> {
    static constexpr std::uintmax_t max_level = 1;

    static std::uintmax_t get_level(const my_type_3<T>* value) noexcept {
        return value->x == -1 ? 0 : std::uintmax_t(-1);
    }
    static void set_level(my_type_3<T>* value, [[maybe_unused]] std::uintmax_t level) noexcept {
        value->x = -1;
    }
};
```

Enables `opt::option_traits` only when `!std::is_same_v<T, float>` is `true` for `my_type_3<T>.`.
When that condition is not met, `opt::option` don't uses `opt::option_traits`.

```cpp
opt::option<my_type_3<int>> a;
// sizeof(opt::option<my_type_3<int>>) == sizeof(my_type_3<int>);
// a.has_value() == false
// a.my_type_3::x == -1

a = my_type_3<int>{5};
// a.has_value() == true
// a.my_type_3::x == 5
```

```cpp
opt::option<my_type_3<float>> b = my_type_3<float>{5.f};
// sizeof(opt::option<my_type_3<float>>) > sizeof(my_type_3<float>)
// b.has_value() == true
// b.my_type_3::x == 5.f
// b.{has value flag} == true

b.reset()
// b.has_value() == false
// b.my_type_3::x == {unspecified}
// b.{has value flag} == false
```

## Disabling specializations

If you have problems with `opt::option` size optimizations, you can always disable specific specializations of `opt::option_traits`.

There is two ways of doing that:
1. Declare but not to define `opt::option_traits` specialization.
2. Set the static constexpr `max_value` variable to 0.

```cpp
template<>
struct opt::option_traits<float>;

template<>
struct opt::option_traits<double> {
    static constexpr std::uintmax_t max_level = 0;
}
```

Disable size optimization for `float` (`opt::option<float>`) and `double` (`opt::option<double>`).

After this specialization the size of `opt::option<float>`/`opt::option<double>` will be greater than `float`/`double` (`opt::option` will store "has value" flag).

> [!TIP]
> This could be useful if `opt::option` behaves incorrectly (compiler optimizations, platform specific stuff).

## Accessing empty state

`opt::option` has `get_unchecked` method to access an underlying value when it is considered empty.

This method doesn't check `opt::option` on emptiness (doesn't use `OPTION_VERIFY`) in contrast of `get`, `value`, `operator*` methods/operator.

```cpp
struct my_type_4 {
    int x{};
    int y{};
    int z{};
};

template<>
struct opt::option_traits<my_type_4> {
    static constexpr std::uintmax_t max_level = 10000;

    static std::uintmax_t get_level(const my_type_4* value) noexcept {
        return value->x == -1 ? value->y : std::uintmax_t(-1);
    }
    static void set_level(my_type_4* value, std::uintmax_t level) noexcept {
        value->x == -1;
        value->y = level;
    }
};
```
When `my_type_4::x` is -1, level value is `my_type_4::y`.
Doesn't affect `my_type_4::z`.

> [!WARNING]
> When accessing the value in an empty state, contents that are not initialized inside `set_level` have unspecified value.

```cpp
opt::option<my_type_4> a;
// a.has_value() == false
// a.my_type_4::x == -1
// a.my_type_4::y == 0
// a.my_type_4::z == {unspecified}

a.get_unchecked().z = 5;
// a.has_value() == false
// a.my_type_4::x == -1
// a.my_type_4::y == 0
// a.my_type_4::z == 5

a.get_unchecked().y = 10;
// a.has_value() == false
// a.my_type_4::x == -1
// a.my_type_4::y == 10 (modify level value)
// a.my_type_4::z == 5

a.get_unchecked().x = 11;
// a.has_value() == true (my_type_4::x != -1, so option is not empty)
// a.my_type_4::x == 5
// a.my_type_4::y == 10
// a.my_type_4::z == 11
```

> [!IMPORTANT]
> Some actions involving `get_unchecked` could lead to failing the check of `OPTION_VERIFY`.

## Overriding builtin option traits

To override the builtin option trait you just need to define specific specialization that you interested to.

```cpp
template<>
struct opt::option_traits<float> {
    static constexpr std::uintmax_t max_level = 1;

    static std::uintmax_t get_level(const float* value) noexcept {
        return *value == 0 ? 0 : std::uintmax_t(-1);
    }
    static void set_level(float* value, [[maybe_unused]] std::uintmax_t level) noexcept {
        *value = 0;
    }
};
```

Override builtin `float` option trait.
When the value is 0, option is considered empty.

```cpp
opt::option<float> a;
// a.has_value() == false
// a.{value} == 0

a = 1.f;
// a.has_value() == true
// a.{value} == 1.f

*a = 0.f;
// a.has_value() == false
// a.{value} == 0.f
```

> [!TIP]
> This could be useful if `opt::option` behaves incorrectly (compiler optimizations, platform specific stuff).




[sfinae]: https://en.cppreference.com/w/cpp/language/sfinae
[std::variant valueless_by_exception]: https://en.cppreference.com/w/cpp/utility/variant/valueless_by_exception
