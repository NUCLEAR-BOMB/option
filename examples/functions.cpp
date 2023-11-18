#include <iostream>
#include <option.hpp>

void zip() {
    opt::option<int> a{1};
    opt::option<float> b{2.f};
    opt::option<double> c{3.};

    opt::option<std::tuple<int, float, double>> abc;
    abc = opt::zip(a, b, c);
    std::cout << abc.has_value() << '\n'; // true

    a = opt::none;
    abc = opt::zip(a, b, c);
    std::cout << abc.has_value() << '\n'; // false
}

void zip_with() {
    opt::option<int> a{10};
    opt::option<float> b{5.f};

    const auto add_and_print = [](int x, float y) {
        std::cout << (x + y) << '\n';
    };

    opt::zip_with(add_and_print, a, b); // 15

    a = opt::none;
    opt::zip_with(add_and_print, a, b); // will not call `add_and_print`
}

void option_cast() {
    opt::option<float> a{2.5f};
    opt::option<int> b;

    b = opt::option_cast<int>(a);
    std::cout << *b << '\n'; // 2

    a = opt::none;
    b = opt::option_cast<int>(a);
    std::cout << b.has_value() << '\n'; // false
}

void hash() {
    opt::option<int> a{12345};

    std::cout << std::hash<opt::option<int>>{}(a) << '\n'; // [some hash]

    a = opt::none;
    std::cout << std::hash<opt::option<int>>{}(a) << '\n'; // [some empty option hash]
}

int main() {
    std::cout << std::boolalpha;
    zip();
    zip_with();
    option_cast();
    hash();
}
