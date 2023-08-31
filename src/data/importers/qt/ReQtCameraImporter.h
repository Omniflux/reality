/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_CAMERA_IMPORTER_H
#define RE_QT_CAMERA_IMPORTER_H

#include <QVariantMap>

#include "ReCamera.h"


namespace Reality {

/*
  Class: ReQtCameraImporter
 */

class ReQtCameraImporter {

public:
  // Constructor: ReQtCameraImporter
  ReQtCameraImporter() {
  };
  // Destructor: ReQtCameraImporter
 ~ReQtCameraImporter() {
  };

  ReCameraPtr importCamera( const QVariantMap& data ) {
    QString name,
            camID;

    name = data.value("name").toString();
    camID = data.value("id").toString();
    ReCameraPtr camera = ReCameraPtr( new ReCamera( name, camID ) );
    camera->setMatrix( data.value( "matrix" ).toList() );
    camera->setFocalLength( data.value( "focalLength" ).toFloat() );
    camera->setFocalDistance( data.value( "focalDistance" ).toFloat() );
    camera->setFStop( data.value( "fStop" ).toFloat() );
    camera->setDOFEnabled( data.value( "dofEnabled" ).toBool() );
    camera->setExposureEnabled( data.value( "exposureEnabled" ).toBool() );
    camera->setISO( data.value( "iso" ).toFloat() );
    camera->setShutter( data.value( "shutter" ).toFloat() );
    camera->setDOFStop( data.value( "dofStop" ).toFloat() );
    camera->setFilmResponse( data.value( "filmResponse" ).toString() );
    camera->setYon( data.value( "yon" ).toFloat() );
    camera->setHither( data.value( "hither" ).toFloat() );
    camera->setFOV( data.value("fov", 45).toFloat() );
    return camera;
  }
};


} // namespace

#endif
