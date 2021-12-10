/**
 * @file    Database.cpp
 * @ingroup SQLiteCpp
 * @brief   Management of a SQLite Database Connection.
 *
 * Copyright (c) 2012-2013 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#include <Database.h>

#include <Statement.h>
#include <Assertion.h>
#include <Exception.h>

#include <string>

#ifndef SQLITE_DETERMINISTIC
#define SQLITE_DETERMINISTIC 0x800
#endif // SQLITE_DETERMINISTIC


namespace SQLite
{


// Open the provided database UTF-8 filename with SQLITE_OPEN_xxx provided flags.
Database::Database(const char* apFilename, const int aFlags /*= SQLITE_OPEN_READONLY*/, const char* apVfs /*= NULL*/) :
    mpSQLite(NULL),
    mFilename(apFilename)
{
    int ret = sqlite3_open_v2(apFilename, &mpSQLite, aFlags, apVfs);
    if (SQLITE_OK != ret)
    {
        std::string strerr = sqlite3_errmsg(mpSQLite);
        sqlite3_close(mpSQLite); // close is required even in case of error on opening
        throw SQLite::Exception(strerr);
    }
}

// Open the provided database UTF-8 filename with SQLITE_OPEN_xxx provided flags.
Database::Database(const std::string& aFilename, const int aFlags /*= SQLITE_OPEN_READONLY*/, const std::string& aVfs) :
    mpSQLite(NULL),
    mFilename(aFilename)
{
    int ret = sqlite3_open_v2(aFilename.c_str(), &mpSQLite, aFlags, aVfs.empty() ? NULL : aVfs.c_str());
    if (SQLITE_OK != ret)
    {
        std::string strerr = sqlite3_errmsg(mpSQLite);
        sqlite3_close(mpSQLite); // close is required even in case of error on opening
        throw SQLite::Exception(strerr);
    }
}

// Close the SQLite database connection.
Database::~Database() noexcept // nothrow
{
    int ret = sqlite3_close(mpSQLite);
    // Never throw an exception in a destructor
    SQLITECPP_ASSERT(SQLITE_OK == ret, sqlite3_errmsg(mpSQLite));  // See SQLITECPP_ENABLE_ASSERT_HANDLER
}

// Shortcut to execute one or multiple SQL statements without results (UPDATE, INSERT, ALTER, COMMIT, CREATE...).
int Database::exec(const char* apQueries)
{
    int ret = sqlite3_exec(mpSQLite, apQueries, NULL, NULL, NULL);
    check(ret);

    // Return the number of rows modified by those SQL statements (INSERT, UPDATE or DELETE only)
    return sqlite3_changes(mpSQLite);
}

// Shortcut to execute a one step query and fetch the first column of the result.
// WARNING: Be very careful with this dangerous method: you have to
// make a COPY OF THE result, else it will be destroy before the next line
// (when the underlying temporary Statement and Column objects are destroyed)
// this is an issue only for pointer type result (ie. char* and blob)
// (use the Column copy-constructor)
Column Database::execAndGet(const char* apQuery)
{
    Statement query(*this, apQuery);
    (void)query.executeStep(); // Can return false if no result, which will throw next line in getColumn()
    return query.getColumn(0);
}

// Shortcut to test if a table exists.
bool Database::tableExists(const char* apTableName)
{
    Statement query(*this, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?");
    query.bind(1, apTableName);
    (void)query.executeStep(); // Cannot return false, as the above query always return a result
    int Nb = query.getColumn(0);
    return (1 == Nb);
}

// Check if aRet equal SQLITE_OK, else throw a SQLite::Exception with the SQLite error message
void Database::check(const int aRet) const
{
    if (SQLITE_OK != aRet)
    {
        throw SQLite::Exception(sqlite3_errmsg(mpSQLite));
    }
}

// Attach a custom function to your sqlite database. Assumes UTF8 text representation.
// Parameter details can be found here: http://www.sqlite.org/c3ref/create_function.html
void Database::createFunction(const char*   apFuncName,
                              int           aNbArg,
                              bool          abDeterministic,
                              void*         apApp,
                              void        (*apFunc)(sqlite3_context *, int, sqlite3_value **),
                              void        (*apStep)(sqlite3_context *, int, sqlite3_value **),
                              void        (*apFinal)(sqlite3_context *),   // NOLINT(readability/casting)
                              void        (*apDestroy)(void *))
{
    int TextRep = SQLITE_UTF8;
    // optimization if deterministic function (e.g. of nondeterministic function random())
    if (abDeterministic) {
        TextRep = TextRep|SQLITE_DETERMINISTIC;
    }
    int ret = sqlite3_create_function_v2(mpSQLite, apFuncName, aNbArg, TextRep,
                                         apApp, apFunc, apStep, apFinal, apDestroy);

    check(ret);
}


}  // namespace SQLite
