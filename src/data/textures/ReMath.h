/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef REMATH_H
#define REMATH_H

#include "Re2DTexture.h"
#include "ReLogger.h"
#include "textures/ReComplexTexture.h"  

// Name of the channels used to store the sub-textures
// The MT prefix stands for Math Texture
#define MT_TEX1 "tex1"
#define MT_TEX2 "tex2"

namespace Reality {

/**
 * Class: ReMath
 *
 * A float texture that imposes a mathematical operation between two textures.
 * The operation can be one of add, subtract or multiply. See <ReTexture>.
 */

class RE_LIB_ACCESS ReMath : public Re2DTexture,
                             public ReComplexTexture
{

public:

  //! The available functions of the pixels of the textures
  enum functions {
    add, subtract, multiply
  };

protected:
  //! The amount of the texture1 used. This value, if different from 1.0 will 
  //! be multiplied with texture1 using a float scale texture in Lux.
  float amountTex1;

  //! Same of above but for texture2
  float amountTex2;
  
  functions function;

  //! Dependency list
  QStringList deps;

private:
  void initChannels();

public:

  ReMath( const QString name, ReTextureContainer* parentMat = 0 );

  ReMath( const QString name,
          ReTextureContainer* parentMat,
          ReTexturePtr tex1, 
          ReTexturePtr tex2, 
          functions func );

  //! Texture conversion ctor
  ReMath( const ReTexturePtr srcTex );

  ~ReMath() {
    // Nothing
  };

  //! Inhibits the redefinition of the data type. Math textures can
  //! only be numeric.
  void setTextureDataType( ReTextureDataType /* newVal */ ) {
    // nothing
  }

  virtual const QString getGUID() {
    // This texture name : GUID of tex1 : GUID of tex2 : amount1 : amount 2
    return QString("%1:%2:%3:%4:%5")
             .arg(getTypeAsString())
             .arg(channels[MT_TEX1].isNull() ? "" : channels[MT_TEX1]->getGUID())
             .arg(channels[MT_TEX2].isNull() ? "" : channels[MT_TEX2]->getGUID())
             .arg(amountTex1)
             .arg(amountTex2);
  };

  virtual void reparent( ReTextureContainer* parentMat );

  inline virtual void setAmount1( const float newVal ) {
    amountTex1 = newVal;
  }

  inline virtual float getAmount1() const {
    return amountTex1;
  }

  inline virtual void setTexture1( ReTexturePtr tex1 ) {
    channels[MT_TEX1] = tex1;
  }
  inline ReTexturePtr getTexture1() {
    return channels[MT_TEX1];
  }

  inline virtual void setAmount2( const float newVal ) {
    amountTex2 = newVal;
  }

  inline virtual float getAmount2() const {
    return amountTex2;
  }

  inline virtual void setTexture2( ReTexturePtr tex2 ) {
    channels[MT_TEX2] = tex2;
  }

  inline ReTexturePtr getTexture2() {
    return channels[MT_TEX2];
  }

  inline void setFunction( const functions func ) {
    function = func;
  }

  inline functions getFunction() const {
    return function;
  }

  inline QString getFunctionAsString() const {
    switch(function) {
      case add:
        return "add";
      case subtract:
        return "subtract";
      case multiply:
        return "multiply";
    }
  }

  void replaceInnerTexture( const QString textureName, ReTexturePtr newTexture );

  QString toString();

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  void setNamedValue( const QString& name, const QVariant& value );
  const QVariant getNamedValue( const QString& name );

  QStringList getDependencies() {
    deps.clear();
    if (!channels[MT_TEX1].isNull()) {
      deps << channels[MT_TEX1]->getName();
    }
    if (!channels[MT_TEX2].isNull()) {
      deps << channels[MT_TEX2]->getName();
    }
    return deps;
  };

};

//!Convenient definition of a shared pointer
typedef QSharedPointer<ReMath> ReMathPtr;


} // namespace


#endif
