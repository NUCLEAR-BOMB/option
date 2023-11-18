#include <iostream>
#include <option.hpp>

void enums() {
    enum class some_enum {
        abcd, abc, ab, a,
        OPTION_EXPLOIT_UNUSED_VALUE
    };
    opt::option<some_enum> a = some_enum::abc;

    std::cout << (sizeof(a) == sizeof(some_enum)) << '\n'; // true
}

struct some_struct {
    int val;
};

template<>
struct opt::option_flag<some_struct> {
    static bool is_empty(const some_struct& value) noexcept {
        return value.val == -1;
    }
    static void construct_empty_flag(some_struct& value) noexcept {
        value = {-1};
    }
};

void custom() {
    opt::option<some_struct> a{{5}};

    std::cout << (sizeof(a) == sizeof(some_struct)) << '\n'; // true
    std::cout << a->val << '\n'; // 5

    a = opt::none;
    std::cout << a.has_value() << '\n'; // false
    std::cout << a.get_unchecked().val << '\n'; // -1
}

int main() {
    std::cout << std::boolalpha;

    enums();
    custom();
}
