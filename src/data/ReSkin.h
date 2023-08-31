
/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RESKIN_H
#define RESKIN_H

#include "reality_lib_export.h"
#include "ReGeometryObject.h"
#include "ReGlossy.h"
#include "ReTools.h"


#define RE_SKIN_ABSORTION_COLOR  QColor(249,140,135)
#define RE_SKIN_ABSORTION_SCALE  8.0f
#define RE_SKIN_SCATTERING_COLOR QColor(255,137,97)
#define RE_SKIN_SCATTERING_SCALE 15.0f
#define RE_SKIN_FRESNEL_AMOUNT   0.05f
// expressed in millimeters
#define RE_SKIN_CLARITY_AT_DEPTH 0.0025   
#define RE_SKIN_IOR              1.3f
#define RE_SKIN_DIRECTION_RGB    -0.9f

// The Hair mask channel
#define RE_SKIN_KHM "Khm"
#define RE_SKIN_VOL_TEMPLATE "%1:SSSVol"
namespace Reality {

/**
 The skin material.
 */
class REALITY_LIB_EXPORT ReSkin: public ReGlossy {

protected:
  //! The amount of the fresnel component of this material
  float fresnelAmount;

  //! Whether SSS is enabled for this material
  bool sssEnabled;

  //! If enabled then the exporter will need to create an hair mask when using SSS
  //! to make the hair, usually for the face, more defined.
  bool hairMask;

  //! Gain for the hair mask
  float hmGain;

  void init();
  
public:

    explicit ReSkin( const QString name, const ReGeometryObject* parent );

    virtual void fromMaterial( const ReMaterial* bm );

    inline bool isSSSEnabled() const {
      return sssEnabled;  
    }

    inline void setSSSEnabled( bool onOff ) {
      sssEnabled = onOff;
      // Set or clear the inner volume based on whether the SSS is on or off
      if (onOff) {
        setInnerVolume(QString(RE_SKIN_VOL_TEMPLATE).arg(parent->getInternalName()));
      }
      else {
        setInnerVolume("");
      }
    }

    inline bool hasHairMask() const {
      return hairMask;  
    }

    void setHairMask( bool onOff );

    //! Retrieves the texture used for the hair mask
    inline const ReTexturePtr getKhm() const {
      return channels[RE_SKIN_KHM];
    };

    //! Sets the texture used for the hair mask
    inline void setKhm( ReTexturePtr newVal ) {
      channels[RE_SKIN_KHM] = newVal;
    };
    
    /*
     Method: getHmGain
     */
    inline float getHmGain() const {
      return hmGain;
    };

    /*
     Method: setHmGain
     */
    inline void setHmGain( float newVal ) {
      hmGain = newVal;
    };


    /**
     * Creates a volume, initializes it with the correct defaults for Skin
     * and then links the volume to this material.
     */
    void createSSSVolume();
    
    /**
     * Searches all the textures in the Kd channel and returns the 
     * file name of the image map for the channel. If there is no
     * image map the it returns an empty string. This is used to 
     * retrieve the base texture for the automatic hair mask.
     */
    inline QString getKhmImageMap() const {
      if (!channels[RE_SKIN_KHM].isNull()) {
        return searchImageMap(channels[RE_SKIN_KHM]);
      }
      return "";
    }
    
    /*
     Method: getFresnelAmount
     */
    inline float getFresnelAmount() const {
      return fresnelAmount;
    };
    /*
     Method: setFresnelAmount
     */
    void setFresnelAmount( float newVal ) {
      fresnelAmount = newVal;
    };
                  
    /*
     Method: serialize      
    */
    void serialize( QDataStream& dataStream ) const;

    void deserialize( QDataStream& dataStream );

    void setNamedValue( const QString& name, const QVariant& value );

    const QVariant getNamedValue( const QString& name ) const;

    QString toString();
};

typedef QSharedPointer<ReSkin> ReSkinPtr;

} // namespace

#endif
