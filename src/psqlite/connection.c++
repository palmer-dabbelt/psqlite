/* Copyright (C) 2014 Palmer Dabbelt <palmer@dabbelt.com> */
/* SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0 OR BSD-3-Clause */

#include "connection.h++"
#include <iterator>
#include <map>
#include <sstream>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
using namespace psqlite;

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

/* SQLite uses a callback for enumerating results, but unfortunately
 * this requires that you use C function pointers instead of C++
 * std::function.  Thus I have to do the whole struct/wrapper thing
 * here... :(. */
struct sqlite3_exec_args {
    result::ptr result_ptr;
};
static int sqlite3_exec_func(void *args,
                             int count,
                             char **data,
                             char **names);

connection::connection(const std::string& db_path)
    : _db(NULL),
      _tr(std::shared_ptr<transaction>(NULL))
{
    int err = sqlite3_open(db_path.c_str(), &_db);
    if (err != SQLITE_OK) {
        perror("Unable t open sqlite database");
        fprintf(stderr, "  database path:'%s'\n", db_path.c_str());
        fprintf(stderr, "  error: %d\n", err);
        abort();
    }

    sqlite3_busy_timeout(_db, 1000);
}

connection::~connection(void)
{
    sqlite3_close(_db);
}

result::ptr connection::select(const table::ptr& table)
{
    return select(table, "'true'='true'");
}


result::ptr connection::select(const table::ptr& table,
                               const char * format, ...)
{
    va_list args; va_start(args, format);
    auto out = select(table, format, args);
    va_end(args);
    return out;
}

result::ptr connection::select(const table::ptr& table,
                               const char * format,
                               va_list args)
{
    return select(table, table->columns(), format, args);
}

result::ptr connection::select(const table::ptr& table,
                               const std::vector<column::ptr>& c,
                               const char *format,
                               va_list args)
{
    /* It turns out that SQLite provides a mechanism for eliminating
     * SQL injection attacks, but it conflicts with GCC's printf-like
     * format string checker.  Thus I work around the problem by
     * simply converting everything to injection-proof right here. */
    char *nformat = new char[strlen(format) + 1];
    strcpy(nformat, format);
    for (size_t i = 0; i < strlen(nformat); ++i)
        if (strncmp(nformat + i, "%s",  2) == 0)
            nformat[i+1] = 'q';

    /* Here we format the string to avoid injection attacks. */
    va_list sargs; va_copy(sargs, args);
    char test_args[2];
    size_t query_length = vsnprintf(test_args, 2, format, sargs) + 1;
    char *query = new char[query_length];
    sqlite3_vsnprintf(query_length, query, nformat, args);

    /* That's not the whole SQL command, we also need the "SELECT (...)
     * FROM ..." part.  */
    size_t column_length = 2;
    for (const auto& column: c)
        column_length += strlen(column->name().c_str()) + 2;
    char *column_spec = new char[column_length];
    column_spec[0] = '\0';
    for (const auto& column: c) {
        strcat(column_spec, column->name().c_str());
        strcat(column_spec, ", ");
    }
    column_spec[strlen(column_spec)-2] = '\0';

    /* Now we can assemble the final SQL query string. */
    size_t command_length =
        strlen("SELECT ")
        + strlen(column_spec)
        + strlen(" FROM ")
        + strlen(table->name().c_str())
        + strlen(" WHERE ")
        + strlen(query)
        + 2;
    char *command = new char[command_length];
    sprintf(command, "SELECT %s FROM %s WHERE %s;",
            column_spec,
            table->name().c_str(),
            query);
#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", command);
#endif

    /* SQLite fills out an argument pointer, so we need one
     * created. */
    auto out = std::make_shared<result>();

    /* At this point the SQL query can actually be run. */
    {
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 command,
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);
    }

    /* Finally we can clean up those allocated strings and return! */
    delete[] nformat;
    delete[] query;
    delete[] column_spec;
    delete[] command;
    return out;
}

result::ptr connection::count(const table::ptr& table)
{
    return count(table, "'true'='true'");
}


result::ptr connection::count(const table::ptr& table,
                              const char * format, ...)
{
    va_list args; va_start(args, format);
    auto out = count(table, format, args);
    va_end(args);
    return out;
}

