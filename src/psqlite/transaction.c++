
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

#include "transaction.h++"
#include "connection.h++"
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
