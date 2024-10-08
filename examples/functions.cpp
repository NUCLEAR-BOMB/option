#include <iostream>
#include <opt/option.hpp>
#include <variant>
#include <tuple>
#include <vector>
#include <functional>

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

int main() {
    std::cout << std::boolalpha;
    zip();
    zip_with();
    option_cast();
    get();
    hash();
    io();
    at();
}
