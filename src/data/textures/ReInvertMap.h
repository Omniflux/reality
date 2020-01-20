/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef __RE_INVERTMAP_H__
#define __RE_INVERTMAP_H__

#include "Re2DTexture.h"

namespace Reality {

/*
  Class: InvertMap

  A texture that inverts an image map. Implemented by using the Mix texture,
  pluggin the texture into the amount and setting the first mix value to 1 and
  the second mix value to 0.

  See also <ReTexture>.

 */

class RE_LIB_ACCESS InvertMap : public Re2DTexture {

public:
  // Constructor: InvertMap
  InvertMap(const QString name, ReTextureContainer* parentMat = 0) : 
    Re2DTexture(name, parentMat) 
  {
    type = TexInvertMap;
  };

  // Texture conversion ctor
  InvertMap( const ReTexturePtr srcTex ) :
    Re2DTexture(srcTex)
  {
    type = TexInvertMap;
  }  

  // Destructor: InvertMap
 ~InvertMap() {
  };

  // const QString getStringAvatar( const quint16  avatarNumber = 1 ) {
  //   return "InvertMap";
  // };

  virtual QString toString() {
    return "InvertMap texture";
  };

  
};

// Typedef: InvertMapPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<InvertMap> InvertMapPtr;


} // namespace


#endif
