/**
  \file

  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CAMERA_EXPORTER_H
#define QT_CAMERA_EXPORTER_H

#include "ReCamera.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * Exports the camera information to the \ref QVariantMap format
 */

class REALITY_LIB_EXPORT ReQtCameraExporter {
public:
  QVariantMap exportCamera( const ReCameraPtr camera ) {
    QVariantMap camData;
    camData["name"]            = camera->getName();
    camData["id"]              = camera->getID();
    camData["matrix"]          = camera->getMatrix()->toList();
    camData["focalLength"]     = camera->getFocalLength();
    camData["focalDistance"]   = camera->getFocalDistance();
    camData["fStop"]           = camera->getFStop();
    camData["dofEnabled"]      = camera->isDOFEnabled();
    camData["exposureEnabled"] = camera->isExposureEnabled();
    camData["iso"]             = static_cast<int>(camera->getISO());
    camData["shutter"]         = camera->getShutter();
    camData["dofStop"]         = camera->getDOFStop();
    camData["filmResponse"]    = camera->getFilmResponse();
    camData["yon"]             = camera->getYon();
    camData["hither"]          = camera->getHither();
    camData["fov"]             = camera->getFOV();
    return camData;
  };
};

}
#endif
