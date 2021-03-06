/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef DRM_SERVICES_H
#define DRM_SERVICES_H

#include <QtCore>
#include "ReDefs.h"

RE_LIB_ACCESS quint32 packVersionNumber( const QString vnum );

RE_LIB_ACCESS QString decryptString(const unsigned char* encStr);

RE_LIB_ACCESS void removeRegistration();

/**
 * Returns a string encoded with the sha1 encryption scheme.
 */
RE_LIB_ACCESS QString sha1( const QString input );

typedef QMap<QString,QString> ReStringMap;

/**
 * Class: ReUserData
 */

class RE_LIB_ACCESS ReUserData {

public:
  //! Constructor: ReUserData
  ReUserData() {
  };

  //! Destructor: ReUserData
 ~ReUserData() {
  };

  static ReStringMap getUserData();  
};

// Original string: RE4PS
// Encrypted string generated with ./encryptStrings.pl
static unsigned char REALITY_PROD_CODE_PS[6] = {
  0xa2,   0x94,   0x82,   0x9d,   0x9f,
  0x00
};

// Original string: RE4DS
// Encrypted string generated with ./encryptStrings.pl
static unsigned char REALITY_PROD_CODE_DS[6] = {
  0xa2,   0x94,   0x82,   0x91,   0x9f,
  0x00
};

// Original string: RE4SA
// Encrypted string generated with ./encryptStrings.pl
static unsigned char REALITY_PROD_CODE_PRO[6] = {
  0xa2,   0x94,   0x82,   0xa0,   0x8d,
  0x00
};

// This is the group where we store the serial number
// Original string: RenderOptions
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_REGISTRATION_ROOT[14] = {
  0xa2,   0xb4,   0xbc,   0xb1,   0xb1,   0xbd,   0x99,   0xb9,   0xbc,   0xb0,   0xb5, 
  0xb3,   0xb7, 
  0x00
};

// Original string: RenderCfgDS
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_REGISTRATION_KEY_DS[12] = {
  0xa2,   0xb4,   0xbc,   0xb1,   0xb1,   0xbd,   0x8d,   0xaf,   0xaf,   0x8b,   0x99,
  0x00
};

// Original string: RenderCfgPS
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_REGISTRATION_KEY_PS[12] = {
  0xa2,   0xb4,   0xbc,   0xb1,   0xb1,   0xbd,   0x8d,   0xaf,   0xaf,   0x97,   0x99,
  0x00
};

//! Serial number validation RegExp 
//! Original string: ^R4(PS|DS|SA|PU|DU)[A-Z,0-9]{33}$
//! Encrypted string generated with ./encryptStrings.pl
static unsigned char SN_VALIDATOR[34] = {
  0xae,   0xa1,   0x82,   0x75,   0x9c,   0x9e,   0xc6,   0x8d,   0x9b,   0xc3,   0x99,
  0x86,   0xc0,   0x93,   0x97,   0xbd,   0x84,   0x94,   0x67,   0x98,   0x7d,   0x68,
  0x94,   0x65,   0x68,   0x64,   0x6f,   0x92,   0xaf,   0x66,   0x65,   0xae,   0x54,
  0x00
};

// Original string: firstName=%1&lastName=%2&orderNo=%3&serialNo=%4&email=%5&productCode=%6&callerID=%7&sendEmail=%8&OS=%9
// Encrypted string generated with ./encryptStrings.pl
static unsigned char VALIDATION_SERVER_STRING[103] = {
  0xb6,   0xb8,   0xc0,   0xc0,   0xc0,   0x99,   0xab,   0xb6,   0xad,   0x84,   0x6b, 
  0x76,   0x6a,   0xaf,   0xa3,   0xb4,   0xb4,   0x8d,   0x9f,   0xaa,   0xa1,   0x78, 
  0x5f,   0x6b,   0x5e,   0xa6,   0xa8,   0x99,   0x99,   0xa5,   0x80,   0xa0,   0x6d, 
  0x54,   0x61,   0x53,   0x9f,   0x90,   0x9c,   0x92,   0x89,   0x93,   0x74,   0x94, 
  0x61,   0x48,   0x56,   0x47,   0x85,   0x8c,   0x7f,   0x86,   0x88,   0x58,   0x3f, 
  0x4e,   0x3e,   0x87,   0x88,   0x84,   0x78,   0x88,   0x75,   0x85,   0x53,   0x7e, 
  0x72,   0x72,   0x49,   0x30,   0x40,   0x2f,   0x6b,   0x68,   0x72,   0x71,   0x69, 
  0x75,   0x4b,   0x45,   0x3d,   0x24,   0x35,   0x23,   0x6f,   0x60,   0x68,   0x5d, 
  0x3d,   0x64,   0x57,   0x5e,   0x60,   0x30,   0x17,   0x29,   0x16,   0x3e,   0x41, 
  0x2a,   0x11,   0x24, 
  0x00
};

// Original string: oldSN=%1
// Used to append the old serial number when processing an upgrade
// Encrypted string generated with ./encryptStrings.pl
static unsigned char PREVIOUS_SN_QUERY[9] = {
  0xbf,   0xbb,   0xb2,   0xa0,   0x9a,   0x88,   0x6f,   0x7a,
  0x00
};

