// This file is part of the mcl project.
// Copyright (c) 2022 merryhime
// SPDX-License-Identifier: MIT

#include <tuple>
#include <type_traits>

#include <mcl/type_traits/function_info.hpp>
#include <mcl/type_traits/is_instance_of_template.hpp>

using namespace mcl;

// function_info

struct Bar {
    int frob(double a) { return a; }
};

static_assert(parameter_count_v<void()> == 0);
static_assert(parameter_count_v<void(int, int, int)> == 3);
static_assert(std::is_same_v<get_parameter<void (*)(bool, int, double), 2>, double>);
static_assert(std::is_same_v<equivalent_function_type<void (*)(bool, int, double)>, void(bool, int, double)>);
static_assert(std::is_same_v<return_type<void (*)(bool, int, double)>, void>);
static_assert(std::is_same_v<equivalent_function_type<decltype(&Bar::frob)>, int(double)>);
static_assert(std::is_same_v<class_type<decltype(&Bar::frob)>, Bar>);

// is_instance_of_template

template<class, class...>
class Foo {};

template<class, class>
class Pair {};

static_assert(is_instance_of_template_v<std::tuple, std::tuple<int, bool>>);
static_assert(!is_instance_of_template_v<std::tuple, bool>);
static_assert(is_instance_of_template_v<Foo, Foo<bool>>);
static_assert(is_instance_of_template_v<Pair, Pair<bool, int>>);
static_assert(!is_instance_of_template_v<Pair, Foo<bool, int>>);
