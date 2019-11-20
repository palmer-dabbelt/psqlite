/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#include "result.h++"
#include <stdlib.h>
using namespace psqlite;

result::result(void)
    : _return_set(false)
{
}

error_code result::return_value(void) const
{
    if (_return_set == false) {
        fprintf(stderr, "return_value() called before return_set()\n");
        abort();
    }

    return _return_value;
}

void result::set_error(int code, std::string str)
{
    if (_return_set == true) {
        fprintf(stderr, "set_error() called twice!\n");
        abort();
    }

    switch (code) {
    case 0:
        _return_value = error_code::SUCCESS;
        break;

    case 19:
        _return_value = error_code::FAILED_UNIQUE;
        break;

    default:
        fprintf(stderr, "Unknown SQLite error code %d\n", code);
        fprintf(stderr, "  Associated string: '%s'\n", str.c_str());
        abort();
        break;
    }

    _return_string = str;
    _return_set = true;
}

void result::add_map(const std::map<std::string, std::string>& m)
{
    if (_return_set == true) {
        fprintf(stderr, "add_map() called after set_return()\n");
        abort();
    }

    _data.push_back(std::make_shared<psqlite::row>(m));
}
