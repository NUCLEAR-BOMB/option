#include <opt/option.hpp>

int main() {
    opt::option<int> x = 0;
    return x.get();
}
