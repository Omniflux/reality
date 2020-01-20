/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReDefs.h"
#include <QByteArray>

// The real name for this function cannot be use because it will be exporter
// in the dylib/DLL, making it easy for hackers to identify what the function
// does. A meaningless name is therefore used.
RE_LIB_ACCESS int getIconResource();