// Original string: MiPdAOtLr6CbKu9x
// Encrypted string generated with ./encryptStrings.pl
static unsigned char registrationKey[17] = {
  0x9d,   0xb8,   0x9e,   0xb1,   0x8d,   0x9a,   0xbe,   0x95,   0xba,   0x7d,   0x89, 
  0xa7,   0x8f,   0xb8,   0x7b,   0xb9, 
  0x00
};

// Original string: Blu3-VeLvEt/
// Encrypted string generated with ./encryptStrings.pl
static unsigned char ACSEL_PASSCODE[13] = {
  0x92,   0xbb,   0xc3,   0x80,   0x79,   0xa1,   0xaf,   0x95,   0xbe,   0x8c,   0xba, 
  0x74, 
  0x00
};

// Original string: http://www.preta3d.com/%1/%2
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_REGISTRATION_URL[29] = {
  0xb8,   0xc3,   0xc2,   0xbd,   0x86,   0x7a,   0x79,   0xc0,   0xbf,   0xbe,   0x74, 
  0xb5,   0xb6,   0xa8,   0xb6,   0xa2,   0x73,   0xa3,   0x6c,   0xa0,   0xab,   0xa8, 
  0x69,   0x5e,   0x69,   0x66,   0x5b,   0x67, 
  0x00
};

// Original string: tasks
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_REGISTRATION_TASKS_DIR[6] = {
  0xc4,   0xb0,   0xc1,   0xb8,   0xbf, 
  0x00
};

// Original string: ProductRegistration.php
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_REGISTRATION_SCRIPT_NAME[24] = {
  0xa0,   0xc1,   0xbd,   0xb1,   0xc1,   0xae,   0xbe,   0x9b,   0xad,   0xae,   0xaf,
  0xb8,   0xb8,   0xb5,   0xa3,   0xb5,   0xa9,   0xae,   0xac,   0x6b,   0xac,   0xa3,
  0xaa,
  0x00
};

// Original string: REAPP
// Encrypted string generated with ./encryptStrings.pl
static unsigned char APP_CODE_NAME[6] = {
  0xa2,   0x94,   0x8f,   0x9d,   0x9c, 
  0x00
};


// Original string: activated
// Encrypted string generated with encryptStrings.pl
static unsigned char encDataSuffix[10] = {
  0xb1,  0xb2,  0xc2,  0xb6,  0xc2,  0xac,  0xbe,  0xae,  0xac,  0x0
};

// Original string: -!+
// Encrypted string generated with encryptStrings.pl
static unsigned char preUserInfo[4] = {
  0x7d,  0x70,  0x79,  0x0
};

// Original string: %!*
// Encrypted string generated with encryptStrings.pl
static unsigned char postUserInfo[4] = {
  0x75,  0x70,  0x78,  0x0
};

// Original string: The license key received or entered is not correct. Please double check it and try again. Reality was not enabled.
// Encrypted string generated with ./encryptStrings.pl
static unsigned char ERR_WRONG_LICENSE[115] = {
  0xa4,  0xb7,  0xb3,  0x6d,  0xb8,  0xb4,  0xad,  0xae,
  0xb6,  0xba,  0xab,  0x65,  0xaf,  0xa8,  0xbb,  0x61,
  0xb2,  0xa4,  0xa1,  0xa2,  0xa5,  0xb1,  0x9f,  0x9d,
  0x58,  0xa6,  0xa8,  0x55,  0x99,  0xa1,  0xa6,  0x96,
  0xa2,  0x94,  0x92,  0x4d,  0x95,  0x9e,  0x4a,  0x97,
  0x97,  0x9b,  0x46,  0x88,  0x93,  0x95,  0x94,  0x86,
  0x83,  0x93,  0x4c,  0x3d,  0x6c,  0x87,  0x7f,  0x7a,
  0x8b,  0x7c,  0x36,  0x79,  0x83,  0x88,  0x74,  0x7d,
  0x75,  0x2f,  0x71,  0x75,  0x71,  0x6e,  0x75,  0x29,
  0x71,  0x7b,  0x26,  0x66,  0x72,  0x67,  0x22,  0x75,
  0x72,  0x78,  0x1e,  0x5e,  0x63,  0x5c,  0x63,  0x67,
  0x26,  0x17,  0x48,  0x5a,  0x55,  0x5f,  0x5b,  0x65,
  0x69,  0xf,   0x65,  0x4e,  0x5f,  0xb,   0x58,  0x58,
  0x5c,  0x7,   0x4b,  0x53,  0x45,  0x45,  0x4e,  0x46,
  0x44,  0xd,   0x0
};

// Original string: OK
// Encrypted string generated with ./encryptStrings.pl
static unsigned char IS_OK[3] = {
  0x9f,  0x9a,  0x00
};

// Original string: N/A
// Encrypted string generated with ./encryptStrings.pl
static unsigned char N_A[4] = {
  0x9e,  0x7e,  0x8f,  0x00
};

