/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_VOLUME_EXPORTER_H
#define LUX_VOLUME_EXPORTER_H

#include <cmath>
#include <algorithm>
#include <QColor>

#include "reality_lib_export.h"
#include "ReVolumes.h"


namespace Reality {

/*
  Class: ReLuxVolumeExporter
 */

class REALITY_LIB_EXPORT ReLuxVolumeExporter {

public:
  // Constructor: ReLuxVolumeExporter
  ReLuxVolumeExporter() {
  };
  // Destructor: ReLuxVolumeExporter
 ~ReLuxVolumeExporter() {
  };

  //! Calculates the absorption-at-depth based on the RGB values and a depth value
  //! expressed in meters. This formula was derived from the one in LuxBlend.
  void colorAbsorption( qreal& r, qreal& g, qreal& b, const float depth, const float scale ) {
    r = (-std::log(std::max(r, 1e-30)) / depth) * scale * (r == 1.0 ? -1 : 1);
    g = (-std::log(std::max(g, 1e-30)) / depth) * scale * (g == 1.0 ? -1 : 1);
    b = (-std::log(std::max(b, 1e-30)) / depth) * scale * (b == 1.0 ? -1 : 1);
  }

  const QString exportVolume( ReVolumePtr volume ) {
    QString str;
    if (volume.isNull()) {
      return str;
    }
    QString volName = volume->getName();
    // QString colorDepthTex = QString("%1:colorDepth").arg(volName);

    qreal r,g,b;
    QColor clr = volume->getColor();
    // gammaCorrect(clr, RealitySceneData->getGamma());

    clr.getRgbF(&r, &g, &b);
    colorAbsorption(r, g, b, volume->getClarityAtDepth(), volume->getAbsorptionScale());

    // The colordepth texture does not work as expected. We use the old color-at-depth formula instead

    // str += QString(
    //          "Texture \"%1\" \"color\" \"colordepth\" \"color Kt\" [%2 %3 %4] \"float depth\" [%5]\n"
    //        )
    //        .arg(colorDepthTex)
    //        .arg(r)
    //        .arg(g)
    //        .arg(b)
    //        .arg(volume->getClarityAtDepth());

    float cauchyB = volume->getCauchyB();

    if (cauchyB == 0.0) {
      str += QString(
               "MakeNamedVolume \"%1\" \"%2\" \"float fresnel\" [%3] "
             )
             .arg(volName)
             .arg(volume->getTypeAsString())
             .arg(volume->getIOR());
    }
    else {
      QString cauchyTexName = QString("%1_cauchyB").arg(volName);
      str += QString(
               "Texture \"%1\" \"fresnel\" \"cauchy\" "
               "\"float cauchyb\" [%2] "
               "\"float index\" [%3]\n"
             )
             .arg(cauchyTexName)
             .arg(cauchyB)
             .arg(volume->getIOR());

      str += QString(
               "MakeNamedVolume \"%1\" \"%2\" \"texture fresnel\" [\"%3\"] "
             )
             .arg(volName)
             .arg(volume->getTypeAsString())
             .arg(cauchyTexName);
    }
    if (volume->getType() == ReVolume::clear) {
      // str += QString("\"texture absorption\" [\"%1\"] ").arg(colorDepthTex);
      str += QString("\"color absorption\" [%1 %2 %3] ").arg(r).arg(g).arg(b);
    }
    else {
      qreal sr,sg,sb;
      float scatteringScale = volume->getScatteringScale();
      volume->getScatteringColor().getRgbF(&sr, &sg, &sb);

      str += QString(
                // "\"texture sigma_a\" [\"%1\"] "
                "\"color sigma_a\" [%1 %2 %3] "
                "\"color sigma_s\" [%4 %5 %6] "
                "\"color g\" [%7 %8 %9] "
             )
             // .arg(colorDepthTex)
             .arg(r)
             .arg(g)
             .arg(b)
             .arg(sr * scatteringScale)
             .arg(sg * scatteringScale)
             .arg(sb * scatteringScale)
             .arg(volume->getDirectionR())
             .arg(volume->getDirectionG())
             .arg(volume->getDirectionB());
    }
    return str + "\n";
  }
};


} // namespace

#endif
