/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_COLORMATH_IMPORTER_H
#define QT_COLORMATH_IMPORTER_H

#include "textures/ReColorMath.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/*
  Class: ReQtColorMathExporter
 */

class RE_LIB_ACCESS ReQtColorMathImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReColorMathPtr tex = baseTex.staticCast<ReColorMath>();
    restoreBasicProperties(tex, data);

    tex->setColor1(data.value("color1").toString());
    tex->setColor2(data.value("color2").toString());
    tex->setFunction(static_cast<ReColorMath::functions>(data.value("function").toInt()));
    restoreChannels(tex, data);
  };
};

}
#endif
