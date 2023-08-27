/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_ALPHA_CHANNEL_MAT_H
#define RE_ALPHA_CHANNEL_MAT_H

#include "ReMaterials.h"
#include "reality_lib_export.h"

namespace Reality {

/*
 Class: ReAlphaChannelMaterial

 A material with an added alpha channel
 */ 
class REALITY_LIB_EXPORT ReAlphaChannelMaterial : public ReMaterial {

protected:
  // ReTexturePtr alphaMap;

  float alphaStrength;

public:

  ReAlphaChannelMaterial( const QString name, const ReGeometryObject* parent );

  void setAlphaMap( ReTexturePtr& m );

  ReTexturePtr getAlphaMap() const;

  /*
   Method: getAlphaStrength
   */
  float getAlphaStrength() const;

  /*
   Method: setAlphaStrength
   */
  void setAlphaStrength( float newVal );

  virtual void setNamedValue( const QString& /* name */, const QVariant& /* value */);
  
  virtual const QVariant getNamedValue( const QString& /* name */) const;

  virtual void fromMaterial( const ReMaterial* srcMat );

  //! This method computes the ACSEL ID based on the unique properties
  //! of each material's subclass.
  virtual QString computeAcselID( const bool forDefaultShader = false );

  void serialize( QDataStream& dataStream ) const;

  void deserialize( QDataStream& dataStream );
};

typedef QSharedPointer<ReAlphaChannelMaterial> ReAlphaChannelMaterialPtr;


} // namespace

#endif
