/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_CAMERA_H
#define RE_CAMERA_H

#include <QList>
#include <QVariantMap>
#include <QSharedPointer>
#include <QDataStream>

#include "ReDefs.h"
#include "ReMatrix.h"
#include "RealityBase.h"
#include "math.h"
#include <algorithm>

using namespace std;

namespace Reality {

/**
 * Definition of the camera used in Reality.
 * Cameras in Reality store the distance values in meters and
 * the coordinate system uses the Y axis as the up-axis.
 * It's the responsibility of the plugins in the host app to 
 * provide the data in the right format to ReCamera.
 */
class ReCamera;
typedef QSharedPointer<ReCamera> ReCameraPtr;

class RE_LIB_ACCESS ReCamera {

private:
  QString name;
  QString ID;

  ReMatrix matrix;

  float focalLength;
  float focalDistance;
  //! Used by DAZ Studio but not by Poser
  float fov;

  float fStop;

  bool dofEnabled;
  bool exposureEnabled;
  quint32 iso;
  float shutter;
  float dofStop;
  QString filmResponse;

  float yon;

  float hither;
  //! This parameter is only valid for Poser cameras. It's currently
  //! not used and not saved to file. It's scheduled for future uses
  float scale;

  //! This variable is used by the toString() method to return the 
  //! camera data as a string efficiently. By returning a reference
  //! to this string we avoid the overhead required by allocating a
  //! new string.

  QString toStringValue;

  void init() {    
    focalLength     = 50.0; // mm
    focalDistance   = 1.0;  // meter
    fov             = 45.0; // degrees
    exposureEnabled = false;
    dofEnabled      = false;
    iso             = 100;
    shutter         = 0.008f; // 1/125"
    fStop           = 4.0; 
    dofStop         = 2.8f;
    filmResponse    = "NONE";
    // Expressed in meters
    hither          = 0.5;
    yon             = 500;
    scale           = 1.0;
  }

  double getPoserFOV(const uint frameWidth, 
                     const uint frameHeight) const;
public:
  // Constructor: ReCamera
  ReCamera( const QString& name, const QString& ID) : 
    name(name),
    ID(ID) 
  {
    init();
  };

  // Constructor: ReCamera
  ReCamera( const QString& name, const QString& ID, const QVariantMap& values) : 
    name(name),
    ID(ID) 
  {
    init();
    setValues(values);
  };

  // Destructor: ReCamera
  ~ReCamera() {
  };

  void setValues( const QVariantMap values ) {
    matrix.setValues(values["matrix"].toList());
    focalLength   = values["focalLength"].toFloat();
    focalDistance = values["focalDistance"].toFloat();
    hither        = values["hither"].toFloat();
    yon           = values["yon"].toFloat();
    scale         = values["scale"].toFloat();        
  }

  void fromHostAppCamera( ReCameraPtr c2 ) {
    matrix = c2->matrix;
    focalLength = c2->focalLength;
    focalDistance = c2->focalDistance;
    hither = c2->hither;
    yon = c2->yon;
  }

  QString& toString();

  double getFOV(const uint frameWidth, const uint frameHeight) const;

  inline ReMatrix* getMatrix() {
    return &matrix;
  }

  inline void setMatrix( const QVariantList& tm ) {
    matrix.setValues(tm);
  }

  inline void setMatrix( const ReMatrix tm ) {
    matrix = tm;
  }

  inline float getFocalLength() const {
    return(focalLength);
  };

  inline void setFocalLength( const float newVal )  {
    focalLength = newVal;
  };
  /*
   Method: getFocalDistance
   */
  inline float getFocalDistance() const {
    return focalDistance;
  };

  /*
   Method: setFocalDistance
   */
  inline void setFocalDistance( float newVal ) {
    focalDistance = newVal;
  };

  inline float getFStop() const {
    return(fStop);
  };

  inline void setFStop( const float newVal ) {
    fStop = newVal;
  };

  inline float getDOFStop() const {
    return(dofStop);
  };

  inline void setDOFStop( const float newVal ) {
    dofStop = newVal;
  };

  /*
   Method: getISO
   */
  inline quint32 getISO() const {
    return iso;
  };

  /*
   Method: setISO
   */
  inline void setISO( quint32 newVal ) {
    iso = newVal;
  };

  /*
   Method: getShutter
   */
  inline float getShutter() const {
    return shutter;
  };

  /*
   Method: setIso
   */
  inline void setShutter( float newVal ) {
    shutter = newVal;
  };

  /*
   Method: isExposureEnabled
   */
  inline bool isExposureEnabled() const {
    return exposureEnabled;
  };
  /*
   Method: setExposureEnabled
   */
  inline void setExposureEnabled( bool newVal ) {
    exposureEnabled = newVal;
  };

  /*
   Method: isDOFEnabled
   */
  inline bool isDOFEnabled() const {
    return dofEnabled;
  };
  /*
   Method: setDOFEnabled
   */
  inline void setDOFEnabled( bool newVal ) {
    dofEnabled = newVal;
  };

  /*
   Method: getFilmresponse
   */
  inline const QString getFilmResponse() const {
    return filmResponse;
  };

  /*
   Method: setFilmResponse
   */
  inline void setFilmResponse( QString newVal ) {
    filmResponse = newVal;
  };

  /*
   Method: getHither
   */
  inline float getHither() const {
    return hither;
  };
  /*
   Method: setHither
   */
  inline void setHither( float newVal ) {
    hither = newVal;
  };

  /*
   Method: getYon
   */
  inline float getYon() const {
    return yon;
  };
  /*
   Method: setYon
   */
  inline void setYon( float newVal ) {
    yon = newVal;
  };

  /*
   Method: getName
   */
  inline const QString getName() const {
    return name;
  };
  /*
   Method: setName
   */
  inline void setName( QString newVal ) {
    name = newVal;
  };
  
  /*
   Method: getID
   */
  inline const QString getID() const {
    return ID;
  };
  /*
   Method: setID
   */
  inline void setID( QString newVal ) {
    ID = newVal;
  };

  inline float getFOV() const{
    return fov;
  }

  inline void setFOV( const float v ) {
    fov = v;
  }

  void setNamedValue( const QString& valueName, const QVariant& value);

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

};

} // namespace


#endif
