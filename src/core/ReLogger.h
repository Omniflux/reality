  /*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LOGGER_H
#define RE_LOGGER_H

//! Logging facility
#include <iostream>
#include <fstream>
#include <QDesktopServices>
#include <QTime>
#include <QFileInfo>

#include "ReDefs.h"
#include "reality_lib_export.h"

// This define must be before including "cpplog.hpp"
// it implements a custom format for the log. See the class definition below
#define LOG_LEVEL(level, logger) CustomLogMessage(__FILE__, "", __LINE__, (level), logger).getStream()
#include "logging/cpplog.hpp"

using namespace cpplog;

//! File-based stream for persistance of the log
extern std::ofstream logFile;
REALITY_LIB_EXPORT extern cpplog::OstreamLogger RealityLogger;

// Shortcut macros to help us with the log
#define RE_LOG_INFO()  LOG_INFO(RealityLogger)
#define RE_LOG_WARN()  LOG_WARN(RealityLogger)
#define RE_LOG_DEBUG() LOG_DEBUG(RealityLogger)

// Used to print a QString using the std::stream 
#define QSS(str) str.toStdString()
#define QVS(var) var.toString().toStdString()

/**
 * This class implements a custom format for the log
 */
class REALITY_LIB_EXPORT CustomLogMessage : public cpplog::LogMessage {
private:
  static QString prefix;

public:
  CustomLogMessage( const char* file, 
                    const char* /* function */,
                    unsigned int line, 
                    cpplog::loglevel_t logLevel,
                    cpplog::BaseLogger &outputLogger ) :
    cpplog::LogMessage(file, line, logLevel, outputLogger, false)
  {
    InitLogMessage();
  }

  static const char* shortLogLevelName(cpplog::loglevel_t logLevel) {
    switch( logLevel ) {
      case LL_TRACE: return "$";
      case LL_DEBUG: return "#";
      case LL_INFO:  return "";
      case LL_WARN:  return "!";
      case LL_ERROR: return "!!";
      case LL_FATAL: return "!!!";
      default:       return "O";
    };
  }

  static void setPrefix( const QString& newPrefix ) {
    prefix = newPrefix;
  }

protected:
  void InitLogMessage() {
    m_logData->stream << QDateTime::currentDateTime()
                           .toString("yy-M-dd hh:mm:ss").toAscii().data()
                      << " - "
// Don't print the file and line number unless we are using the
// debug build                      
#ifndef NDEBUG                      
                      << m_logData->fileName << ":"
                      << m_logData->line
#endif                      
                      << (prefix.isEmpty() ? "" : QString("(%1)").arg(prefix).toStdString())
                      << shortLogLevelName(m_logData->level)
                      << " ";
  }
};

inline void RE_createLogger( QString prefix = "" ) {
  QString documentsDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
  QString logFileName = QString("%1/Reality_plugin_Log.txt").arg(documentsDir);
  QFile logFileHandler(logFileName);
  if (logFileHandler.size() > RE_LOG_FILE_MAX_SIZE) {
    logFileHandler.remove();
  }
  logFile.open(logFileName.toUtf8(), std::ios::app);
  if (!prefix.isEmpty()) {
    CustomLogMessage::setPrefix(prefix);
  }
}

inline void RE_closeLogger() {
  if ( logFile.is_open() ) {
    logFile.close();
  }
}

#endif
