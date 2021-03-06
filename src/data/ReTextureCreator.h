/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef TEXTURECREATOR_H
#define TEXTURECREATOR_H

#include "ReTexture.h"
#include "ReTextureContainer.h"
#include <QDataStream>
#include <QTextStream>

namespace Reality {

/**
 * A class factory used to de-serialize <ReTexture> classes from a stream.
 */
class RE_LIB_ACCESS TextureCreator {

public:

  static ReTexture* createTexture( const QString texName, 
                                   const ReTextureType texType,
                                   ReTextureContainer* parentMat,
                                   const ReTexture::ReTextureDataType dataType = ReTexture::color );

  //! Creates a new texture based on another one. It uses the copy ctor
  static ReTexture* createTexture( const QString texName, 
                                   const ReTexturePtr baseTex );
  /**
   * Creates a texture from a QVariantMap. Used to restore a texture from the data
   * saved in the host-app scene file.
   */
  static ReTexturePtr createTexture( const QVariantMap& data, ReTextureContainer* parentMat );
  
  static ReTexturePtr deserialize( QDataStream& dataStream, ReTextureContainer* parentMat );

};

} // namespace

#endif
