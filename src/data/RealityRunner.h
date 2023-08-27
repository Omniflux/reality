/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_REALITY_RUNNER_H
#define RE_REALITY_RUNNER_H

#include <QProcess>
#include <QSharedPointer>
#include "ReIPC.h"
#include "reality_lib_export.h"

#define REALITY_MAC "Reality.app/Contents/MacOS/Reality"
#define REALITY_W   "Reality.exe"
#define RE_RUNNER_POLL_INTERVAL 500

namespace Reality {

/**
 * Executes the Reality GUI and keeps track if it's already running
 */
class REALITY_LIB_EXPORT RealityRunner {

private:
  QSharedPointer<QProcess> realityProc;
  static RealityRunner* instance;
  bool guiStarted;

public:
  // Constructor: RealityRunner
  RealityRunner() {
    guiStarted = false;
  };
  // Destructor: RealityRunner
 ~RealityRunner() {
    if (instance) {
      delete instance;      
    }
  };

  static RealityRunner* getInstance();
  bool launchGUI( const QString directory );
};

} // namespace


#endif
