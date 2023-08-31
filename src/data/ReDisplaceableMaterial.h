/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_DISPLACEABLE_MAT_H
#define RE_DISPLACEABLE_MAT_H

#include <QSharedPointer>

#include "reality_lib_export.h"
#include "ReAlphaChannelMaterial.h"


namespace Reality {

#define RE_BM_CHANNEL_NAME "Bm"
#define RE_DM_CHANNEL_NAME "Dm"
#define RE_EMISSION_CHANNEL_NAME "Kl"

/**
 A material that can be displaced with a map. It also supports Bump/Normal maps and other
 geometry transformations.

 This class will be renamed as ReModifiedMaterial because it contains
 information about other properties that are not directly related to
 displacement. For example, it hold the data if the material emits light
 */ 
class REALITY_LIB_EXPORT DisplaceableMaterial : public ReAlphaChannelMaterial {

protected:
  bool isNormalMap;
  float bmNegative;
  float bmPositive;
  float bmStrength;

  float dmNegative;
  float dmPositive;
  float dmStrength;

  quint16 subdivision;
  bool useMicrofacets;
  bool smoothnessFlag;
  bool keepSharpEdgesFlag;

  //! Light emitting properties
  bool emitsLight;
  float lightGain;
public:

  DisplaceableMaterial( const QString name, const ReGeometryObject* parent );
  
  /*
   Method: getBumpMap
   */
  ReTexturePtr getBumpMap() const;
  /*
   Method: setBumpMap
   */
  void setBumpMap( ReTexturePtr& newVal );

  bool hasBumpMap() const;

  //! Copies the settings from another material
  virtual void fromMaterial( const ReMaterial* srcmat );

  /*
   Method: getBmPositive
   */
  float getBmPositive() const;

  /*
   Method: setBmPositive
   */
  void setBmPositive( float newVal );
  
  /*
   Method: getBmNegative
   */
  float getBmNegative() const;
  /*
   Method: setBmNegative
   */
  void setBmNegative( float newVal );
  /*
   Method: getBmStrength
   */
  float getBmStrength() const ;
  /*
   Method: setBmStrength
   */
  void setBmStrength( float newVal );
    
  bool hasNormalMap() const;
  
  void setNormalMapFlag( const bool f );
  
  inline ReTexturePtr getDisplacementMap() const {
    return channels["Dm"];
  };

  void setDisplacementMap( ReTexturePtr newVal );
  
  bool hasDisplacementMap() const;

  inline float getDmPositive() const {
    return dmPositive;
  };
 
  void setDmPositive( float newVal );
  
  inline float getDmNegative() const {
    return dmNegative;
  };

  void setDmNegative( float newVal );
  
  inline float getDmStrength() const {
    return dmStrength;
  };

  void setDmStrength( float newVal );
  
  //! Returns the pointer to the Ambient map, if available
  ReTexturePtr getAmbientMap() const;

  //! Sets an Ambient map for this material
  void setAmbientMap( ReTexturePtr& newVal );

  inline quint16 getSubdivisions() const {
    return subdivision;
  };

  void setSubdivisions( quint16 newVal );
  
  inline bool usesMicrofacets() const {
    return useMicrofacets;
  };

  void setUseMicrofacets( bool newVal );
  
  inline bool isSmooth() const {
    return smoothnessFlag;
  };

  void setIsSmooth( bool newVal );
  
  inline bool keepsSharpEdges() const {
    return keepSharpEdgesFlag;
  };

  void setKeepSharpEdges( bool newVal );

  //! Sets the flag that determines if this material emits light
  void setEmitsLight( const bool newVal );

  //! Returns the fglag that determines if this material emits light
  inline bool isEmittingLight() const {
    return emitsLight;
  };

  //! Sets the light gain for the material
  void setLightGain( const float newVal );

  //! returns the light gain when the material emits light
  inline float getLightGain() const {
    return lightGain;
  };
  
  virtual void setNamedValue( const QString& /* name */, const QVariant& /* value */);
  
  virtual const QVariant getNamedValue( const QString& /* name */) const;

  virtual void serialize( QDataStream& dataStream ) const;

  virtual void deserialize( QDataStream& dataStream );
};

typedef QSharedPointer<DisplaceableMaterial> ReDisplaceableMaterialPtr;

} // namespace

#endif
