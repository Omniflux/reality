/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef REGUID_H
#define REGUID_H

#include <QtCore>
#include "dznode.h"

namespace Reality {

/**
  A class to get/set a GUID to a DAZ Studio node
 */
class ReGUID {
private:
  static QString GUID_value;

public:
  //! Constructor: ReGUID
  ReGUID() {
  };

  //! Destructor: ReGUID
 ~ReGUID() {
  };

  static QString& getGUID( const DzNode* node );
  static QString& getGUID();

  static QString& setGUID( DzNode* node );

  static const char* InvalidGUID; // = "NO_GUID";
};

} // namespace

#endif
