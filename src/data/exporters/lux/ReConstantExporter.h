/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_CONSTANT_TEXTURE_EXPORTER_H
#define LUX_CONSTANT_TEXTURE_EXPORTER_H

#include "ReTexture.h"
#include "ReLuxTextureExporter.h"
#include "ReTools.h"  
#include "ReSceneData.h"
  
namespace Reality {

/**
 * LuxRender exporter for the Constant texture
 */
class RE_LIB_ACCESS ReLuxConstantExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr texbase, const QString& assignedName = "", bool isForPreview = false ) {
    double r,g,b;
    ReConstantPtr tex = texbase.staticCast<ReConstant>();
    QColor clr = tex->getColor();
    gammaCorrect(clr, RealitySceneData->getGamma());
    clr.getRgbF(&r,&g,&b);    
    QString str;

    QString chStr = findChannel(texbase);
    if (!chStr.isEmpty()) {
      str = QString("# From channel %1\n").arg(chStr);
    }
    switch(tex->getDataType()) {
      case ReTexture::color: {
        str += QString("Texture \"%1\" \"color\" \"constant\" \"color value\" [%2 %3 %4] ")
                 .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
                 .arg(r,4,'f')
                 .arg(g,4,'f')
                 .arg(b,4,'f');
        break;
      }
      case ReTexture::numeric: 
      case ReTexture::fresnel: {
        // Since this is a numeric texture it's supposed to be a gray color.
        // Any channel of it will be equivalent
        str += QString("Texture \"%1\" \"%2\" \"constant\" \"float value\" [%3] ")
                 .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
                 .arg(tex->getDataTypeAsString())
                 .arg(r,4,'f');
        break;
      }
    }
    return str;
  };
};

}
#endif
