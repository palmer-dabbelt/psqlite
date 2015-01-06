
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

#ifndef PSQLITE__TRANSACTION_HXX
#define PSQLITE__TRANSACTION_HXX

#include <memory>

namespace psqlite {
    class connection;
}

namespace psqlite {
    /* This is a generic transaction object.  Note that you'll
     * want to use one of the more specific ones below, as they're
     * actually useful. */
    class transaction {
    public:
        typedef std::shared_ptr<transaction> ptr;

    public:
        /* Here is an explicitly non-shared pointer to the
         * connection, which we need because these come from the
         * connection and I don't want to deal with a self-pointer
         * right now... */
        connection *_conn;

    public:
        /* Creates a new transaction. */
        transaction(connection *conn);

        /* Closes a transaction, actually committing it. */
        virtual ~transaction(void);
    };

    class deferred_transaction: public transaction {
    public:
        typedef std::shared_ptr<deferred_transaction> ptr;
        deferred_transaction(connection *conn);
        ~deferred_transaction(void);
    };

    class immediate_transaction: public deferred_transaction {
    public:
        typedef std::shared_ptr<immediate_transaction> ptr;
        immediate_transaction(connection *conn);
        ~immediate_transaction(void);
    };

    class exclusive_transaction: public immediate_transaction {
    public:
        typedef std::shared_ptr<exclusive_transaction> ptr;
        exclusive_transaction(connection *conn);
        ~exclusive_transaction(void);
    };
}

#endif
