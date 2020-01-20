/**
 * \file
 *
 * Created by Paolo H Ciccone on 6/28/11.
 * Copyright (c) 2011 Pret-A-3D. All rights reserved.
 *
 */

#include <QFileInfo>

#include "ReDefs.h"
#include "RealityBase.h"
#include "ReLogger.h"
#include "ReRenderContext.h"

#define RE_DEBUG_RUNNER  0

#if RE_DEBUG_RUNNER ==1
  #include <QDebug>
#endif

#include "ReLuxRunner.h"

using namespace Reality;

// Static variables initialization

ReLuxRunner::ReLuxRunner() {
  retCode  = LR_NO_ERROR;
};


QString ReLuxRunner::makeGuiCommandLine( const QString& doc, 
                                         const QStringList& args,
                                         const LUX_LOG_LEVEL logLevel ) 
{
  // Retrieve the list of render nodes and add it to the command line to 
  // use network rendering.
  auto config = RealityBase::getConfiguration();
  QString renderNodeCfg = config->value("LuxRenderNodes").toString();
  QString renderNodes;
  if (!renderNodeCfg.isEmpty()) {
    renderNodes = QString(" -u ") + renderNodeCfg.split(",").join(" -u ");
  }
  QString luxProgramPath = RealityBase::getRealityBase()
                             ->getRendererPath(LuxRender);
  #ifdef __APPLE__
  // Using open makes the program behave as if it was started
  // from Finder, which is better in general. It also works around
  // the bug in LuxRender 1.3.1 that stops the program when non-ASCII
  // characters, Unicode, are in the document's file name
  // -n is used to open a new instance of the app, even if one is already
  // running. We want to be able to run multiple instances of Lux.
  QString cmdLine = QString("/usr/bin/open \"%1\" -n -a \"%2\"")
                      .arg(doc)
                      .arg(luxProgramPath);
  cmdLine += QString(" --args %1").arg(args.join(" "));
  if (!renderNodes.isEmpty()) {
    cmdLine += renderNodes;
  }
  #elif defined(WIN32)
  // Quotes to avoid issues with spaces in the paths
  QString cmdLine = QString("\"%1\" \"%2\"")
                      .arg(luxProgramPath)
                      .arg(doc);
  cmdLine += " " + args.join(" ");
  if (!renderNodes.isEmpty()) {
    cmdLine += renderNodes;
  }
  #endif
  // String the sets the level of verbosity for the log tab
  QString logLevelParam;
  switch(logLevel) {
    case LUX_WARNINGS:
      logLevelParam = "-q";
      break;
    case LUX_ERRORS:
      logLevelParam = "-x";
      break;
    case LUX_DEBUG:
      logLevelParam = "-V";
    default:
      break;
  }

  return cmdLine + " " + logLevelParam;
}

/**
 * Used to run the GUI version of Lux.
 */
ReLuxRunner::ExecutionResult ReLuxRunner::runGuiLux( const QString sceneFileName, 
                                                     const uint maxThreads,
                                                     const LUX_LOG_LEVEL logLevel ) 
{
  QProcess luxProc;
  QStringList args;
  auto renderContext = ReRenderContext::getInstance();

  if ( maxThreads != 0 ) {    
    args << "--threads" << QString("%1").arg(maxThreads);
  }
  // If we use LuxCore then it's best to use a value for min epsilon of
  // 10^-5 to avoid recognizable patterns with the Sobol sampler
  if (renderContext->usesLuxCore()) {
    args << "-e" << "0.00001";
  }
  QString cmdLine = makeGuiCommandLine(sceneFileName, args, logLevel);
  // RE_LOG_INFO() << "Lux cmdline: " << cmdLine.toStdString();
  if (!luxProc.startDetached(cmdLine) ) {
    return ReLuxRunner::retCode = LR_COULD_NOT_START;
  };
  return ReLuxRunner::retCode = LR_NORMAL_EXIT;
}


/**
 * Used to run the GUI version of Lux but with a render queue. The Queue is passed as parameter and it's 
 * expected to be an array of scene file names.
 */

const char* REALITY_RENDER_QUEUE_FILE = "RealityQueue.lxq";

