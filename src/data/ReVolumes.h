/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    

  File: ReVolumes.h
*/

#ifndef VOLUMES_H
#define VOLUMES_H

#include <QtGui>
#include <QDataStream>
#include "ReDefs.h"

namespace Reality {


/**
 * This class defines a volume as used in LuxRender.
 */
class RE_LIB_ACCESS ReVolume {

public:
  enum VolumeType {
    clear,
    homogeneous
  };

protected:
  VolumeType volumeType;

  QString name;

  float IOR;
  //! Variable used to support the dispersion feature of 
  //! glass2. Usually set at zero, it is set by the glass editor
  float cauchyB;

  float clarityAtDepth;
  // Absorption color
  QColor color;
  float absorptionScale;

  bool editable;

  QColor scatteringColor;
  float scatteringScale;

  float directionR,
        directionG,
        directionB;

public:
  // Constructor: ReVolume
  ReVolume() : 
    volumeType(clear),
    IOR(1.0),
    cauchyB(0.0),
    clarityAtDepth(1.0),
    color(248,248,248),
    absorptionScale(1.0),
    editable(true),
    scatteringColor(QColor(0,0,0)),
    scatteringScale(1.0),
    directionR(0.0),
    directionG(0.0),
    directionB(0.0)
  {
  };

  // Constructor: ReVolume
  ReVolume( const QString name ) :
    volumeType(clear),
    name(name),
    IOR(1.0),
    cauchyB(0.0),
    clarityAtDepth(1.0),
    color(248,248,248),
    absorptionScale(1.0),
    editable(true),
    scatteringColor(QColor(0,0,0)),
    scatteringScale(1.0),
    directionR(0.0),
    directionG(0.0),
    directionB(0.0)
  {
  };

  // Destructor: ReVolume
  virtual ~ReVolume() {
  };
  
  inline const QString getName() const {
    return name;
  }

  inline void setName( const QString newName ) {
    name = newName;
  }

  inline float getIOR() const {
    return IOR;
  }

  inline void setIOR( float newVal ) {
    IOR = newVal;
  }

  inline float getCauchyB() const {
    return cauchyB;
  }

  inline void setCauchyB( float newVal ) {
    cauchyB = newVal;
  }

  /*
   Method: getClarityAtDepth
   */
  inline float getClarityAtDepth() const {
    return clarityAtDepth;
  };
  /*
   Method: setClarityAtDepth
   */
  void setClarityAtDepth( float newVal ) {
    clarityAtDepth = newVal;
  };

  /*
   Method: getEditable
   */
  inline bool isEditable() const {
    return editable;
  };
  /*
   Method: setEditable
   */
  void setEditable( bool newVal ) {
    editable = newVal;
  };

  /*
   Method: getColor
   */
  inline const QColor& getColor() const {
    return color;
  };
  /*
   Method: setColor
   */
  inline void setColor( QColor newVal ) {
    color = newVal;
  };

  /*
   Method: getabsorptionScale
   */
  inline float getAbsorptionScale() const {
    return absorptionScale;
  };
  /*
   Method: setabsorptionScale
   */
  inline void setAbsorptionScale( const float newVal ) {
    absorptionScale = newVal;
  };

  /*
   Method: getType
   */
  inline VolumeType getType() const {
    return volumeType;
  };
  /*
   Method: setType
   */
  inline void setType( VolumeType newVal ) {
    volumeType = newVal;
  };
  
  virtual QString getTypeAsString() const {      
    switch(volumeType){
      case clear:
        return "clear";
      case homogeneous:
        return "homogeneous";
    }
    return "clear";
  }

  /*
   Method: getDirectionR
   */
  inline float getDirectionR() const {
    return directionR;
  };
  /*
   Method: setDirectionR
   */
  void setDirectionR( float newVal ) {
    directionR = newVal;
  };
  
  /*
   Method: getDirectionG
   */
  inline float getDirectionG() const {
    return directionG;
  };
  /*
   Method: setDirectionG
   */
  void setDirectionG( float newVal ) {
    directionG = newVal;
  };
  
  /*
   Method: getDirectionB
   */
  inline float getDirectionB() const {
    return directionB;
  };
  /*
   Method: setDirectionB
   */
  inline void setDirectionB( float newVal ) {
    directionB = newVal;
  };
  
  /*
   Method: getScatteringColor
   */
  inline const QColor& getScatteringColor() const {
    return scatteringColor;
  };
  /*
   Method: setScatteringColor
   */
  inline void setScatteringColor( QColor newVal ) {
    scatteringColor = newVal;
  };   

  /*
   Method: getScatteringScale
   */
  inline float getScatteringScale() const {
    return scatteringScale;
  };
  /*
   Method: setScatteringScale
   */
  inline void setScatteringScale( float newVal ) {
    scatteringScale = newVal;
  };

  virtual void serialize( QDataStream& dataStream ) const;

  /**
   The type is not extracted at this point because that piece of data is read by the 
   material deserializer before we call this method.
   */
  virtual void deserialize( QDataStream& dataStream );

};



typedef QSharedPointer<ReVolume> ReVolumePtr;

} // namespace

#endif
