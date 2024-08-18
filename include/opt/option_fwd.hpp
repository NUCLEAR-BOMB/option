#pragma once

namespace opt {

template<class T>
class option;

template<class T, class = void>
struct option_traits;

template<class>
inline constexpr bool is_option = false;
template<class T>
inline constexpr bool is_option<option<T>> = true;

struct option_tag {};

}