result::ptr connection::count(const table::ptr& table,
                              const char * format,
                              va_list args)
{
    return count(table, table->columns(), format, args);
}

result::ptr connection::count(const table::ptr& table,
                              const std::vector<column::ptr>& c,
                              const char *format,
                              va_list args)
{
    /* It turns out that SQLite provides a mechanism for eliminating
     * SQL injection attacks, but it conflicts with GCC's printf-like
     * format string checker.  Thus I work around the problem by
     * simply converting everything to injection-proof right here. */
    char *nformat = new char[strlen(format) + 1];
    strcpy(nformat, format);
    for (size_t i = 0; i < strlen(nformat); ++i)
        if (strncmp(nformat + i, "%s",  2) == 0)
            nformat[i+1] = 'q';

    /* Here we format the string to avoid injection attacks. */
    va_list sargs; va_copy(sargs, args);
    char test_args[2];
    size_t query_length = vsnprintf(test_args, 2, format, sargs) + 1;
    char *query = new char[query_length];
    sqlite3_vsnprintf(query_length, query, nformat, args);

    /* That's not the whole SQL command, we also need the "SELECT (...)
     * FROM ..." part.  */
    size_t column_length = 2;
    for (const auto& column: c)
        column_length += strlen("COUNT( ),  ") + strlen(column->name().c_str());
    char *column_spec = new char[column_length];
    column_spec[0] = '\0';
    for (const auto& column: c) {
        strcat(column_spec, "COUNT(");
        strcat(column_spec, column->name().c_str());
        strcat(column_spec, "), ");
    }
    column_spec[strlen(column_spec)-2] = '\0';

    /* Now we can assemble the final SQL query string. */
    size_t command_length =
        strlen("SELECT ")
        + strlen(column_spec)
        + strlen(" FROM ")
        + strlen(table->name().c_str())
        + strlen(" WHERE ")
        + strlen(query)
        + 2;
    char *command = new char[command_length];
    sprintf(command, "SELECT %s FROM %s WHERE %s;",
            column_spec,
            table->name().c_str(),
            query);
#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", command);
#endif

    /* SQLite fills out an argument pointer, so we need one
     * created. */
    auto out = std::make_shared<result>();

    /* At this point the SQL query can actually be run. */
    {
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 command,
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);
    }

    /* Finally we can clean up those allocated strings and return! */
    delete[] nformat;
    delete[] query;
    delete[] column_spec;
    delete[] command;
    return out;
}

result::ptr connection::insert(const table::ptr& table,
                                              const row::ptr& row)
{
    std::string command = "INSERT INTO " + table->name() + " (";
    for (const auto& column: row->columns()) {
        auto format = new char[column.size() + 10];
        sqlite3_snprintf(column.size() + 10,
                         format, "%s",
                         column.c_str()
            );
        command = command + format + ", ";
        delete[] format;
    }
    command.replace(strlen(command.c_str())-2, 1, "\0");
    command = command + ") VALUES (";
    for (const auto& column: row->columns()) {
        auto format = new char[row->value(column).size() + 10];
        sqlite3_snprintf(row->value(column).size() + 10,
                         format, "'%q'",
                         row->value(column).c_str());
        command = command + format + ", ";
        delete[] format;
    }
    command.replace(strlen(command.c_str())-2, 1, "\0");
    command = command + ")";

#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", command.c_str());
#endif

    /* SQLite fills out an argument pointer, so we need one
     * created. */
    auto out = std::make_shared<result>();

    /* At this point the SQL query can actually be run. */
    {
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 command.c_str(),
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);
    }

    /* Finally we can clean up those allocated strings and return! */
    return out;
}

result::ptr connection::replace(const table::ptr& table,
                                               const row::ptr& row,
                                               const char *format,
                                               ...)
{
    va_list args; va_start(args, format);
    auto out = replace(table, row, format, args);
    va_end(args);
    return out;
}

