/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#include "row.h++"
#include <stdlib.h>
using namespace psqlite;

template<class K, class V>
static std::vector<K> map_keys(const std::map<K, V>& m);

row::row(const std::map<std::string, std::string>& m)
    : _columns(map_keys(m)),
      _m(m)
{
}

std::string row::get_str(const std::string& col)
{
    auto l = _m.find(col);
    if (l == _m.end()) {
        fprintf(stderr, "Unable to find column '%s'\n", col.c_str());
        abort();
    }

    return l->second;
}

unsigned row::get_uint(const std::string& col)
{
    auto str = get_str(col);
    auto uint = atoi(str.c_str());
    if (uint < 0) {
        fprintf(stderr, "Unable to parse '%s' as unsigned\n", str.c_str());
        abort();
    }
    return uint;
}

bool row::get_bool(const std::string& col)
{
    auto uint = get_uint(col);
    switch (uint) {
    case 0:
        return false;

    case 1:
        return true;

    default:
        fprintf(stderr, "Unable to parse %d as bool\n", uint);
        abort();
    }
}

template<class K, class V>
std::vector<K> map_keys(const std::map<K, V>& m)
{
    std::vector<K> out;
    for (const auto& pair: m)
        out.push_back(pair.first);
    return out;
}
