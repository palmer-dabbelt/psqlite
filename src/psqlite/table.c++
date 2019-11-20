/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#include "table.h++"
using namespace psqlite;

table::table(const std::string& name,
             const std::vector<column::ptr>& cols)
    : _name(name),
      _cols(cols)
{
}
