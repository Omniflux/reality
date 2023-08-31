/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REWATER_H
#define REWATER_H

#include "reality_lib_export.h"
#include "ReModifiedMaterial.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"

namespace Reality {
  class ReVolume;
  typedef QSharedPointer<ReVolume> ReVolumePtr;
};


#define WM_NUM_RIPPLE_PRESETS 6

namespace Reality {

class REALITY_LIB_EXPORT ReWater: public ReModifiedMaterial {
public:

protected:
  float ripples;
  // The selected ripple preset
  int ripplePreset;

  //! Create the volume necessary for this material, if needed.
  void createVolume();

private:
  
  inline ReVolumePtr getInnerVolume() const {
    if (innerVolume != "") {
      return RealitySceneData->getVolume(innerVolume);
    }
    return ReVolumePtr();
  };

  static QVariantMap ripplePresets[WM_NUM_RIPPLE_PRESETS];
  // Used to driver the initialization of the preset table
  static bool presetsInitialized;
  // Symbolic names for the preset
  static QString ripplePresetsNames[WM_NUM_RIPPLE_PRESETS];

public:

  explicit ReWater( const QString name, const ReGeometryObject* parent );

  inline int getNumPresets() const {
    return WM_NUM_RIPPLE_PRESETS;
  }

  //! Obtains one ripple preset from the catalog of available presets.
  //! The catalog is a an array of QVariantMap with each entry organized as 
  //! follows:
  //!   "noiseBasis" => <QVariant:Procedural texture ID>
  //!   "distortionType" => <QVariant:Procedural texture ID>
  inline const QVariantMap& getPreset( const int presetNum ) const {
    return ripplePresets[presetNum];
  }


  inline QString& getPresetName( const int presetIndex ) {
    return ripplePresetsNames[presetIndex];
  }

  inline int getPresetNumber() const {
    return ripplePreset;
  }

  inline void setPresetNumber( const int pn ) {
    ripplePreset = pn;    
  }

  /**
   * Returns the Transmission color.
   */
  QColor getKt() const;

  /**
   * Sets the Transmission color for the water.
   */
  void setKt( QColor newVal );

  /*
   Method: getClarityAtDepth
   */
  float getClarityAtDepth() const;

  /*
   Method: setClarityAtDepth
   */
  void setClarityAtDepth( float newVal );

  /**
   Handling of the IOR is simply a proxy for the underlying volume
   */
  float getIOR() const;

  /*
   Method: setIOR
   */
  void setIOR( float newVal );

  /*
   Method: getRipples
   */
  inline float getRipples() const {
    return ripples;
  };

  /*
   Method: setRipples
   */
  void setRipples( float newVal );

  //! Convert a material to Water
  void fromMaterial( const ReMaterial* bm );

  void setNamedValue( const QString& name, const QVariant& value );

  const QVariant getNamedValue( const QString& name ) const;

  void serialize( QDataStream& dataStream ) const;

  void deserialize( QDataStream& dataStream );
};

typedef QSharedPointer<ReWater> ReWaterPtr;


} // namespace

#endif
