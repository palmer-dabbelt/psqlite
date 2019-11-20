/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

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
