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

#ifndef PSQLITE__CONNECTION_HXX
#define PSQLITE__CONNECTION_HXX

namespace psqlite {
    class connection;
}

#include <memory>
#include "result.h++"
#include "table.h++"
#include "transaction.h++"
#include <sqlite3.h>
#include <string>
#include <vector>

namespace psqlite {
    /* Holds a single database connection.  To avoid deadlock you
     * almost certainly only want to open one of these per
     * process! */
    class connection {
    public:
        typedef std::shared_ptr<connection> ptr;

    private:
        /* The whole point of this class is to wrap a regular
         * SQLite3 database connection in a way that's ammenable
         * to C++11. */
        struct sqlite3 *_db;

        /* Holds a copy of the current transaction, which we use
         * to ensure that we only hand out a single
         * transaction. */
        std::weak_ptr<transaction> _tr;

    public:
        /* Opens a new connection to a SQLite database given the
         * full path to the file that contains that database. */
        connection(const std::string& path);

    public:
        /* Runs a SELECT query against the given table, returning
         * a list of results that match the query.  Note that
         * there are a few different forms of this call: by
         * default this will select every row and column, but it's
         * strongly suggested that you use better query
         * methods. */
        result::ptr select(const table::ptr& table);
        result::ptr select(const table::ptr& table,
                           const std::vector<column::ptr>& c);
        result::ptr select(const table::ptr& table,
                           const char *format,
                           ...) __attribute__(( format(printf, 3, 4) ));
        result::ptr select(const table::ptr& table,
                           const char *format,
                           va_list args);
        result::ptr select(const table::ptr& table,
                           const std::vector<column::ptr>& c,
                           const char *format,
                           ...) __attribute__(( format(printf, 4, 5) ));
        result::ptr select(const table::ptr& table,
                           const std::vector<column::ptr>& c,
                           const char *format,
                           va_list args);

        /* Exactly like select(), but just returns the count instead. */
        result::ptr count(const table::ptr& table);
        result::ptr count(const table::ptr& table,
                          const std::vector<column::ptr>& c);
        result::ptr count(const table::ptr& table,
                          const char *format,
                          ...) __attribute__(( format(printf, 3, 4) ));
        result::ptr count(const table::ptr& table,
                          const char *format,
                          va_list args);
        result::ptr count(const table::ptr& table,
                          const std::vector<column::ptr>& c,
                          const char *format,
                          ...) __attribute__(( format(printf, 4, 5) ));
        result::ptr count(const table::ptr& table,
                          const std::vector<column::ptr>& c,
                          const char *format,
                          va_list args);

        /* Runs a INSERT query against the given table, returning
         * a list of results that come back from the SQLite server
         * (note that I expect this to be none...). */
        result::ptr insert(const table::ptr& table,
                           const row::ptr& row);

        /* Runs a REPLACE query against the given table, returning
         * a list of results that come back from the SQLite server
         * (note that I expect this to be none...). */
        result::ptr replace(const table::ptr& table,
                            const row::ptr& row,
                            const char *format,
                            ...) __attribute__(( format(printf, 4, 5) ));
        result::ptr replace(const table::ptr& table,
                            const row::ptr& row,
                            const char *format,
                            va_list args);

        /* Removes any matching entries. */
        result::ptr remove(const table::ptr& table,
                           const char *format,
                           ...);
        result::ptr remove(const table::ptr& table,
                           const char *format,
                           va_list args);

        /* Sets the given columns to NULL in the matching
         * rows. */
        result::ptr clear(const table::ptr& table,
                          const std::vector<std::string>& cols,
                          const char *format,
                          ...) __attribute__(( format(printf, 4, 5) ));
        result::ptr clear(const table::ptr& table,
                          const std::vector<std::string>& cols,
                          const char *format,
                          va_list args);

        /* You can ask for two sorts of transactions on a
         * database: either a write-only lock or a read-write
         * lock. */
        psqlite::exclusive_transaction::ptr exclusive_transaction(void);
        psqlite::immediate_transaction::ptr immediate_transaction(void);
        psqlite::deferred_transaction::ptr deferred_transaction(void);

        /* Creates a new table */
        result::ptr create(const table::ptr& table);

    protected:
        /* This is really only allowed to be called from transaction. */
        friend class transaction;
        result::ptr commit_transaction(void);
    };
}

#endif
