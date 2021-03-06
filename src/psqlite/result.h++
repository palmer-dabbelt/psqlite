/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#ifndef PSQLITE__RESULT_HXX
#define PSQLITE__RESULT_HXX

#include <memory>
#include "error_code.h++"
#include "row.h++"
#include <map>
#include <string>
#include <vector>

namespace psqlite {
    /* Holds the result of a SQL command. */
    class result {
    public:
        typedef std::shared_ptr<result> ptr;

    private:
        /* Here we handle the return code from SQLite.*/
        bool _return_set;
        enum error_code _return_value;
        std::string _return_string;

        /* This contains the actual return data, which is the
         * whole point of this object. */
        std::vector<row::ptr> _data;

    public:
        /* Creates a new result set that hasn't yet been
         * finalized. */
        result(void);

    public:
        /* Returns the error code (which can be "Success") that
         * cooresponds to this command.  Note that you can only
         * call this _after_ set_error()! */
        enum error_code return_value(void) const;
        const std::string &return_string(void) const { return _return_string; }

        /* This actually finalizes the construction of the result
         * and makes it usable. */
        void set_error(int code, std::string str);

        /* Adds an entry to the list of results. */
        void add_map(const std::map<std::string, std::string>& m);

        /* Returns the number of results that exist in this
         * entry. */
        size_t result_count(void) const
            { return _data.size(); }

        /* Returns a single row from the listing. */
        const row::ptr& rowi(size_t i) const
            { return _data[i]; }
        const std::vector<row::ptr>& rows(void) const
            { return _data; }
    };
}

#endif
