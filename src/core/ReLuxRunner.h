/**
 *  \file ReLuxRunner.h
 *
 *  Created by Paolo H Ciccone on 6/28/11.
 *  Copyright 2011 Pret-A-3D. All rights reserved.
 *
 */

#ifndef RE_LUX_RUNNER
#define RE_LUX_RUNNER

#include <QProcess>
#include <QHash>
#include <QByteArray>
#include "ReDefs.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * This class is used to executed LuxRender and SLG.
 */
class REALITY_LIB_EXPORT ReLuxRunner : public QObject {
  Q_OBJECT
  
public:

  enum ExecutionResult {
    LR_NO_ERROR,
    LR_NORMAL_EXIT,
    LR_COULD_NOT_START,
    LR_ABNORMAL_EXIT,
    LR_LUX_NOT_FOUND,
    LR_SLG_NOT_FOUND,
    LR_CANNOT_CREATE_ANIMATION_FILE
  };
  
protected:    
  ExecutionResult retCode;

  QProcess luxProc;
  QByteArray procOutput;

  //! Returns the command line to run the LuxRender gui.
  //! \param doc The scene file to render with LuxRender
  //! \param args A list of arguments to pass to the program
  QString makeGuiCommandLine(  const QString& doc, 
                               const QStringList& args,
                               const LUX_LOG_LEVEL logLevel = LUX_WARNINGS );

public:

  // ctor
  ReLuxRunner();

  inline ExecutionResult getReCode() {
    return retCode;
  }

  inline QProcess::ProcessState getState() {
    return luxProc.state();
  }

  inline bool isFinished() {
    return luxProc.state() == QProcess::NotRunning;
  }
  
  /**
   * Used to run the GUI version of Lux.
   */
  ExecutionResult runGuiLux( const QString sceneFileName, 
                                    const uint maxThreads,
                                    const LUX_LOG_LEVEL logLevel = LUX_WARNINGS );  
  ExecutionResult runGuiLux( const QStringList renderQueue, 
                                    const uint maxThreads,
                                    const LUX_LOG_LEVEL logLevel = LUX_WARNINGS );

  ExecutionResult runLuxConsole( const QString& processInput, 
                                 const bool useBinDump = true,
                                 const bool waitForCompletion = true );

  ExecutionResult renderViaConsole( const QString& sceneFileName,
                                    bool waitForCompletion = false );

  ExecutionResult startSuspendedLuxConsole( const bool useBinDump = true );
  ExecutionResult activateLuxConsole( const QString& processInput );

  inline QByteArray& getProcOutput() {
    return procOutput;
  };

  inline void writeToStdin( const QString& input ) {
    luxProc.write(input.toUtf8());
    luxProc.closeWriteChannel();
  }

  inline void waitForFinished() {
    luxProc.waitForFinished();
  }

  inline QByteArray& readAllStandardOutput() {
    procOutput = luxProc.readAllStandardOutput();
    return procOutput;
  }
  
  /**
   * Searches the path pointed by the user and returns the absolute path of the Lux GUI executable 
   * as used by the current OS.
   */
  QString getLuxRenderProgramName();

  /**
   * Used to run SmallLuxGPU.
   */
  ExecutionResult runSLG( const QString sceneFileName );  


  /**
   * Searches the path pointed by the user and returns the absolute path of the SLG  executable 
   * as used by the current OS.
   */  
  QString getSLGRenderProgramName();
   
  /**
   * Searches the path pointed by the user and returns the absolute path of the Lux console executable 
   * as used by the current OS.
   */
  QString getLuxConsoleProgramName();
 
  /**
   * Called when the SLG process has data in stdout that is ready to be read
   */
  void processSLGstdout();

  /**
   * Stop the execution of the current process. Usually called in response to the user's
   * request to stop the render for background processes (SLG batch mode)
   */
  void killProcess();

signals:
  void SLGStdoutReady( QByteArray& stdoutMsg );
  void SLGfinished();
  void luxconsoleFailed();

};

typedef QSharedPointer<ReLuxRunner> ReLuxRunnerPtr;

}
#endif
