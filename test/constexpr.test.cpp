/*
#define OPTION_FORCE_CONSTEXPR

#include "utils.hpp"

#define D [[maybe_unused]] constexpr

namespace {

[[maybe_unused]]
void basic() {
    D opt::option<bool> ob1{};
    D opt::option<bool> ob2{true};

    D opt::option<opt::option<bool>> oob1{};
    D opt::option<opt::option<bool>> oob2{true};

    D static int x = 1;
    D opt::option<const int*> op1{};
    D opt::option<const int*> op2{&x};
}

}
*/
