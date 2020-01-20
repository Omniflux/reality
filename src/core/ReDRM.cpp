/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReDRM.h"
#include "ReUserData.h"
#include "RealityBase.h"
#include "RegistrationDialog.h"
#include "ReLogger.h"
#include "blowfish.h"

bool RealityIsRegistered = false;

using namespace Reality;

static char FOR_CRACKERS[] = "Hello Hackers. So you are now looking at my disassembled code "
                      "trying to crack it. You will find the task not too hard, I don't "
                      "waste my time trying to make my code hacker-proof. We both know "
                      "that it's not possible. But, I must ask you, do you really have "
                      "to crack something like Reality that sells for less than $50.00 "
                      "and has some of the best support in the world? I'm a single developer, "
                      "do you really want to damage some of your own? What's the point? "
                      "I'm not this faceless, powerful corporation. I'm just a regular guy "
                      "trying to make a buck with my software. So, if you really must, "
                      "crack and steal my product, which I wrote in long, unpaid, months. "
                      "But one thing I ask you to consider: please don't give the crack away. "
                      "That will do me enourmous damage. Bragging is such a childish emotion "
                      "and I would like to think that we, as intelligent human beings, are "
                      "above that. Use Reality on your own and let others crack it on their "
                      "own. Please consider buying a regular license. It's cheap and you will "
                      "help a fellow programmer. Thank you kindly. Paolo.";


char* getMessageForHackers() {
  QString str(UPDATER_URL_DECOY);
  return FOR_CRACKERS;
}

ReLuxManager::ReLuxManager( QSplashScreen* ss ) : splashScreen(ss) {
  // Calling this ctor will also call the anti-temper layer of the base class.

  // Reset the condition...
  RealityIsRegistered = false;
  verifyLuxVersion();
}

void ReLuxManager::verifyLuxVersion() {
  // Check if the registration data is already stored...
  ReConfigurationPtr config = RealityBase::getRealityBase()->getConfiguration();

  config->beginGroup(decryptString(RE_REGISTRATION_ROOT));
  QString productKey;
  switch (RealityBase::getRealityBase()->getHostAppID()) {
    case Poser:
      productKey = config->value(decryptString(RE_REGISTRATION_KEY_PS)).toString();     
      break;
    case DAZStudio:
      productKey = config->value(decryptString(RE_REGISTRATION_KEY_DS)).toString();    
      break;
  }
  config->endGroup();
  // Reset the condition...
  RealityIsRegistered = false;
  // If not, then call the registration system
  if (productKey.size() == 0) {
    splashScreen->hide();
    RegistrationDialog* dialog = new RegistrationDialog();
    dialog->setVerifier(this);
    dialog->setSizeGripEnabled(false);

    // Retval is actually not used, just more obfuscation
    int retval = dialog->exec();
  }
  else {
    QByteArray encData(QByteArray::fromBase64(productKey.toAscii()));

    Blowfish cipher;
    QByteArray clearKey(decryptString(registrationKey).toAscii());
    
    cipher.Set_Passwd( clearKey.data() );    
    cipher.Decrypt(((unsigned char*) encData.data()), encData.size());
    // if (encData.contains("activated")) {
    if (encData.contains(decryptString(encDataSuffix).toAscii())) {
      RealityIsRegistered = true;
      clear();
    };
  }  
};


