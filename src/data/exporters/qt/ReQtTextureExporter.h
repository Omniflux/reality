/**
  \file
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_TEXTURE_EXPORTER_H
#define QT_TEXTURE_EXPORTER_H

#include "textures/Re2DTexture.h"
#include "textures/ReComplexTexture.h"

/**
 * Base class for all Qt texture exporters. It defines a series of 
 * common functions needed by all the subclasses
 */

namespace Reality {

class RE_LIB_ACCESS ReQtTextureExporter {

protected:
  QVariantMap texData;

public:
  virtual ~ReQtTextureExporter() {
  }
  
  void storeBasicProperties( const ReTexture* tex ) {
    texData["name"] = tex->getName();
    texData["type"] = tex->getType();
    texData["dataType"] = tex->getDataType();    
  }

  /**
   * Method: serializeChannels
   * Copies the texture channels into the map for JSON export
   */
  void exportChannels( const ReTexture* baseTex ) {
    QVariantMap chList;
    const ReComplexTexture* cTex = dynamic_cast<const ReComplexTexture*>(baseTex);
    if (!cTex) {
      return;
    }
    ReNodeDictionaryIterator ic(cTex->getChannels());  
    while(ic.hasNext()) {
      ic.next();      
      ReTexturePtr tex = ic.value();
      chList[ic.key()] = (tex.isNull() ? "" : tex->getName());
    }
    texData["channels"] = chList;
  }

  void export3DMapping( const ReTexture3D* tex3D ) {
    float xr, yr, zr;
    tex3D->getRotation(xr, yr, zr );
    texData["mapping"] = tex3D->getMapping();
    texData["scale"]   = tex3D->getScale();
    texData["xRot"]    = (double)xr;
    texData["yRot"]    = (double)yr;
    texData["zRot"]    = (double)zr;
  }

  void export2DMapping( const Re2DTexture* tex2D ) {
    texData["uTile"]   = (double)tex2D->getUTile();
    texData["vTile"]   = (double)tex2D->getVTile();
    texData["uOffset"] = (double)tex2D->getUOffset();
    texData["vOffset"] = (double)tex2D->getVOffset();
  }

  //! Export the texture to a QVariantMap. 
  //! 
  //! \param tex The input texture that we are converting
  //! \return A \ref QVariantMap reference that contains the data from the texture.
  virtual QVariantMap exportTexture( const ReTexture* tex ) = 0;
};

typedef QSharedPointer<ReQtTextureExporter> ReQtTextureExporterPtr;

} // namespace

#endif