result::ptr connection::replace(const table::ptr& table,
                                               const row::ptr& row,
                                               const char *format,
                                               va_list args)
{
    /* It turns out that SQLite provides a mechanism for eliminating
     * SQL injection attacks, but it conflicts with GCC's printf-like
     * format string checker.  Thus I work around the problem by
     * simply converting everything to injection-proof right here. */
    char *nformat = new char[strlen(format) + 1];
    strcpy(nformat, format);
    for (size_t i = 0; i < strlen(nformat); ++i)
        if (strncmp(nformat + i, "%s",  2) == 0)
            nformat[i+1] = 'q';

    /* Here we format the string to avoid injection attacks. */
    va_list sargs; va_copy(sargs, args);
    char test_args[2];
    size_t query_length = vsnprintf(test_args, 2, format, sargs) + 1;
    char *query = new char[query_length];
    sqlite3_vsnprintf(query_length, query, nformat, args);

    /* That's not the whole SQL command, we also need the "SELECT (...)
     * FROM ..." part.  */
    size_t column_length = 3;
    for (const auto& column: row->columns()) {
        column_length += strlen(column.c_str());
        column_length += 2;
        column_length += strlen(row->value(column).c_str());
        column_length += 3;
    }
    char *column_spec = new char[column_length];
    column_spec[0] = '\0';
    for (const auto& column: row->columns()) {
        strcat(column_spec, column.c_str());
        strcat(column_spec, "='");
        strcat(column_spec, row->value(column).c_str() );
        strcat(column_spec, "', ");
    }
    column_spec[strlen(column_spec)-2] = '\0';

    /* Now we can assemble the final SQL query string. */
    size_t command_length =
        strlen("UPDATE ")
        + strlen(table->name().c_str())
        + strlen(" SET ")
        + strlen(column_spec)
        + strlen(" WHERE ")
        + strlen(query)
        + 2;
    char *command = new char[command_length];
    sprintf(command, "UPDATE %s SET %s WHERE %s;",
            table->name().c_str(),
            column_spec,
            query);
#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", command);
#endif

    /* SQLite fills out an argument pointer, so we need one
     * created. */
    auto out = std::make_shared<result>();

    /* At this point the SQL query can actually be run. */
    {
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 command,
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);
    }

    /* Finally we can clean up those allocated strings and return! */
    delete[] nformat;
    delete[] query;
    delete[] column_spec;
    delete[] command;
    return out;
}

result::ptr connection::remove(const table::ptr& table,
                                              const char *format,
                                              ...)
{
    va_list args; va_start(args, format);
    auto out = remove(table, format, args);
    va_end(args);
    return out;
}

result::ptr connection::remove(const table::ptr& table,
                                              const char *format,
                                              va_list args)
{
    /* It turns out that SQLite provides a mechanism for eliminating
     * SQL injection attacks, but it conflicts with GCC's printf-like
     * format string checker.  Thus I work around the problem by
     * simply converting everything to injection-proof right here. */
    char *nformat = new char[strlen(format) + 1];
    strcpy(nformat, format);
    for (size_t i = 0; i < strlen(nformat); ++i)
        if (strncmp(nformat + i, "%s",  2) == 0)
            nformat[i+1] = 'q';

    /* Here we format the string to avoid injection attacks. */
    va_list sargs; va_copy(sargs, args);
    char test_args[2];
    size_t query_length = vsnprintf(test_args, 2, format, sargs) + 1;
    char *query = new char[query_length];
    sqlite3_vsnprintf(query_length, query, nformat, args);

    /* Now we can assemble the final SQL query string. */
    size_t command_length =
        strlen("DELETE FROM ")
        + strlen(table->name().c_str())
        + strlen(" WHERE ")
        + strlen(query)
        + 2;
    char *command = new char[command_length];
    sprintf(command, "DELETE FROM %s WHERE %s;",
            table->name().c_str(),
            query);
#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", command);
#endif

    /* SQLite fills out an argument pointer, so we need one
     * created. */
    auto out = std::make_shared<result>();

    /* At this point the SQL query can actually be run. */
    {
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 command,
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);
    }

    /* Finally we can clean up those allocated strings and return! */
    delete[] nformat;
    delete[] query;
    delete[] command;
    return out;
}

result::ptr connection::clear(const table::ptr& table,
                                             const std::vector<std::string>& cols,
                                             const char *format,
                                             ...)
{
    va_list args; va_start(args, format);
    auto out = clear(table, cols, format, args);
    va_end(args);
    return out;
}

