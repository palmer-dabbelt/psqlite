/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#ifndef PSQLITE__ROW_HXX
#define PSQLITE__ROW_HXX

#include <memory>
#include <map>
#include <string>
#include <vector>

namespace psqlite {
    /* Holds a single row that was returned from a query. */
    class row {
    public:
        typedef std::shared_ptr<row> ptr;

    private:
        const std::vector<std::string> _columns;
        const std::map<std::string, std::string> _m;

    public:
        /* Creates a new row, given the data it contains. */
        row(const std::map<std::string, std::string>& m);

    public:
        /* These provide a mechanism for obtaining access to
         * particular fields within this row. */
        std::string get_str(const std::string& col);
        unsigned get_uint(const std::string& col);
        bool get_bool(const std::string& col);

        /* Returns the list of the columns that are known to this
         * row.  This is guarnteed */
        std::vector<std::string> columns(void) const
            { return _columns; }

        /* Returns the value associated with a column. */
        std::string value(const std::string& str) const
            { 
                auto l = _m.find(str);
                return l->second;
            }

        /* Returns TRUE if the value exists in the map, and FALSE
         * otherwise.  Note that this is FALSE for NULL SQL
         * values, as well as FALSE for values that aren't in the
         * SQL table at all! */
        bool has(const std::string& str) const
            {
                auto l = _m.find(str);
                if (l == _m.end())
                    return false;
                else
                    return true;
            }
    };
}

#endif
