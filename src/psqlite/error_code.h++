/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#ifndef PSQLITE__ERROR_CODE_HXX
#define PSQLITE__ERROR_CODE_HXX

#include <sqlite3.h>
#include <string>

namespace psqlite {
    enum class error_code {
        SUCCESS = 0,
        FAILED_UNIQUE = 19,
    };
}

#endif
