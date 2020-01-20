/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReUserData.h"
#include "blowfish.h"
#include "RealityBase.h"

using namespace Reality;

ReStringMap __userData;

/**
 * Convert a version number in the format v.sub.patch.build to a 32-bit unsigned integer
 */
quint32 packVersionNumber( const QString vnum ) {
  QStringList numbers = vnum.split(".");
  if (numbers.count() != 4) {
    return(0);
  }
  qint32 v;
  v = numbers[0].toInt()  << 24;
  v += numbers[1].toInt() << 16;
  v += numbers[2].toInt() <<  8;
  v += numbers[3].toInt();
  return(v);
};

QString sha1( const QString input ) { 
  return(QCryptographicHash::hash(input.toAscii(), QCryptographicHash::Sha1).toHex()); 
};
  
// Returns a string decrypted 
QString decryptString(const unsigned char* encStr) {
  QString newStr = "";
  int i=0;
  while(*encStr != 0x0) {
    newStr += (char)((int)*encStr - 80 + i);
    i++;
    encStr++;
  }
  return newStr;
};

//! Removes the registration data 
void removeRegistration() {
  ReConfigurationPtr config = RealityBase::getRealityBase()->getConfiguration();

  config->beginGroup(decryptString(RE_REGISTRATION_ROOT));
  switch(RealityBase::getRealityBase()->getHostAppID()) {
    case Poser:
      config->remove(decryptString(RE_REGISTRATION_KEY_PS));
      break;    
    case DAZStudio:
      config->remove(decryptString(RE_REGISTRATION_KEY_DS));
      break;
    
  }
  config->endGroup();
  // config->remove(decryptString(RE_REGISTRATION_ROOT));
}

ReStringMap ReUserData::getUserData() {
  __userData.clear();

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

  if (productKey != "") {
    QByteArray encData(QByteArray::fromBase64(productKey.toAscii()));
    Blowfish cipher;
    QByteArray clearKey(decryptString(registrationKey).toAscii());

    cipher.Set_Passwd( clearKey.data() );    
    cipher.Decrypt(((unsigned char*) encData.data()), encData.size());

    QStringList fields = QString::fromUtf8(encData.data()).split(",");
    __userData[decryptString(RE_USER_DATA_EMAIL)]      = fields[0];
    __userData[decryptString(RE_USER_DATA_FIRST_NAME)] = fields[1];
    __userData[decryptString(RE_USER_DATA_LAST_NAME)]  = fields[2];
    __userData[decryptString(RE_USER_DATA_ORDER_NO)]   = fields[3];
    __userData[decryptString(RE_USER_DATA_SERIAL_NO)]  = fields[4];
  };
  return(__userData);
}
