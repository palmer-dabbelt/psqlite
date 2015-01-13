
/*
 * Copyright (C) 2014 Palmer Dabbelt
 *   <palmer@dabbelt.com>
 *
 * This file is part of psqlite.
 *
 * psqlite is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * psqlite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with psqlite.  If not, see <http://www.gnu.org/licenses/>.
 */

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
