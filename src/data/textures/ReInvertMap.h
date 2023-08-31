/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef __RE_INVERTMAP_H__
#define __RE_INVERTMAP_H__

#include "reality_lib_export.h"
#include "textures/Re2DTexture.h"


namespace Reality {

/*
  Class: ReInvertMap

  A texture that inverts an image map. Implemented by using the Mix texture,
  plugging the texture into the amount and setting the first mix value to 1 and
  the second mix value to 0.

  See also <ReTexture>.

 */

class REALITY_LIB_EXPORT ReInvertMap : public Re2DTexture {

public:
  // Constructor: ReInvertMap
  ReInvertMap(const QString name, ReTextureContainer* parentMat = 0) :
    Re2DTexture(name, parentMat) 
  {
    type = TexInvertMap;
  };

  // Texture conversion ctor
  ReInvertMap( const ReTexturePtr srcTex ) :
    Re2DTexture(srcTex)
  {
    type = TexInvertMap;
  }  

  // Destructor: ReInvertMap
 ~ReInvertMap() {
  };

  // const QString getStringAvatar( const quint16  avatarNumber = 1 ) {
  //   return "InvertMap";
  // };

  virtual QString toString() {
    return "ReInvertMap texture";
  };

  
};

// Typedef: ReInvertMapPtr
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReInvertMap> ReInvertMapPtr;


} // namespace


#endif