result::ptr connection::clear(const table::ptr& table,
                                             const std::vector<std::string>& cols,
                                             const char *format,
                                             va_list args)
{
    /* It turns out that SQLite provides a mechanism for eliminating
     * SQL injection attacks, but it conflicts with GCC's printf-like
     * format string checker.  Thus I work around the problem by
     * simply converting everything to injection-proof right here. */
    char *nformat = new char[strlen(format) + 1];
    strcpy(nformat, format);
    for (size_t i = 0; i < strlen(nformat); ++i)
        if (strncmp(nformat + i, "%s",  2) == 0)
            nformat[i+1] = 'q';

    /* Here we format the string to avoid injection attacks. */
    va_list sargs; va_copy(sargs, args);
    char test_args[2];
    size_t query_length = vsnprintf(test_args, 2, format, sargs) + 1;
    char *query = new char[query_length];
    sqlite3_vsnprintf(query_length, query, nformat, args);

    /* That's not the whole SQL command, we also need the "SELECT (...)
     * FROM ..." part.  */
    size_t column_length = 3;
    for (const auto& column: cols) {
        column_length += strlen(column.c_str());
        column_length += 3;
        column_length += strlen("NULL");
        column_length += 1;
    }
    char *column_spec = new char[column_length];
    column_spec[0] = '\0';
    for (const auto& column: cols) {
        strcat(column_spec, column.c_str());
        strcat(column_spec, "=");
        strcat(column_spec, "NULL");
        strcat(column_spec, ", ");
    }
    column_spec[strlen(column_spec)-2] = '\0';

    /* Now we can assemble the final SQL query string. */
    size_t command_length =
        strlen("UPDATE ")
        + strlen(table->name().c_str())
        + strlen(" SET ")
        + strlen(column_spec)
        + strlen(" WHERE ")
        + strlen(query)
        + 2;
    char *command = new char[command_length];
    sprintf(command, "UPDATE %s SET %s WHERE %s;",
            table->name().c_str(),
            column_spec,
            query);
#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", command);
#endif

    /* SQLite fills out an argument pointer, so we need one
     * created. */
    auto out = std::make_shared<result>();

    /* At this point the SQL query can actually be run. */
    {
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 command,
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);
    }

    /* Finally we can clean up those allocated strings and return! */
    delete[] nformat;
    delete[] query;
    delete[] column_spec;
    delete[] command;
    return out;
}

exclusive_transaction::ptr
connection::exclusive_transaction(void)
{
    /* Check to see if there's already an existing transaction. */
    auto tr = _tr.lock();
    if (tr != NULL) {
        auto cast = std::dynamic_pointer_cast<psqlite::exclusive_transaction>(tr);
        if (cast == NULL) {
            fprintf(stderr, "Attempted to grab an exclusive transaction\n");
            fprintf(stderr, "  Weaker transaction already in use\n");
            abort();
        }

        return cast;
    }

#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", "BEGIN EXCLUSIVE TRANSACTION;");
#endif

    /* At this point the SQL query can actually be run. */
    {
        auto out = std::make_shared<result>();
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 "BEGIN EXCLUSIVE TRANSACTION;",
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);

        switch (out->return_value()) {
        case error_code::SUCCESS:
            break;

            /* These error codes can't actually happen. */
        case error_code::FAILED_UNIQUE:
            fprintf(stderr, "Error opening transaction: '%s'\n",
                    out->return_string().c_str());
            abort();
            break;
        }
    }

    auto ntr = std::make_shared<psqlite::exclusive_transaction>(this);
    _tr = ntr;
    return ntr;
}

