/**
 * @file    Assertion.h
 * @ingroup SQLiteCpp
 * @brief   Definition of the SQLITECPP_ASSERT() macro.
 *
 * Copyright (c) 2012-2013 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */
#pragma once

#include <cassert>
#include "ReLogger.h"
#include "ReDefs.h"
/**
 * SQLITECPP_ASSERT SQLITECPP_ASSERT() is used in destructors, where exceptions shall not be thrown
 *
 * Define SQLITECPP_ENABLE_ASSERT_HANDLER at the project level
 * and define a SQLite::assertion_failed() assertion handler
 * to tell SQLiteC++ to use it instead of assert() when an assertion fail.
*/
#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER

// if an assert handler is provided by user code, use it instead of assert()
namespace SQLite
{
    // declaration of the assert handler to define in user code
    inline void assertion_failed(const char* apFile, 
                                 const long apLine, 
                                 const char* UNUSED(apFunc),
                                 const char* UNUSED(apExpr), 
                                 const char* apMsg) 
    {
      RE_LOG_WARN() << QString(
                         "Assertion failed in SQLite: %1@%2 -- %3"
                       )
                       .arg(apFile)
                       .arg(apLine)
                       .arg(apMsg)
                       .toStdString();
    }

} // namespace SQLite

#ifdef _MSC_VER
    #define __func__ __FUNCTION__
#endif
// call the assert handler provided by user code
#define SQLITECPP_ASSERT(expression, message) \
    if (!(expression))  SQLite::assertion_failed(__FILE__, __LINE__, __func__, #expression, message)

#else

// if no assert handler provided by user code, use standard assert()
// (note: in debug mode, assert() does nothing)
#define SQLITECPP_ASSERT(expression, message)   assert(expression && message)

#endif
