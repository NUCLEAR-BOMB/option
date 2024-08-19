#pragma once

namespace opt {

template<class T>
class option;

template<class T, class = void>
struct option_traits;

template<class>
struct is_option { static constexpr bool value = false; };
template<class T>
struct is_option<option<T>> { static constexpr bool value = true; };

template<class T>
inline constexpr bool is_option_v = is_option<T>::value;

struct option_tag {};

}
