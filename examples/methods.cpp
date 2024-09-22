#include <iostream>
#include <opt/option.hpp>
#include <array>
#include <tuple>
#include <utility>

void has_value_and() {
    opt::option<int> a = 3;

    std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; //$ true

    a = 0;
    std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; //$ false

    a = opt::none;
    std::cout << a.has_value_and([](int x) { return x > 1; }) << '\n'; //$ false
}

void take() {
    opt::option<int> a = 1;

    std::cout << a.has_value() << '\n'; //$ true

    auto b = a.take();

    std::cout << a.has_value() << '\n'; //$ false
    std::cout << *b << '\n'; //$ 1

    a = opt::none;
    b = a.take();
    std::cout << a.has_value() << '\n'; //$ false
    std::cout << b.has_value() << '\n'; //$ false
}

void take_if() {
    opt::option<int> a = 1;

    auto b = a.take_if([](int& x) {
        x += 1;
        return x >= 3;
    });
    std::cout << *a << '\n'; //$ 2
    std::cout << b.has_value() << '\n'; //$ false

    b = a.take_if([](int& x) {
        x += 1;
        return x >= 3;
    });
    std::cout << a.has_value() << '\n'; //$ false
    std::cout << *b << '\n'; //$ 3
}

// skip insert()

void inspect() {
    opt::option<int> a = 1;

    a.inspect([](int x) { std::cout << x << '\n'; }); //$ 1

    a.map([](int x) { return x * 2; })
     .inspect([](int x) { std::cout << x << '\n'; }) //$ 2
     .map([](int x) { return float(x * 2) + 0.5f; })
     .inspect([](float x) { std::cout << x << '\n'; }) //$ 4.5
     .and_then([](float x) { return x > 5.f ? opt::option<double>{x} : opt::none; })
     .inspect([](double x) { std::cout << x << '\n'; }); // will not print `x`
}

void map_or() {
    opt::option<int> a = 2;

    std::cout << a.map_or(0, [](int x) { return x * 200; }) << '\n'; //$ 400

    a = opt::none;
    std::cout << a.map_or(0, [](int x) { return x * 2; }) << '\n'; //$ 0
}

void map_or_else() {
    opt::option<int> a = 3;

    std::cout << a.map_or_else(
        [] { std::cout << "will not print"; return 0; },
        [](int x) { return x + 1; }
    ) << '\n'; //$ 4

    a = opt::none;
    std::cout << a.map_or_else(
        [] { std::cout << "will print "; return -100; },
        [](int x) { return x - 2; }
    ) << '\n'; //$ will print -100
}

void ptr_or_null() {
    int a = 2;

    opt::option<int&> b = a;

    std::cout << (b.ptr_or_null() == &a) << '\n'; //$ true

    b = opt::none;
    std::cout << b.ptr_or_null() << '\n'; //$ [nullptr]
}

void filter() {
    const auto is_odd = [](auto x) {
        return x % 2 != 0;
    };
    opt::option<int> a = 1;

    std::cout << *a.filter(is_odd) << '\n'; //$ 1

    a = 2;
    std::cout << a.filter(is_odd).has_value() << '\n'; //$ false

    a = opt::none;
    std::cout << a.filter(is_odd).has_value() << '\n'; //$ false
}

void flatten() {
    opt::option<opt::option<int>> a = 1;

    opt::option<int> b = a.flatten();
    std::cout << *b << '\n'; //$ 1

    *a = opt::none;
    std::cout << a.flatten().has_value() << '\n'; //$ false

    a = opt::none;
    std::cout << a.flatten().has_value() << '\n'; //$ false
}

void and_then() {
    const auto do_something = [](int x) {
        return x == 0 ? opt::option<float>{1.5f} : opt::none;
    };

    opt::option<int> a = 0;

    std::cout << a.and_then(do_something).get() << '\n'; //$ 1.5

    a = 1;
    std::cout << a.and_then(do_something).has_value() << '\n'; //$ false

    a = opt::none;
    std::cout << a.and_then(do_something).has_value() << '\n'; //$ false
}

void map() {
    const auto to_float = [](int x) {
        return float(x) / 2.f;
    };

    opt::option<int> a = 1;

    std::cout << a.map(to_float).get() << '\n'; //$ 0.5

    a = opt::none;
    std::cout << a.map(to_float).has_value() << '\n'; //$ false
}

void or_else() {
    opt::option<int> a = 2;

    std::cout << a.or_else(
        [] { std::cout << "will not print"; return opt::option{3}; }
    ).get() << '\n'; //$ 2

    a = opt::none;
    std::cout << a.or_else(
        [] { std::cout << "will print "; return opt::option{10}; }
    ).get() << '\n'; //$ will print 10

    std::cout << a.or_else(
        [] { std::cout << "will print "; return opt::option<int>{}; }
    ).has_value() << '\n'; //$ will print false
}

void unzip() {
    // unzip() works for std::tuple, std::pair, std::array

    opt::option<std::tuple<int, float>> a{1, 2.5f};

    std::tuple<opt::option<int>, opt::option<float>> unzipped_a;
    unzipped_a = a.unzip();

    std::cout << std::get<0>(unzipped_a).get() << '\n'; //$ 1
    std::cout << std::get<1>(unzipped_a).get() << '\n'; //$ 2.5

    opt::option<std::array<int, 3>> b = opt::none;

    std::array<opt::option<int>, 3> unzipped_b;
    unzipped_b = b.unzip();

    std::cout << (!unzipped_b[0] && !unzipped_b[1] && !unzipped_b[2]) << '\n'; //$ true
}

void replace() {
    opt::option<int> a = 1;

    opt::option<int> b = a.replace(2);

    std::cout << *a << '\n'; //$ 2
    std::cout << *b << '\n'; //$ 1

    a = opt::none;
    b = a.replace(3);

    std::cout << *a << '\n'; //$ 3
    std::cout << b.has_value() << '\n'; //$ false
}

void begin() {
    opt::option<int> a = 1;

    std::cout << *(a.begin()) << '\n'; //$ 1

    for (int& x : a) {
        std::cout << x << '\n'; //$ 1
        x = 2;
        std::cout << x << '\n'; //$ 2
    }

    a = opt::none;
    for (const int& x : a) {
        std::cout << x << '\n';
    }

    opt::option<std::array<int, 3>> b{{1, 2, 3}};
    for (const auto& v : b) {
        for (const int& x : v) {
            std::cout << x << ' '; //$ 1 2 3
        }
        std::cout << '\n';
    }
}

void end() {
    opt::option<int> a = 1;
    std::cout << *(a.begin()) << '\n'; //$ 1
    std::cout << *(--(a.end())) << '\n'; //$ 1
    std::cout << (a.begin() == a.end()) << '\n'; //$ false
    std::cout << (a.begin() + 1 == a.end()) << '\n'; //$ true

    a = opt::none;
    std::cout << (a.begin() == a.end()) << '\n'; //$ true
}

int main() {
    std::cout << std::boolalpha;

    has_value_and();
    take();
    take_if();
    inspect();
    map_or();
    map_or_else();
    ptr_or_null();
    filter();
    flatten();
    and_then();
    map();
    or_else();
    unzip();
    replace();
    begin();
    end();
}
