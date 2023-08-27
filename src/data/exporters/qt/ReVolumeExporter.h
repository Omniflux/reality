/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_VOLUME_EXPORTER_H
#define QT_VOLUME_EXPORTER_H

#include "ReVolumes.h"
#include "ReMaterial.h"
#include "reality_lib_export.h"

namespace Reality {

class REALITY_LIB_EXPORT ReQtVolumeExporter  {

public:
  const QVariantMap exportVolume( const ReVolumePtr volume ) {
    QVariantMap volData;
    volData["volumeType"]      = volume->getType();
    volData["name"]            = volume->getName();
    volData["IOR"]             = volume->getIOR();
    volData["clarityAtDepth"]  = volume->getClarityAtDepth();
    volData["color"]           = volume->getColor().name();
    volData["editable"]        = volume->isEditable();
    volData["scatteringColor"] = volume->getScatteringColor().name();
    volData["absorptionScale"] = volume->getAbsorptionScale();
    volData["scatteringScale"] = volume->getScatteringScale();
    volData["directionR"]      = volume->getDirectionR();
    volData["directionG"]      = volume->getDirectionG();
    volData["directionB"]      = volume->getDirectionB();
    volData["cauchyB"]         = volume->getCauchyB();
    return volData;
  };
};

}
#endif
