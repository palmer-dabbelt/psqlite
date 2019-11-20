/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#ifndef PSQLITE__TABLE_HXX
#define PSQLITE__TABLE_HXX

#include <memory>
#include "column.h++"
#include <string>
#include <vector>

namespace psqlite {
    /* This represents a SQL table, which pretty much just
     * consists of a table name and a bunch of columns in some
     * order. */
    class table {
    public:
        typedef std::shared_ptr<table> ptr;

    private:
        std::string _name;
        std::vector<column::ptr> _cols;

    public:
        /* Creates a new column from a list of tables. */
        table(const std::string& name,
              const std::vector<column::ptr>& cols);

    public:
        /* Returns the list of all columns in this table. */
        const std::vector<column::ptr>& columns(void) const
            { return _cols; }

        const std::string& name(void) const
            { return _name; }
    };
}

#endif