ReLuxRunner::ExecutionResult ReLuxRunner::runGuiLux( const QStringList renderQueue, 
                                                     const uint maxThreads,
                                                     const LUX_LOG_LEVEL logLevel ) {
  QProcess luxProc;
  QStringList args;

  QFileInfo firstFile(renderQueue.value(0));

  QString renderQueueFileName(QString("%1/%2").arg(firstFile.absolutePath()).arg(REALITY_RENDER_QUEUE_FILE));
  QFile renderQueueFile(renderQueueFileName);

  if (!renderQueueFile.open(QIODevice::WriteOnly)) {
    return ReLuxRunner::retCode = LR_CANNOT_CREATE_ANIMATION_FILE;
  }
  renderQueueFile.write(renderQueue.join("\n").toUtf8());
  renderQueueFile.close();

  QString cmdLine = RealityBase::getRealityBase()->getRendererPath(LuxRender);
  args << "--list-file" << renderQueueFileName;
  args << "--fixedseed";  // Useful to avoid noise in animations
  // String the sets the level of verbosity for the log tab
  QString logLevelParam;
  switch(logLevel) {
    case LUX_WARNINGS:
      logLevelParam = "-q";
      break;
    case LUX_ERRORS:
      logLevelParam = "-x";
      break;
    case LUX_DEBUG:
      logLevelParam = "-V";
    default:
      break;
  }
  if (!logLevelParam.isEmpty()) {
    args << logLevelParam;
  }
  if ( maxThreads != 0 ) {
    args << "--threads" << QString("%1").arg(maxThreads);
  }

  if (!luxProc.startDetached(cmdLine,args) ) {
    return ReLuxRunner::retCode = LR_COULD_NOT_START;
  };
  return ReLuxRunner::retCode = LR_NORMAL_EXIT;  
}

ReLuxRunner::ExecutionResult 
ReLuxRunner::runLuxConsole( const QString& processInput, 
                            const bool useBinDump,
                            const bool waitForCompletion ) 
{
  procOutput.clear();
  QStringList args;
  QString cmd = getLuxConsoleProgramName();
  if (cmd.isEmpty()) {
    return LR_LUX_NOT_FOUND;
  }
  // Make luxconsole extra quiet
  args << "-x";
  // Do we want to read the result as a stream of bytes?
  if (useBinDump) {
    args << "--bindump";
  }
  // Accept input from stdin
  args << "-";

  luxProc.start(cmd,args);
  if ( !luxProc.waitForStarted(2000) ) {
    RE_LOG_DEBUG() << "Timeout in starting luxconsole " << cmd.toStdString();
    return ReLuxRunner::retCode = LR_COULD_NOT_START;
  }
  if (!processInput.isEmpty()) {
    // RE_LOG_INFO() << "Writing scene to the luxconsole";
    luxProc.write(processInput.toUtf8());
    luxProc.closeWriteChannel();
  }

  if (waitForCompletion) {
    if ( !luxProc.waitForFinished() ) {
      RE_LOG_DEBUG() << "Timeout in executing luxconsole";
      procOutput = luxProc.readAllStandardOutput();
      procOutput.append(luxProc.readAllStandardError());
      return ReLuxRunner::retCode = LR_ABNORMAL_EXIT;
    }
    procOutput = luxProc.readAllStandardOutput();
  }
  return ReLuxRunner::retCode = LR_NORMAL_EXIT;
}

ReLuxRunner::ExecutionResult ReLuxRunner::renderViaConsole( 
                               const QString& sceneFileName,
                               bool waitForCompletion
                             )
{
  procOutput.clear();
  QStringList args;
  QString cmd = getLuxConsoleProgramName();
  if (cmd.isEmpty()) {
    return LR_LUX_NOT_FOUND;
  }
  // Make luxconsole extra quiet
  args << "-x";
  args << sceneFileName;
  luxProc.start(cmd,args);
  if ( !luxProc.waitForStarted(2000) ) {
    RE_LOG_DEBUG() << "Timeout in starting luxconsole " << cmd.toStdString();
    return ReLuxRunner::retCode = LR_COULD_NOT_START;
  }
  if (waitForCompletion) {
    if ( !luxProc.waitForFinished() ) {
      RE_LOG_DEBUG() << "Timeout in executing luxconsole";
      procOutput = luxProc.readAllStandardOutput();
      procOutput.append(luxProc.readAllStandardError());
      return ReLuxRunner::retCode = LR_ABNORMAL_EXIT;
    }
    procOutput = luxProc.readAllStandardOutput();
    return ReLuxRunner::retCode = LR_NORMAL_EXIT;
  }
  return ReLuxRunner::retCode = LR_NORMAL_EXIT;
}


