/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RELIGHTMATERIAL_H
#define RELIGHTMATERIAL_H

#include "ReMaterial.h"
#include "ReLightStand.h"
#include <QSharedPointer>
  
namespace Reality {

/*
  Class: ReLightMaterial
 */

class RE_LIB_ACCESS ReLightMaterial : public ReMaterial, public ReLightStand {

private:
  ReLightPtr light;
  QString lightID;
  //! When we convert a material to be a light we need to save the 
  //! original material type so that we can convert back.
  ReMaterialType originalType;

public:
  // Constructor: ReLightMaterial
  ReLightMaterial( const QString name, const ReGeometryObject* parent );

  // Destructor: ReLightMaterial
  ~ReLightMaterial();

  ReLightPtr getLight();

  inline const QString getLightID() const {
    return lightID;
  };

  inline void setLightID( const QString newVal ) {
    lightID = newVal;
  }

  inline void setLight( const ReLightPtr newLight ) {
    light = newLight;
  }

  inline void setOriginalType( const ReMaterialType oType ) {
    originalType = oType;
  }

  ReMaterialType getOriginalType() {
    return originalType;
  }

  void serialize( QDataStream& dataStream ) const;

  void deserialize( QDataStream& dataStream );

  //! Conversion from other material
  void fromMaterial( const ReMaterial* baseMat );
};

typedef QSharedPointer<ReLightMaterial> ReLightMaterialPtr;
} // namespace


#endif
