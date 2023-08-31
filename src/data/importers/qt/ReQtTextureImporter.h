/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_TEXTURE_IMPORTER_H
#define QT_TEXTURE_IMPORTER_H

#include "ReTexture.h"
#include "ReTextureContainer.h"
#include "textures/Re2DTexture.h"
#include "textures/ReComplexTexture.h"


namespace Reality {

/**
 * Base class for all Qt texture importers.
 */
class ReQtTextureImporter {
public:
  virtual ~ReQtTextureImporter() {

  }
  
  void restoreBasicProperties( ReTexturePtr tex, const QVariantMap& data ) {
    tex->setTextureDataType( 
      static_cast<ReTexture::ReTextureDataType>(data.value("dataType").toInt())
    );
  }

  /**
   * 
   */
  void restoreChannels( ReTexturePtr tex, const QVariantMap& data ) {
    QVariantMap chList;
    QMapIterator<QString, QVariant> ci(data.value("channels").toMap());  
    while( ci.hasNext() ) {
      ci.next();      
      QString chTexName = ci.value().toString();
      ReTexturePtr chTex = ReTexturePtr();
      // A blank channel can exist and it is represented by an empty string
      if (chTexName != "") {
        chTex = tex->getParent()->getTexture(chTexName);
        ReComplexTexturePtr cTex = tex.dynamicCast<ReComplexTexture>();
        if (!cTex.isNull()) {
          cTex->setChannel(ci.key(), chTex);
        }
      }
    }
  }

  void restore3DMapping( ReTexture3DPtr tex, const QVariantMap& data ) {
    tex->setMapping( static_cast<ReTexture3D::Mapping3D>(data.value("mapping").toInt()) );
    tex->setScale( data.value("scale").toFloat() );
    tex->setRotation( data.value("xRot").toFloat(), 
                      data.value("xRot").toFloat(), 
                      data.value("xRot").toFloat() );
  }

  void restore2DMapping( Re2DTexturePtr tex, const QVariantMap& data ) {
    tex->setUTile( data.value("uTile").toFloat() );
    tex->setVTile( data.value("vTile").toFloat() );
    tex->setUOffset( data.value("uOffset").toFloat() );
    tex->setVOffset( data.value("vOffset").toFloat() );
  }

  virtual void restoreTexture( ReTexturePtr tex, const QVariantMap& data ) = 0;
};

typedef QSharedPointer<ReQtTextureImporter> ReQtTextureImporterPtr;

} // namespace

#endif
