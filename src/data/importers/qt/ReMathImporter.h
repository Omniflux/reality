/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_MATH_IMPORTER_H
#define QT_MATH_IMPORTER_H

#include "textures/ReMath.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/*
  Class: ReQtMathExporter
 */

class RE_LIB_ACCESS ReQtMathImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReMathPtr tex = baseTex.staticCast<ReMath>();
    restoreBasicProperties(tex, data);

    tex->setAmount1(data.value("amount1").toFloat());
    tex->setAmount2(data.value("amount2").toFloat());
    tex->setFunction(static_cast<ReMath::functions>(data.value("function").toInt()));
    restoreChannels(tex, data);
  };
};

}
#endif
