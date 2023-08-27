/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REGLASS_H
#define REGLASS_H

#include "ReMaterial.h"
#include "ReDisplaceableMaterial.h"
#include <QSharedPointer>
#include "reality_lib_export.h"

#define RE_GLASS_DEFAULT_CAUCHYB 0.00420
// Default value, in meters, of clarity at depth
#define RE_GLASS_DEFAULT_CLARITY 0.2

namespace Reality {

/*
 Class: ReGlass
 */
class REALITY_LIB_EXPORT ReGlass: public DisplaceableMaterial {

public:
  enum GlassType {
    StandardGlass,
    VolumetricGlass,
    ArchitecturalGlass,
    FrostedGlass
  };

protected:
  GlassType glassType;
  float uRoughness, vRoughness;
  float thinFilmIOR;
  float thinFilmThickness;
  float IOR;
  //! Flag that controls if dispersion is enabled
  bool dispersion;
  //! The Cauchy B coefficient if dispersion is enabled
  float cauchyB;

  //! Stores the name of the preset that corresponds to 
  //! the selected IOR, if any.
  QString IORLabel;

  QVariant val;

public:

    explicit ReGlass( const QString name, const ReGeometryObject* parent );

    virtual void fromMaterial( const ReMaterial* bm );

    //! Overloaded from base class and used to synchronized the volume
    //! data for glass2-type glass
    void textureUpdated( const QString& textureName );
    
    /*
     Method: getKt
     */
    inline ReTexturePtr getKt() const {
      return channels["Kt"];
    };
    /*
     Method: setKt
     */
    void setKt( ReTexturePtr newVal );

    /*
     Method: getKr
     */
    inline ReTexturePtr getKr() const {
      return channels["Kr"];      
    };

    /*
     Method: setKr
     */
    void setKr( ReTexturePtr newVal );

    /*
     Method: getIOR
     */
    inline float getIOR() const {
      return IOR;
    };
    /*
     Method: setIOR
     */
    inline void setIOR( float newVal ) {
      IOR = newVal;
    };

    /*
     Method: getIORLabel
     */
    inline QString getIORLabel() const {
      return IORLabel;
    };
    /*
     Method: setIORLabel
     */
    inline void setIORLabel( QString newVal ) {
      IORLabel = newVal;
    };

    /*
     Method: getThickness
     */
    float getThinFilmThickness() const;
    /*
     Method: setThickness
     */
    void setThinFilmThickness( float newVal );
        
    /*
     Method: getThinFilmIOR
     */
    float getThinFilmIOR() const;
    /*
     Method: setThinFilmIOR
     */
    void setThinFilmIOR( float newVal );
    
    /*
     Method: getGlassType
     */
    GlassType getGlassType() const;
    /*
     Method: setGlassType
     */
    void setGlassType( GlassType newVal );

    /**
     Method: getURoughness
     */
    inline float getURoughness() const {
      return uRoughness;
    };

    /*
     Method: setURoughness
     */
    inline void setURoughness( float newVal ) {
      uRoughness = newVal;
    };

    /**
     Method: getCauchyB
     */
    inline float getCauchyB() const {
      return cauchyB;
    };

    /*
     Method: setCauchyB
     */
    inline void setCauchyB( float newVal ) {
      cauchyB = newVal;
    };

    /**
     Method: hasDispersions
     */
    inline float hasDispersion() const {
      return dispersion;
    };

    /*
     Method: setDispersion
     */
    inline void setDispersion( float newVal ) {
      dispersion = newVal;
    };

    /*
     Method: getVRoughness
     */
    inline float getVRoughness() const {
      return vRoughness;
    };

    /*
     Method: setURoughness
     */
    inline void setVRoughness( float newVal ) {
      vRoughness = newVal;
    };
    /*
     Method: serialize      
    */
    virtual void serialize( QDataStream& dataStream ) const;

    virtual void deserialize( QDataStream& dataStream );

    void setNamedValue( const QString& name, const QVariant& value );

    const QVariant getNamedValue( const QString& name ) const;

    // OBSOLETE, replaced by correct version in ReAlphaChannelMaterial
    // //! This method computes the ACSEL ID based on the unique properties
    // //! of each material's subclass.
    // QString computeAcselID( const bool forDefaultShader = false );

};

typedef QSharedPointer<ReGlass> ReGlassPtr;

} // namespace

#endif