ReLuxRunner::ExecutionResult ReLuxRunner::startSuspendedLuxConsole( const bool useBinDump ) {
  procOutput.clear();
  QStringList args;
  QString cmd = getLuxConsoleProgramName();


  // Make luxconsole extra quiet
  args << "-x";
  // Do we want to read the result as a stream of bytes?
  if (useBinDump) {
    args << "--bindump";
  }
  // Accept input from stdin
  args << "-";

  luxProc.start(cmd,args);
  if ( !luxProc.waitForStarted(2000) ) {
    RE_LOG_DEBUG() << "Timeout in starting luxconsole";
    return ReLuxRunner::retCode = LR_COULD_NOT_START;
  }

  return ReLuxRunner::retCode = LR_NORMAL_EXIT;
}

ReLuxRunner::ExecutionResult ReLuxRunner::activateLuxConsole( const QString& processInput ) {
  luxProc.write(processInput.toUtf8());
  luxProc.closeWriteChannel();

  if ( !luxProc.waitForFinished() ) {
    RE_LOG_DEBUG() << "Timeout in executing luxconsole";
    procOutput = luxProc.readAllStandardOutput();
    procOutput.append(luxProc.readAllStandardError());
    return ReLuxRunner::retCode = LR_ABNORMAL_EXIT;
  }
  procOutput = luxProc.readAllStandardOutput();
  return ReLuxRunner::retCode = LR_NORMAL_EXIT;  
}

void ReLuxRunner::processSLGstdout() {
  procOutput = luxProc.readAllStandardOutput();
  RE_LOG_INFO() << procOutput.data();
  emit SLGStdoutReady(procOutput);
};

/**
 * Executes SLG.
 * The directory is set to where the scene is saved and the scene file name is passed without
 * path so that all the relative references, including the collected textures, are working.
 */

ReLuxRunner::ExecutionResult ReLuxRunner::runSLG( const QString sceneFileName ) {
  QString slgProgramPath = RealityBase::getRealityBase()->getRendererPath(SLG);
  RE_LOG_INFO() << "SLG cmd line: " << slgProgramPath.toStdString();
  QFileInfo slgPathInfo(slgProgramPath);
  if (!slgPathInfo.exists()) {
    return ReLuxRunner::retCode = LR_SLG_NOT_FOUND;
  }

  QFileInfo sceneInfo(sceneFileName);
  RE_LOG_INFO() << "Working dir for SLG:" << sceneInfo.path().toStdString() 
                << " -- scene file: " << sceneInfo.fileName().toStdString();
  QStringList args;
  args << sceneInfo.fileName();
  // Connect to the process
  luxProc.setProcessChannelMode(QProcess::MergedChannels);
  luxProc.disconnect();
  connect(&luxProc,SIGNAL(readyReadStandardOutput()),
          this, SLOT(processSLGstdout()));
  connect(&luxProc,SIGNAL(readyReadStandardError()),
          this, SLOT(processSLGstdout()));
  connect(&luxProc,SIGNAL(finished(int,QProcess::ExitStatus)),
          this, SIGNAL(SLGfinished()));

  luxProc.setWorkingDirectory(sceneInfo.path());

  if (!luxProc.startDetached(slgProgramPath,args, sceneInfo.path())) {
    return ReLuxRunner::retCode = LR_COULD_NOT_START;
  };
  return ReLuxRunner::retCode = LR_NORMAL_EXIT;
}


/**
 * Returns the name of the Lux GUI executable 
 * as used by the current OS.
 */
QString ReLuxRunner::getLuxRenderProgramName() {
  QString appName = "";
  #ifdef WIN32
    appName = "luxrender.exe";
  #elif defined(__APPLE__)
    appName = "LuxRender.app/Contents/MacOS/luxrender";
  #else
    // Not really an option but you never know...
    appName = "luxrender";
  #endif
  return(appName);  
}


/**
 * Searched the path pointed by the user and returns the absolute path of the Lux console executable 
 * as used by the current OS.
 */
QString ReLuxRunner::getLuxConsoleProgramName() {
  QString luxProgramDir = QFileInfo(
                            RealityBase::getRealityBase()->getRendererPath(LuxRender)
                          ).absolutePath();
  #ifdef __APPLE__
    return QString("%1/luxconsole").arg(luxProgramDir);
  #elif defined(WIN32)
    return QString("%1/luxconsole.exe").arg(luxProgramDir);
  #endif
  return "";
}


/**
 * Returns the name of the SLG  executable 
 * as used by the current OS.
 */  
QString ReLuxRunner::getSLGRenderProgramName() {
  QString appName = "";
  #ifdef Q_WS_WIN
    appName = "smallluxgpu2.exe";
  #elif defined(Q_WS_MAC)
    appName = "slg2";
  #else
    // Not really an option but you never know...
    appName = "slg2";
  #endif
  return(appName);    
};

void ReLuxRunner::killProcess() {
  luxProc.kill();
};

