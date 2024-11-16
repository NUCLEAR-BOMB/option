#include <iostream>
#include <opt/option.hpp>

struct some_struct {
    int val;
};

template<>
struct opt::option_traits<some_struct> {
    static constexpr std::uintmax_t max_level = 1;

    static std::uintmax_t get_level(const some_struct* value) noexcept {
        return value->val == -1 ? 0 : std::uintmax_t(-1);
    }
    static void set_level(some_struct* value, [[maybe_unused]] std::uintmax_t level) noexcept {
        value->val = -1;
    }
};

void custom() {
    opt::option<some_struct> a{5};

    std::cout << (sizeof(a) == sizeof(some_struct)) << '\n'; //$ true
    std::cout << a->val << '\n'; //$ 5

    a = opt::none;
    std::cout << a.has_value() << '\n'; //$ false
    std::cout << a.get_unchecked().val << '\n'; //$ -1
}

struct my_type {
    unsigned x;

    bool operator==(const my_type& other) const { return x == other.x; }
};
template<>
struct opt::sentinel_option_traits<my_type> {
    static constexpr my_type sentinel_value{0u};
};

void using_sentinel() {
    opt::option<my_type> a{5u};

    std::cout << (sizeof(a) == sizeof(my_type)) << '\n'; //$ true
    std::cout << a->x << '\n'; //$ 5

    a.reset();
    std::cout << a.get_unchecked().x << '\n'; //$ 0

    a = my_type{1u};
    std::cout << a->x << '\n'; //$ 1
    a->x = 0u;
    std::cout << a.has_value() << '\n'; //$ false
}

int main() {
    std::cout << std::boolalpha;

    custom();
    using_sentinel();
}
