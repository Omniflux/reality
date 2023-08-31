/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef MIX_MATERIAL_H
#define MIX_MATERIAL_H

#include "reality_lib_export.h"
#include "ReMaterial.h"


namespace Reality {

/*
  Class: ReMix
 */

class REALITY_LIB_EXPORT ReMix : public ReMaterial {
private:

  // ReTexturePtr mixTexture;

  // Variable: mixAmount
  //   The amount of mixing. This is used if there is no "mix" texture
  //   available in the <channels>. Otherwise that texture is used to
  //   define the level of mixing on a per-pixel-basis
  float mixAmount;

  ReMaterialPtr mat1, mat2;

  // Variable: ownsMaterials
  //   This variable indicates if the mix material owns the 
  //   embedded materials or not. If ownership is achieved
  //   then the material is responsible for exporting the 
  //   definition of the materials to the renderer. Otherwise
  //   it's assumed that the materials have been exported by
  //   whatever object owns them.
  //   By default it's true.
  bool ownsMaterials;

public:

  // Constructor: ReMix
  ReMix( const QString name, const ReGeometryObject* parent );

  // Destructor: ReMix
  ~ReMix() {
  };

  void setMaterial1( ReMaterialPtr mat);
  inline ReMaterialPtr getMaterial1() const {
    return mat1;
  };

  void setMaterial2( ReMaterialPtr mat);
  inline ReMaterialPtr getMaterial2() const {
    return mat2;    
  };

  void setMaterials( ReMaterialPtr mat1, ReMaterialPtr mat2);
  void setMixTexture( ReTexturePtr tex );
  inline ReTexturePtr getMixTexture() const {
    return channels["mix"];
  };

  inline bool hasMixTexture() const {
    return !channels["mix"].isNull();
  };

  void setMixAmount( const float mixAmount);

  inline float getMixAmount() const {
    return mixAmount;
  };

  void serialize( QDataStream& dataStream ) const;

  void deserialize( QDataStream& dataStream );

  /*
   Method: hasOwnership
   */
  inline bool hasOwnership() const {
    return ownsMaterials;
  };
  /*
   Method: setOwnsMaterials
   */
  void setOwnership( bool newVal ) {
    ownsMaterials = newVal;
  };
  
};

typedef QSharedPointer<ReMix> ReMixPtr;

} // namespace

#endif
