/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

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
