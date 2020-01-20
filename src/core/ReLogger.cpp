/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReLogger.h"
#include "ReDefs.h"  

//! The log file that we use to keep track of important message from the library
ofstream logFile;
OstreamLogger RealityLogger(logFile);

QString CustomLogMessage::prefix = "";
