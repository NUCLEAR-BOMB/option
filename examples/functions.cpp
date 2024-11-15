#include <iostream>
#include <opt/option.hpp>
#include <variant>
#include <tuple>
#include <vector>
#include <functional>
#include <array>

void zip() {
    opt::option<int> a{1};
    opt::option<float> b{2.f};
    opt::option<double> c{3.};

    opt::option<std::tuple<int, float, double>> abc;
    abc = opt::zip(a, b, c);
    std::cout << abc.has_value() << '\n'; //$ true

    a = opt::none;
    abc = opt::zip(a, b, c);
    std::cout << abc.has_value() << '\n'; //$ false
}

void zip_with() {
    opt::option<int> a{10};
    opt::option<float> b{5.f};

    const auto add_and_print = [](int x, float y) {
        std::cout << (x + y) << '\n';
    };

    opt::zip_with(add_and_print, a, b); //$ 15

    a = opt::none;
    opt::zip_with(add_and_print, a, b); // will not call `add_and_print`
}

void option_cast() {
    opt::option<float> a{2.5f};
    opt::option<int> b;

    b = opt::option_cast<int>(a);
    std::cout << *b << '\n'; //$ 2

    a = opt::none;
    b = opt::option_cast<int>(a);
    std::cout << b.has_value() << '\n'; //$ false
}

void get() {
    opt::option<std::tuple<int, float>> a{1, 2.f};

    if (auto b = opt::get<0>(a)) {
        std::cout << *b << '\n'; //$ 1
        *b = 2;
    }
    std::cout << *opt::get<int>(a) << '\n'; //$ 2

    opt::option<std::variant<int, float>> c{123};

    if (auto d = opt::get<int>(c)) {
        std::cout << *d << '\n'; //$ 123
    }
    c = 2.f;
    if (auto d = opt::get<float>(c)) {
        std::cout << *d << '\n'; //$ 2
    }
}

void hash() {
    opt::option<int> a{12345};

    std::cout << std::hash<opt::option<int>>{}(a) << '\n'; //$ [number]

    a = opt::none;
    std::cout << std::hash<opt::option<int>>{}(a) << '\n'; //$ [number]
}

void io() {
    opt::option<int> a;
    std::cout << opt::io(a) << '\n'; //$
    std::cout << opt::io(a, "empty") << '\n'; //$ empty

    a = 1;
    std::cout << opt::io(a) << '\n'; //$ 1
    std::cout << opt::io(a, "empty") << '\n'; //$ 1
}

void at() {
    std::vector<int> a{{10, 11, 12, 13, 14}};

    std::cout << (opt::at(a, 0) == 10) << '\n'; //$ true
    std::cout << (opt::at(a, 5) == 15) << '\n'; //$ false

    a = {1, 2};
    std::cout << (opt::at(a, 0) == 1) << '\n'; //$ true
    std::cout << (opt::at(a, 1) == 2) << '\n'; //$ true
    std::cout << (opt::at(a, 2) == 3) << '\n'; //$ false
}

void flatten() {
    opt::option<opt::option<int>> a = 1;

    opt::option<int> b = opt::flatten(a);
    std::cout << *b << '\n'; //$ 1

    *a = opt::none;
    std::cout << opt::flatten(a).has_value() << '\n'; //$ false
    a = opt::none;
    std::cout << opt::flatten(a).has_value() << '\n'; //$ false

    opt::option<opt::option<opt::option<float>>> c = 2.f;

    opt::option<float> d = opt::flatten(c);
    std::cout << *d << '\n'; //$ 2

    c.get().get() = opt::none;
    std::cout << opt::flatten(c).has_value() << '\n'; //$ false
    c.get() = opt::none;
    std::cout << opt::flatten(c).has_value() << '\n'; //$ false
    c = opt::none;
    std::cout << opt::flatten(c).has_value() << '\n'; //$ false
}

void unzip() {
    opt::option<std::tuple<int, float>> a{1, 2.5f};

    std::tuple<opt::option<int>, opt::option<float>> unzipped_a;
    unzipped_a = opt::unzip(a);

    std::cout << std::get<0>(unzipped_a).get() << '\n'; //$ 1
    std::cout << std::get<1>(unzipped_a).get() << '\n'; //$ 2.5

    opt::option<std::array<int, 3>> b = opt::none;

    std::array<opt::option<int>, 3> unzipped_b = opt::unzip(b);

    std::cout << (!unzipped_b[0] && !unzipped_b[1] && !unzipped_b[2]) << '\n'; //$ true
}

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

void as_option() {
    opt::option<my_type> a = opt::as_option(my_type{1});
    std::cout << opt::io(a.map(&my_type::x), "empty") << '\n'; //$ 1

    a = opt::as_option(my_type{100});
    std::cout << opt::io(a.map(&my_type::x), "empty") << '\n'; //$ empty
}

int main() {
    std::cout << std::boolalpha;
    zip();
    zip_with();
    option_cast();
    get();
    hash();
    io();
    at();
    flatten();
    unzip();
    as_option();
}
