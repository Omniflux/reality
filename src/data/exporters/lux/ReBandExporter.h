/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_BAND_TEXTURE_EXPORTER_H
#define LUX_BAND_TEXTURE_EXPORTER_H

#include "textures/ReBand.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * This class implements support for LuxRender band texture, a color
 * gradient.
 */
class REALITY_LIB_EXPORT ReLuxBandExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texBase, 
                               const QString& assignedName = "", 
                               bool isForPreview = false ) 
  {
    ReBandPtr tex = texBase.staticCast<ReBand>();
    if (!tex) {
      return "";
    }
    QString str;
    QString chStr = findChannel(texBase);
    if (!chStr.isEmpty()) {
      str = QString("# From channel %1\n").arg(chStr);
    }

    str += QString("Texture \"%1\" \"%2\" \"band\"\n")
             .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
             .arg(tex->getDataTypeAsString());

    int numBands = tex->getNumBands();
    QString offsets;
    for (int i = 0; i < numBands; i++) {
      ReTexturePtr bandTex = ReLuxTextureExporter::getTextureFromCache(
                               tex->getNamedValue( 
                                 QString("tex%1").arg(i+1) 
                               ).value<ReTexturePtr>()
                             );
      if (bandTex.isNull()) {
        RE_LOG_INFO() << "Texture #" << i+1 << " for band texture " << tex->getName() << " is null";
        continue;
      }
      str += QString(
               "  \"texture tex%1\" [\"%2\"]\n"
             )
             .arg(i+1)
             .arg( bandTex->getUniqueName() );
      offsets += QString::number( 
                   tex->getNamedValue(QString("offset%1").arg(i+1) ).toFloat(),
                   'f',
                   3 
                 ) + " ";
    }
    str += QString("  \"float offsets\" [%1]\n").arg(offsets);
    ReTexturePtr mapTex = ReLuxTextureExporter::getTextureFromCache(
                            tex->getNamedValue("map").value<ReTexturePtr>()
                          );
    if (mapTex.isNull()) {
      str += QString("  \"float amount\" [%1]\n").arg(tex->getAmount());
    }
    else {
      str += QString("   \"texture amount\" [\"%1\"]\n").arg(mapTex->getUniqueName());
    }
    return str;  
  };
};

} //namespace

#endif
