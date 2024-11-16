#pragma once

// Copyright 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

namespace opt {

template<class T>
class option;

template<class T, class = void>
struct option_traits;

template<class T, class = void>
struct sentinel_option_traits;

template<class>
struct is_option { static constexpr bool value = false; };
template<class T>
struct is_option<option<T>> { static constexpr bool value = true; };

template<class>
inline constexpr bool is_option_v = false;
template<class T>
inline constexpr bool is_option_v<option<T>> = true;

struct option_tag {};

namespace impl {
    struct none_tag {};
}
struct none_t {
    constexpr explicit none_t(impl::none_tag) {}
};
inline constexpr none_t none{impl::none_tag{}};

}