immediate_transaction::ptr
connection::immediate_transaction(void)
{
    /* Check to see if there's already an existing transaction. */
    auto tr = _tr.lock();
    if (tr != NULL) {
        auto cast = std::dynamic_pointer_cast<psqlite::immediate_transaction>(tr);
        if (cast == NULL) {
            fprintf(stderr, "Attempted to grab an immediate transaction\n");
            fprintf(stderr, "  Weaker transaction already in use\n");
            abort();
        }

        return cast;
    }

#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", "BEGIN IMMEDIATE TRANSACTION;");
#endif

    /* At this point the SQL query can actually be run. */
    {
        auto out = std::make_shared<result>();
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 "BEGIN IMMEDIATE TRANSACTION;",
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);

        switch (out->return_value()) {
        case error_code::SUCCESS:
            break;

            /* These error codes can't actually happen. */
        case error_code::FAILED_UNIQUE:
            fprintf(stderr, "Error opening transaction: '%s'\n",
                    out->return_string().c_str());
            abort();
            break;
        }
    }

    auto ntr = std::make_shared<psqlite::immediate_transaction>(this);
    _tr = ntr;
    return ntr;
}

deferred_transaction::ptr
connection::deferred_transaction(void)
{
    /* Check to see if there's already an existing transaction. */
    auto tr = _tr.lock();
    if (tr != NULL) {
        auto cast = std::dynamic_pointer_cast<psqlite::deferred_transaction>(tr);
        if (cast == NULL) {
            fprintf(stderr, "Attempted to grab an deferred transaction\n");
            fprintf(stderr, "  Weaker transaction already in use\n");
            abort();
        }

        return cast;
    }

#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", "BEGIN DEFERRED TRANSACTION;");
#endif

    /* At this point the SQL query can actually be run. */
    {
        auto out = std::make_shared<result>();
        struct sqlite3_exec_args args;
        char *error_string = NULL;
        args.result_ptr = out;
        int error = sqlite3_exec(_db,
                                 "BEGIN DEFERRED TRANSACTION;",
                                 &sqlite3_exec_func,
                                 &args,
                                 &error_string);
        if (error_string == NULL)
            error_string = (char *)"";
        out->set_error(error, error_string);

        switch (out->return_value()) {
        case error_code::SUCCESS:
            break;

            /* These error codes can't actually happen. */
        case error_code::FAILED_UNIQUE:
            fprintf(stderr, "Error opening transaction: '%s'\n",
                    out->return_string().c_str());
            abort();
            break;
        }
    }

    auto ntr = std::make_shared<psqlite::deferred_transaction>(this);
    _tr = ntr;
    return ntr;
}

result::ptr connection::create(const table::ptr& table)
{
    std::ostringstream ss;

    ss << "CREATE TABLE IF NOT EXISTS " << table->name() << " (";
    {
        std::vector<std::string> col_names;
        for (const auto& col: table->columns()) {
            col_names.push_back(col->name());
            col_names.push_back(", ");
        }
        col_names.erase(col_names.begin() + (col_names.size() - 1));
        copy(col_names.begin(),
             col_names.end(),
             std::ostream_iterator<std::string>(ss, ""));
    }
    ss << ");";

#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", ss.str().c_str());
#endif

    auto out = std::make_shared<result>();
    struct sqlite3_exec_args args;
    char *error_string = NULL;
    args.result_ptr = out;
    int error = sqlite3_exec(_db,
                             ss.str().c_str(),
                             &sqlite3_exec_func,
                             &args,
                             &error_string);
    if (error_string == NULL)
        error_string = (char *)"";
    out->set_error(error, error_string);

    return out;
}

result::ptr connection::commit_transaction(void)
{
#ifdef DEBUG_SQLITE_COMMANDS
    fprintf(stderr, "command: '%s'\n", "END TRANSACTION;");
#endif

    auto out = std::make_shared<result>();
    struct sqlite3_exec_args args;
    char *error_string = NULL;
    args.result_ptr = out;
    int error = sqlite3_exec(_db,
                             "END TRANSACTION;",
                             &sqlite3_exec_func,
                             &args,
                             &error_string);
    if (error_string == NULL)
        error_string = (char *)"";
    out->set_error(error, error_string);

    return out;
}

int sqlite3_exec_func(void *args_uncast,
                      int count,
                      char **data,
                      char **names)
{
    std::map<std::string, std::string> name2datum;
    for (int i = 0; i < count; ++i) {
        if (data[i] != NULL) {
            std::string name = names[i];
            std::string datum = data[i];
            name2datum[name] = datum;
        }
    }

    struct sqlite3_exec_args *args = (struct sqlite3_exec_args *)args_uncast;
    args->result_ptr->add_map(name2datum);

    return 0;
}
