/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REDRM_H
#define REDRM_H


#ifdef __APPLE__
// Stop the compiler from complaining about unused variables
#pragma GCC diagnostic ignored "-Wall"
#elif defined(_MSC_VER)
  // Disable endless warning about truncation with VC++
  #pragma warning (disable:4305)
  #pragma warning (disable:4309)
#endif
  
#include <QtCore>
#include <QSplashScreen>

// This is retired and only used as a decoy
#define UPDATER_URL_DECOY "http://www.preta3d.com/tasks/checkRealityUpdates.php"

//! This function is only to make the "FOR_HACKERS" string stick in the executable. 
//! Otherwise the optimizer will strip it.
char* getMessageForHackers();


/**
 * Base classe for the product registration class. This layer holds the
 * anti-tamper layer so that the code is separate from the Product Registration class
 * Just a quick way of obfuscating things in the executable.
 */
class ReRenderManager {

private:
  bool isCleared;

public:
  // ctor
  ReRenderManager() {
    // This is set to "dirty" and hopefully cleared only by the verification of the serial number
    // If the hacker bypass the external part of the code the flag will not be cleared.
    // Just one more layer of safety
    // qDebug() << "Anti-tamper layer...";
    isCleared = false;
  };

  /**
   * This is called by the Registration Dialog or the code that verifies the serial number installed
   * If this flag is cleared then we know that no crucial parts have been bypassed. It's weak but 
   * better than nothing.
   */
  void clear() {
    isCleared = true;
  }
  bool verify() {
    return(isCleared);
  };
};

/**
 * This is the Product Registration checker. The name has been changed to obfuscate it agains
 * scans of the catalog in the binary file. The Qt Meta-Object holds too much information in 
 * clear and that makes it trivial for a hacker to identify what this class does.
 */

class ReLuxManager: public ReRenderManager {

private:
  QSplashScreen* splashScreen;
  
public:
  // ctor
  ReLuxManager( QSplashScreen* ss );

  // This method should be called licenseCheck(). This is were the verification is done
  void verifyLuxVersion();
};

extern bool RealityIsRegistered;

#endif
