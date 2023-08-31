/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_COLORMATH_IMPORTER_H
#define QT_COLORMATH_IMPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReQtTextureImporter.h"
#include "textures/ReColorMath.h"


namespace Reality {

/*
  Class: ReQtColorMathExporter
 */

class REALITY_LIB_EXPORT ReQtColorMathImporter : public ReQtTextureImporter {

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
