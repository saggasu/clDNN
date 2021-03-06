/*
// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
#pragma once

#include <map>
#include <functional>

template<typename T, typename U>
class singleton_map : public std::map<T, U> {
    singleton_map() : std::map<T, U>() {};
    singleton_map(singleton_map const&) = delete;
    void operator=(singleton_map const&) = delete;

    public:
    static singleton_map &instance() {
        static singleton_map instance_;
        return instance_;
    }
};

namespace cldnn {

struct permute;
struct reorder;
struct reshape;
struct data;
struct input_layout;
struct prior_box;

struct primitive_impl;

template <class PType>
struct typed_program_node;

template<typename primitive_kind>
struct implementation_key 
{
    typedef std::tuple<engine_types, data_types, format::type> type;
    type operator()(engine_types engine_type, const typed_program_node<primitive_kind>& primitive)
    {
        return std::make_tuple(engine_type, primitive.get_dependency(0).get_output_layout().data_type, primitive.get_dependency(0).get_output_layout().format);
    }
};

template<>
struct implementation_key<permute>
{
    typedef cldnn::engine_types type;
    type operator()(engine_types engine_type, const typed_program_node<permute>&)
    {
        return engine_type;
    }
};

template<>
struct implementation_key<reorder>
{
    typedef cldnn::engine_types type;
    type operator()(engine_types engine_type, const typed_program_node<reorder>&)
    {
        return engine_type;
    }
};

template<>
struct implementation_key<reshape>
{
    typedef cldnn::engine_types type;
    type operator()(engine_types engine_type, const typed_program_node<reshape>&)
    {
        return engine_type;
    }
};

template<>
struct implementation_key<data>
{
    typedef cldnn::engine_types type;
    type operator()(engine_types engine_type, const typed_program_node<data>&)
    {
        return engine_type;
    }
};

template<>
struct implementation_key<input_layout>
{
    typedef cldnn::engine_types type;
    type operator()(engine_types engine_type, const typed_program_node<input_layout>&)
    {
        return engine_type;
    }
};

template<>
struct implementation_key<prior_box>
{
    typedef cldnn::engine_types type;
    type operator()(engine_types engine_type, const typed_program_node<prior_box>&)
    {
        return engine_type;
    }
};

template<typename primitive_kind>
class implementation_map {
public:
    using key_builder = implementation_key<primitive_kind>;
    using key_type = typename key_builder::type;
    using factory_type = std::function<primitive_impl*(const typed_program_node<primitive_kind>&)>;
    using map_type = singleton_map<key_type, factory_type>;

    static factory_type get(engine_types engine_type, const typed_program_node<primitive_kind>& primitive) {
        // lookup in database; throw if not found 
        auto key = key_builder()(engine_type, primitive);
        auto it = map_type::instance().find(key);
        if (it == std::end(map_type::instance())) 
            throw std::runtime_error("not yet implemented");

        // create implementation & attach it to result 
        return it->second;
    }

    static void add(typename map_type::key_type key, factory_type factory) {
        map_type::instance().insert({ key, factory });
    }

    static void add(std::initializer_list<typename map_type::value_type> il) {
        map_type::instance().insert(il);
    }
};
}
