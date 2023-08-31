/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RECLOTH_H
#define RECLOTH_H

#include "reality_lib_export.h"
#include "ReDisplaceableMaterial.h"


#define WARP_KD "warp_Kd"
#define WARP_KS "warp_Ks"
#define WEFT_KD "weft_Kd"
#define WEFT_KS "weft_Ks"

#define RE_CLOTH_NUM_PRESETS 6

namespace Reality {

/**
 * This class holds the data for the fabric presets
 */
class REALITY_LIB_EXPORT ReClothPreset {
public:
  QString name;
  QString label;
  QColor warp_Kd,
         warp_Ks,
         weft_Kd,
         weft_Ks;

  float uRepeat,
        vRepeat;

  // Constructor: ReClothPreset
  ReClothPreset( const QString name,
                 const QString label,
                 const QColor warp_Kd,
                 const QColor warp_Ks,
                 const QColor weft_Kd, 
                 const QColor weft_Ks,
                 const float uRepeat,
                 const float vRepeat ) :
     name(name),
     label(label),
     warp_Kd(warp_Kd),
     warp_Ks(warp_Ks),
     weft_Kd(weft_Kd),
     weft_Ks(weft_Ks),
     uRepeat(uRepeat),
     vRepeat(vRepeat)
  {
  };
  
  //! Returns a preset based on the index in the table of available
  //! presets 
  static ReClothPreset* getPreset( unsigned short index );
};

#define RE_CLOTH_DENIM            0
#define RE_CLOTH_SILK_CHARMEUSE   1
#define RE_CLOTH_SILK_SHANTUNG    2
#define RE_CLOTH_COTTON_TWILL     3
#define RE_CLOTH_WOOL_GABARDINE   4
#define RE_CLOTH_POLYESTER_LINING 5

/**
 Implementation of the Lux Cloth material
 */
class REALITY_LIB_EXPORT ReCloth: public ReModifiedMaterial {

private:
  QString presetName;
  float uRepeat,
        vRepeat;

public:

  static ReClothPreset clothPresets[RE_CLOTH_NUM_PRESETS];

  // Constructor: ReCloth
  explicit ReCloth( const QString name, const ReGeometryObject* parent );

  void fromMaterial( const ReMaterial* bm );

  // Destructor: ReCloth
 ~ReCloth();

  /**
   Method: getURepeat
   */
  inline float getURepeat() const {
    return uRepeat;
  };

  /**
   Method: setURepeat
   */
  void setURepeat( const float newVal ) {
    uRepeat = newVal;
  };

  /*
   Returns the name of the currently selected preset
   */
  const QString getPresetName() const;

  /*
   Sets the preset only,without loading the parameters for the preset.
   */
  void setPresetName( const QString& newVal );

  /**
   Returns the index corresponding to the current preset name  
   */
  int getPresetNumber();

  /**
   * Switches to a new preset and loads all the values
   */
  void setPreset( const int presetIndex );


  /**
   */
  inline float getVRepeat() const {
    return uRepeat;
  };
  /**
    Sets the vRepeat value
   */
  inline void setVRepeat( float newVal ) {
    uRepeat = newVal;
  };

  /*
   Method: getWarp_Kd
   */
  const ReTexturePtr getWarp_Kd() const;

  /*
   Method: setWarp_Kd
   */
  void setWarp_Kd( ReTexturePtr newVal );

  /*
   Method: getWarp_Ks
   */
  const ReTexturePtr getWarp_Ks() const;

  /*
   Method: setWarp_Ks
   */
  void setWarp_Ks( ReTexturePtr newVal );

  /*
   Method: getWeft_Kd
   */
  const ReTexturePtr getWeft_Kd() const;

  /*
   Method: setWeft_Kd
   */
  void setWeft_Kd( ReTexturePtr newVal );

  /*
   Method: getWeft_Ks
   */
  const ReTexturePtr getWeft_Ks() const;

  /*
   Method: setWarp_Ks
   */
  void setWeft_Ks( ReTexturePtr newVal );

  
  /**
   Sets the properties values using a parameter for the name
   */
  void setNamedValue( const QString& vname, const QVariant& value );
  /**
   Get the value of a property by name
   */
  const QVariant getNamedValue( const QString& vname ) const;

  /**
   This is used to transmit the material between the host-side and GUI-side
   parts of Reality.
  */
  void serialize( QDataStream& dataStream ) const;

  /**
   Rebuilds the material from a data stream. Used to transmit the material between the 
   host-side and GUI-side parts of Reality.
   */
  void deserialize( QDataStream& dataStream );
  
};

typedef QSharedPointer<ReCloth> ReClothPtr;


} // namespace

#endif