// Original string: http://preta3d.com/tasks/CheckProductUpdates.php
// Encrypted string generated with ./encryptStrings.pl
static unsigned char UPDATER_URL[49] = {
  0xb8,   0xc3,   0xc2,   0xbd,   0x86,   0x7a,   0x79,   0xb9,   0xba,   0xac,   0xba,
  0xa6,   0x77,   0xa7,   0x70,   0xa4,   0xaf,   0xac,   0x6d,   0xb1,   0x9d,   0xae,
  0xa5,   0xac,   0x67,   0x7a,   0x9e,   0x9a,   0x97,   0x9e,   0x82,   0xa3,   0x9f,
  0x93,   0xa3,   0x90,   0xa0,   0x80,   0x9a,   0x8d,   0x89,   0x9b,   0x8b,   0x98,
  0x52,   0x93,   0x8a,   0x91,
  0x00
};

// Original string: application/x-www-form-urlencoded
// Encrypted string generated with drm/encryptStrings.pl
static unsigned char HTTP_POST_HEADER[34] = {
  0xb1,   0xbf,   0xbe,   0xb9,   0xb5,   0xae,   0xab,   0xbd,   0xb1,   0xb6,   0xb4,
  0x74,   0xbc,   0x70,   0xb9,   0xb8,   0xb7,   0x6c,   0xa4,   0xac,   0xae,   0xa8,
  0x67,   0xae,   0xaa,   0xa3,   0x9b,   0xa3,   0x97,   0xa2,   0x96,   0x96,   0x94,
  0x00
};

// Original string: productCode
// Encrypted string generated with ./encryptStrings.pl
static unsigned char FLD_PROD_CODE[12] = {
  0xc0,   0xc1,   0xbd,   0xb1,   0xc1,   0xae,   0xbe,   0x8c,   0xb7,   0xab,   0xab,
  0x00
};

// Original string: buildNo
// Encrypted string generated with ./encryptStrings.pl
static unsigned char FLD_BUILD_NO[8] = {
  0xb2,   0xc4,   0xb7,   0xb9,   0xb0,   0x99,   0xb9, 
  0x00
};

// Original string: OS
// Encrypted string generated with ./encryptStrings.pl
static unsigned char FLD_OS[3] = {
  0x9f,   0xa2,
  0x00
};

// Original string: hostApp
// Encrypted string generated with drm/encryptStrings.pl
static unsigned char FLD_HOST_APP[8] = {
  0xb8,   0xbe,   0xc1,   0xc1,   0x8d,   0xbb,   0xba,
  0x00
};

// Original string: hostVersion
// Encrypted string generated with drm/encryptStrings.pl
static unsigned char FLD_HOST_VERSION[12] = {
  0xb8,   0xbe,   0xc1,   0xc1,   0xa2,   0xb0,   0xbc,   0xbc,   0xb1,   0xb6,   0xb4,
  0x00
};

// Original string: description
// Encrypted string generated with ./encryptStrings.pl
static unsigned char FLD_DESCRIPTION[12] = {
  0xb4,   0xb4,   0xc1,   0xb0,   0xbe,   0xb4,   0xba,   0xbd,   0xb1,   0xb6,   0xb4, 
  0x00
};

// Original string: isFree
// Encrypted string generated with ./encryptStrings.pl
static unsigned char FLD_IS_FREE[7] = {
  0xb9,   0xc2,   0x94,   0xbf,   0xb1,   0xb0, 
  0x00
};

// Original string: status
// Encrypted string generated with ./encryptStrings.pl
static unsigned char FLD_STATUS[7] = {
  0xc3,   0xc3,   0xaf,   0xc1,   0xc1,   0xbe, 
  0x00
};

// Original string: url
// Encrypted string generated with ./encryptStrings.pl
static unsigned char FLD_URL[4] = {
  0xc5,   0xc1,   0xba, 
  0x00
};

// Original string: #!#
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_LICENSE_STATUS_BAD[4] = {
  0x73,   0x70,   0x71, 
  0x00
};

// Original string: email
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_USER_DATA_EMAIL[6] = {
  0xb5,   0xbc,   0xaf,   0xb6,   0xb8, 
  0x00
};

// Original string: firstName
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_USER_DATA_FIRST_NAME[10] = {
  0xb6,   0xb8,   0xc0,   0xc0,   0xc0,   0x99,   0xab,   0xb6,   0xad, 
  0x00
};

// Original string: lastName
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_USER_DATA_LAST_NAME[9] = {
  0xbc,   0xb0,   0xc1,   0xc1,   0x9a,   0xac,   0xb7,   0xae, 
  0x00
};

// Original string: orderNo
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_USER_DATA_ORDER_NO[8] = {
  0xbf,   0xc1,   0xb2,   0xb2,   0xbe,   0x99,   0xb9, 
  0x00
};

// Original string: serialNo
// Encrypted string generated with ./encryptStrings.pl
static unsigned char RE_USER_DATA_SERIAL_NO[9] = {
  0xc3,   0xb4,   0xc0,   0xb6,   0xad,   0xb7,   0x98,   0xb8, 
  0x00
};

#endif
