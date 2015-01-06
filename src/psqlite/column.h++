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

#ifndef PSQLITE__COLUMN_HXX
#define PSQLITE__COLUMN_HXX

#include <memory>
#include <string>

namespace psqlite {
    /* Represents a single SQLite column.  */
    class column {
    public:
        typedef std::shared_ptr<column> ptr;
            
    private:
        const std::string _name;

    public:
        column(const std::string& name)
            : _name(name)
            {
            }

    public:
        const std::string& name(void) const
            { return _name; }
    };

    /* Represents a singel SQLite column type, which is actually
     * capable of casting things around. */
    template<class T> class column_t: public column {
    public:
        column_t(const std::string& name)
            : column(name)
            {
            }
        
    public:
    };
}

#endif
