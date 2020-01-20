/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_FRESNEL_TEXTURE_EXPORTER_H
#define LUX_FRESNEL_TEXTURE_EXPORTER_H

#include "ReTools.h"
#include "ReTexture.h"
#include "textures/ReColorMath.h"
#include "textures/ReFresnelColor.h"

#include "ReLuxTextureExporter.h"

namespace Reality {

/*
  Class: ReLuxFresnelExporter
 */

class RE_LIB_ACCESS ReLuxFresnelExporter : public ReLuxTextureExporter {

public:
  const QString exportTexture( ReTexturePtr basetex, const QString& assignedName = "", bool isForPreview = false ) {
    ReFresnelColorPtr tex = basetex.staticCast<ReFresnelColor>();
    QString str = QString("Texture \"%1\" \"fresnel\" \"fresnelcolor\" ")
                    .arg(assignedName != "" ? assignedName : tex->getUniqueName());
    ReTexturePtr fresnelTex = ReLuxTextureExporter::getTextureFromCache(
                                tex->getTexture()
                              );
    QColor clr = tex->getColor();
    if (fresnelTex.isNull()) {
      double r,g,b;
      clr.getRgbF(&r,&g,&b);
      str.append(QString("\"color Kr\" [%1 %2 %3]")
                   .arg(r, 6, 'f')
                   .arg(g, 6, 'f')
                   .arg(b, 6, 'f'));
    }
    else {
      if (!isPureWhite(clr)) {
        // If there is a color and a texture create a texture that multiples those two together
        ReColorMathPtr cmt = ReColorMathPtr(
                               new ReColorMath(
                                 QString("%1_cm_fresnel").arg(fresnelTex->getUniqueName())
                               )
                             );
        cmt->setFunction(ReColorMath::multiply);
        cmt->setTexture1(clr);
        cmt->setTexture2(fresnelTex);

        str.append(QString("\"texture Kr\" [\"%1\"]")
                     .arg(cmt->getUniqueName()));
      }
      else {
        str.append(QString("\"texture Kr\" [\"%1\"]")
                     .arg(fresnelTex->getUniqueName()));
      }
    }
    return str;
  };
};

}
#endif

