
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
