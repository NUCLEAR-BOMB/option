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

int main() {
    std::cout << std::boolalpha;

    custom();
}
