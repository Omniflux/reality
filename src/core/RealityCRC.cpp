/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

/**
 * This file has the only purpose to define a variable that contains the CRC
 * value for the Reality executable. The value is computed after the executable
 * is compiled and it is stored in the dylib/DLL library to which Reality is 
 * linked to. 
 *
 * The order of actions is:
 *   - Compile the Reality library with this source and the CRC area at 0
 *   - The Reality executable is compiled and it links to 
 *     Reality_3.(dylib|DLL). At this point the executable would abort at 
 *     startup because Reality_3.(dylib|DLL) contains the wrong value for the 
 *      CRC
 *   - The build system uses the compute_crc program to compute the correct
 *     CRC value for the newly compiled executable and replace the value where
 *     the zeros are
 *   - Happyness ensues
 */

#include "RealityCRC.h"



#define CRC_BUFFER_SIZE 16
#define CRC_VALUE_OFFSET 9
// The 0000 portion will be replaced with the binary value of the CRC.
const char* RE_iconResource = "Reality![00000000]!";

// Returns the buffer initialized with the data that contains the CRC
RE_LIB_ACCESS int getIconResource() {
  int crcValue;  
  memcpy(&crcValue, RE_iconResource+CRC_VALUE_OFFSET, sizeof(crcValue));
  return crcValue;
}
