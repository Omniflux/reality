/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MIX_TEXTURE_H
#define RE_MIX_TEXTURE_H

#include "ReTexture.h"
#include "ReTextureCreator.h"
#include "textures/ReImageMap.h"
#include "textures/ReComplexTexture.h"  
#include "ReTextureContainer.h"

namespace Reality {

/**
  The name of this texture doesn't follow the usual rules, it has "texture" 
  appended to it, because there is also a Mix material and we need to 
  tell them apart easily.

  See also <ReTexture>.
 */
class RE_LIB_ACCESS ReMixTexture : public ReTexture,
                                   public ReComplexTexture
{

private:
  float mixAmount;
  QStringList deps;

  //! The LuxRender Mix texture allows to use simple float values for
  //! tex1 and tex2. Currently Reality does not support this feature
  //! in the UI but we have partial support in here in order to 
  //! be able to create temporary textures that use that feature.
  //! The float values are not serialized/deserialized or saved
  //! in the scene file, at this time.
  float fTex1;
  float fTex2;

  bool useFloatValuesFlag;
public:

  //! Constructor
  ReMixTexture( const QString name, 
                ReTextureContainer* parentMat = 0,
                const ReTextureDataType dataType = color );

  //! Texture conversion ctor
  ReMixTexture( const ReTexturePtr srcTex );

  // Destructor: ReMixTexture
  ~ReMixTexture();

  //! Initializes the three channels for this texture: tex1, tex2,
  //! and mixtex.
  //! \param baseTex Used to pass a pointer to a mix texture that needs
  //!                to be replicated. If the parameter is not null
  //!                its subtextures are duplicated and connected to the
  //!                channels of this texture.
  void initChannels( const ReTexturePtr baseTex = ReTexturePtr() );

  // const QString getStringAvatar( const quint16 avatarNumber = 0) ;
  void reparent( ReTextureContainer* parentMat );

  void setTextureDataType( ReTextureDataType newVal );

  /*
   * Method: getTexture1
   */
  ReTexturePtr getTexture1();

  /**
   * setTexture1
   */
  void setTexture1( ReTexturePtr newVal );
  
  inline void setTexture1( float f ) {
    fTex1 = f;
    setTexture1(ReTexturePtr());
    useFloatValuesFlag = true;
  }
  /*
   * Method: getTexture2
   */
  ReTexturePtr getTexture2();

  /**
   * setTexture2
   */
  void setTexture2( ReTexturePtr newVal );

  inline void setTexture2( float f ) {
    fTex2 = f;
    setTexture2(ReTexturePtr());
    useFloatValuesFlag = true;
  }

  inline bool useFloatValues() {
    return useFloatValuesFlag;
  }

  inline float getFloat1() const {
    return fTex1;
  }
  inline float getFloat2() const {
    return fTex2;
  }
  
  /*
   Method: getMixAmount
   */
  float getMixAmount() const;
  /*
   Method: setMixAmount
   */
  void setMixAmount( const float newVal );

  /*
   Method: getMixTexture
   */
  ReTexturePtr getMixTexture() const;
  
  /*
   Method: setMixTexture
   */
  void setMixTexture( const ReTexturePtr newVal );

  void replaceInnerTexture( const QString textureName, ReTexturePtr newTexture );
  
  QString toString();

  void setNamedValue( const QString& name, const QVariant& value );
  const QVariant getNamedValue( const QString& name );
    
  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  QStringList getDependencies();

};

// Typedef: MixPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReMixTexture> ReMixTexturePtr;

}

#endif
