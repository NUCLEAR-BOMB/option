
# Customizable option traits

The `opt::option_traits` type allows to remove size overhead on `opt::option` by 
storing "is empty" state directly in the only avaliable underlying value.

See [`opt::option_traits`][option-traits] for more detailed info.

- [Creaing new specialization](#creaing-new-specialization)
- [Optional methods](#optional-methods)
- [Optional static members](#optional-static-members)
- [SFINAE](#sfinae)
- [Disabling specializations](#disabling-specializations)
- [Accessing empty state](#accessing-empty-state)

## Creaing new specialization

To implement custom option traits you need to specialize the `opt::option_traits` type.
```cpp
struct my_type { int a{}; int b{}; };

template<>
struct opt::option_traits<my_type> {
    ...
};
```
Next, you need to implement the `is_empty` and `set_empty` static methods inside created `opt::option_traits` specialization.

> The `is_empty` static method is called to check if `opt::option` contains a value.

> The `set_empty` static method is called to set the contained value to an empty state, e.g. in default constructor, when contained value is destroyed.

`is_empty` and `set_empty` signatures:
```cpp
static *constexpr bool is_empty(const T&) *noexcept;
static *constexpr void set_empty(T&) *noexcept;
```
> \* - optional specifiers \
`T` - the type that corresponds to the specialization `opt::option_traits<T>` (could be a template parameter).

`opt::option_traits` specialization with methods.
```cpp
template<>
struct opt::option_traits<my_type> {
    static bool is_empty(const my_type& value) {
        return value.b == 100;
    }
    static void set_empty(my_type& value) {
        value.b = 100;
    }
};

static_assert(sizeof(opt::option<my_type>) == sizeof(my_type));
```

Before the `opt::option_traits` specialization (assume `sizeof(int) == 4`) size of `opt::option<my_type>` was 8 bytes[^1].

After the `opt::option_traits` specialization the `opt::option` stores the "is empty" flag directly in the underlying object, so the size of `opt::option<my_type>` becomes same as the `my_type` size.
> [!WARNING]
> After contained value is constructed, `is_empty` method must 
return `false` (the [`OPTION_VERIFY`][option-verify] macro checks).
So constructors of the contained value must additionally prepare value.

## Optional methods

If you for some reason allocating or changing global state inside `set_empty` method, you can use the `unset_empty` static method to deallocate/revert global state.
```cpp
static *constexpr void unset_empty(T&) *noexcept;
```
> [!NOTE]
> This method is called before construction/complete destruction of contained value.
> You can imagine that is this a destructor of the "is empty" flag (while `set_empty` being a constructor).

```cpp
struct my_type { int a; int* b = nullptr; };

template<>
struct opt::option_traits<my_type> {
    static bool is_empty(const my_type& value) {
        return value.b != nullptr;
    }
    static void set_empty(my_type& value) {
        value.b = new int{};
    }
    static void unset_empty(my_type& value) {
        delete value.b;
    }
};

static_assert(sizeof(opt::option<my_type>) == sizeof(my_type));
```

## Optional static members

You can define the `empty_value` static constexpr member variable inside `opt::option_traits` to help the Visual Studio debugger to show if there is avaliable contained value.

The debugger understands that by bitwise comparing contained value and `empty_value` value to check if they equal, and if so, it displays that `opt::option` does not contain a value.

```cpp
static constexpr *type empty_value = ...;
```
> \*type - any type with the same size as `T`.

```cpp
struct my_type { int* a; };

template<>
struct opt::option_traits<my_type> {
    static constexpr uintptr_t empty_value = 0x12345;

    static bool is_empty(const my_type& value) {
        return reinterpret_cast<uintptr_t>(value.a) == empty_value;
    }
    static void set_empty(my_type& value) {
        value.a = reinterpret_cast<int*>(empty_value);
    }
};

static_assert(sizeof(opt::option<my_type>) == sizeof(my_type));
```

## SFINAE

The second template parameter in `opt::option_traits` allows to perform [SFINAE][sfinae] on any type.

This template parameter defaults to `void` type, so you don't need to specify type for [`std::enable_if`][enable-if].

```cpp
template<class T>
struct my_type { T a{}; };

template<class T>
struct opt::option_traits<my_type<T>, std::enable_if_t<std::is_same_v<T, int>>> {
    static bool is_empty(const my_type<T>& value) {
        return value.a == -1;
    }
    static void set_empty(my_type<T>& value) {
        value.a = -1;
    }
};

static_assert(sizeof(opt::option<my_type<int>>) == sizeof(my_type<int>));
static_assert(sizeof(opt::option<my_type<float>>) == 8); // assume sizeof(float) == 4
```

## Disabling specializations

You can disable the default specializations of `opt::option_traits` by declaring but not defining it.

This can be useful when on specific type size optimization breaks `opt::option`.

```cpp
struct my_type { int x; };

template<>
struct opt::option_traits<my_type>;

static_assert(sizeof(opt::option<my_type>) == 8); // assume sizeof(int) == 4

template<>
struct opt::option_traits<float>;

static_assert(sizeof(opt::option<float>) == 8); // assume sizeof(float) == 4
```

## Accessing empty state

`opt::option` allows getting reference to the contained value when it's in an empty state by calling `get_unchecked` method.

This method, which differs from `get`, `value`, `operator*`, etc., does not check (no [`OPTION_VERIFY`][option-verify]) wherever option contained a value.

> [!IMPORTANT]
> Using this method don't give you any performance in release. It is same as `get` or `operator*`, just doesn't checks existence of the contained value with [`OPTION_VERIFY`][option-verify].

```cpp
struct my_type { int a{}; int b{}; };

template<>
struct opt::option_traits<my_type> {
    static bool is_empty(const my_type& value) {
        return value.a > 100 && value.b > 100;
    }
    static void set_empty(my_type& value) {
        value.a = 200;
        value.b = 200;
    }
};

int main() {
    opt::option<my_type> x = my_type{};
    x->a = 5;
    x.reset();
    x.get_unchecked().a = 101;
}
```

[^1]: The `opt::option` only adds `bool` member, but because of alignment requirements it's size can't be equal to 5. See [object alignment][object-alignment].

[enable-if]: https://en.cppreference.com/w/cpp/types/enable_if
[sfinae]: https://en.cppreference.com/w/cpp/language/sfinae
[option-verify]: ./macros.md#option_verify
[option-traits]: ./reference.md#option_traits
[object-alignment]: https://en.cppreference.com/w/c/language/object#Alignment
