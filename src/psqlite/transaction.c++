/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#include "transaction.h++"
#include "connection.h++"
#include <stdlib.h>
using namespace psqlite;

transaction::transaction(connection *conn)
    : _conn(conn)
{
}

transaction::~transaction(void)
{
    if (_conn == NULL)
        return;

    auto resp = _conn->commit_transaction();
    switch (resp->return_value()) {
    case error_code::SUCCESS:
        break;

        /* These error codes can't actually happen. */
    case error_code::FAILED_UNIQUE:
        fprintf(stderr, "Error closing transaction: '%s'\n",
                resp->return_string().c_str());
        abort();
        break;
    }
}


exclusive_transaction::exclusive_transaction(connection *conn)
    : immediate_transaction(conn)
{
}

exclusive_transaction::~exclusive_transaction(void)
{
}


immediate_transaction::immediate_transaction(connection *conn)
    : deferred_transaction(conn)
{
}

immediate_transaction::~immediate_transaction(void)
{
}


deferred_transaction::deferred_transaction(connection *conn)
    : transaction(conn)
{
}

deferred_transaction::~deferred_transaction(void)
{
}
