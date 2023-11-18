#include <iostream>
#include <option.hpp>

void or_operator() {
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
}

void and_operator() {
    opt::option<int> a = 1;
    opt::option<float> b = 2.5f;

    std::cout << (a & b).get() << '\n'; // 2.5

    a = opt::none;
    std::cout << (a & b).has_value() << '\n'; // false

    a = 1;
    b = opt::none;
    std::cout << (a & b).has_value() << '\n'; // false
}

void xor_operator() {
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
}

int main() {
    std::cout << std::boolalpha;

    or_operator();
    and_operator();
    xor_operator();
}
