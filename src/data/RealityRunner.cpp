/**
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityRunner.h"

#include <QProcess>

#include "RealityBase.h"
#include "ReIPC.h"
#include "ReLogger.h"


namespace Reality {

// Static variables initialization
RealityRunner* RealityRunner::instance = NULL;

RealityRunner* RealityRunner::getInstance() {
  if (!instance) {
    instance = new RealityRunner();
  }
  return instance;
}

/**
 * Method: launchGUI
 *   Runs the Reality GUI from the directory passed in the 
 *   first parameter.
 *
 * Returns:
 *  True if the program is running, false if there was problem in executing 
 *  the program.
 */
bool RealityRunner::launchGUI( const QString directory ) {
  RealityBase* rbase = RealityBase::getRealityBase();
  Reality::OSType osType = rbase->getOSType();
  QString hostVersion = rbase->getHostVersion();

  QString programPath;
  if (osType == Reality::MAC_OS) {
    programPath = QString("%1/%2").arg(directory).arg(REALITY_MAC);
  }
  else if (osType == Reality::WINDOWS) {
    programPath = QString("%1/%2").arg(directory).arg(REALITY_W);
  }
  QStringList args;
  args << QString("--hostVersion=%1").arg(hostVersion);
  if (realityProc.isNull()) {
    realityProc = QSharedPointer<QProcess>( new QProcess() );
  }
  time_t currentTime;
  time(&currentTime);

  // We determine that the GUI is running if it pinged us no later than a second ago
  bool isGUIRunning = (currentTime - realityIPC->getLastGUIPing()) < 2;

  if ((realityProc->state() == QProcess::NotRunning) || !isGUIRunning) {
    // If guiStarted is true then we are looking at the pointer
    // to a process that previously crashed. This means that we need to
    // cleanup and create a new process object.
    if (guiStarted) {
      realityProc->close();
      realityProc.clear();
      realityProc = QSharedPointer<QProcess>( new QProcess() );
      guiStarted = false;
    }
    realityProc->start(programPath, args);
    if (!realityProc->waitForStarted()) {
      RE_LOG_DEBUG() << "Error in starting the Reality GUI at " << QSS(programPath);
      return false;
    }
    return guiStarted = true;
  }
  else {
    realityIPC->bringGUIToForeGround();
    return true;
  }
  return false;
}

}
