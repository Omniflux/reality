/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReTexture.h"
#include "ReComplexTexture.h"
#include "textures/ReConstant.h"
#include "ReTools.h"
  
#ifndef RE_CHECKERS_H
#define RE_CHECKERS_H

#define RECH_TEX1 "tex1"
#define RECH_TEX2 "tex2"

namespace Reality {

/**
  Interface to the LuxRender's checkerboard texture
 */
class ReCheckers : public ReTexture3D, public ReComplexTexture {
private:
  bool is3DFlag;

public:
  // Constructor: ReCheckers
  ReCheckers( const QString name, 
              ReTextureContainer* parentMat = 0 ) : 
    ReTexture3D(name, parentMat),
    is3DFlag(true)
  {
    type = TexCheckers;
    textureDataType = numeric;
    // Black texture
    channels[RECH_TEX1] = ReTexturePtr(
                         new ReConstant(QString("%1_checkers1").arg(name), parent, 8 )
                       );
    // White texture
    channels[RECH_TEX2] = ReTexturePtr(
                         new ReConstant(QString("%1_checkers2").arg(name), parent, 240 )
                       );
    parent->addTextureToCatalog(channels[RECH_TEX1]);
    parent->addTextureToCatalog(channels[RECH_TEX2]);
  };


  // Conversion ctor
  ReCheckers( const ReTexturePtr srcTex ) :
    ReTexture3D(srcTex),
    is3DFlag(true)
  {
    type = TexCheckers;
    textureDataType = numeric;
    // Black texture
    channels[RECH_TEX1] = ReTexturePtr(
                         new ReConstant(QString("%1_checkers1").arg(name), parent, 8 )
                       );
    // White texture
    channels[RECH_TEX2] = ReTexturePtr(
                         new ReConstant(QString("%1_checkers2").arg(name), parent, 240 )
                       );
    parent->addTextureToCatalog(channels[RECH_TEX1]);
    parent->addTextureToCatalog(channels[RECH_TEX2]);
  }

  // Destructor: ReCheckers
  ~ReCheckers() {
  };

  QStringList getDependencies() {
    QStringList deps;
    deps << channels[RECH_TEX1]->getName() << channels[RECH_TEX2]->getName();
    return deps;
  }

  //! Some texture include references to other textures. This is the case
  //! for Color Math, Mix, Bricks etc. 
  //! The user can change the type of those texture and so the master texture
  //! must have the ability to replace the old reference with a new one.
  void replaceInnerTexture( const QString textureName, ReTexturePtr newTexture ) {
    if (channels[RECH_TEX1]->getName() == textureName) {
      channels[RECH_TEX1].clear();
      channels[RECH_TEX1] = newTexture;
    }
    else if (channels[RECH_TEX2]->getName() == textureName) {
      channels[RECH_TEX2].clear();
      channels[RECH_TEX2] = newTexture;
    }
  };

  /*
   Method: getTex1
   */
  inline const ReTexturePtr getTex1() const {
    return channels[RECH_TEX1];
  };

  /*
   Method: setTex1
   */
  inline void setTex1( ReTexturePtr newVal ) {
    channels[RECH_TEX1] = newVal;
  };

  /*
   Method: getTex1
   */
  inline const ReTexturePtr getTex2() const {
    return channels[RECH_TEX2];
  };
  
  /*
   Method: setTex2
   */
  inline void setTex2( ReTexturePtr newVal ) {
    channels[RECH_TEX2] = newVal;
  };

  inline bool is3D() const {
    return is3DFlag;
  };

  inline void set3DFlag( bool onOff ) {
    is3DFlag = onOff;
  }

  inline void setNamedValue( const QString& name , const QVariant& value ) {
    if ( name == RECH_TEX1 ) {
      channels[RECH_TEX1] = value.value<ReTexturePtr>();
    }
    else if ( name == RECH_TEX2 ) {
      channels[RECH_TEX2] = value.value<ReTexturePtr>();
    }    
    else {
      ReTexture3D::setNamedValue(name, value);      
    }
  };

  inline const QVariant getNamedValue( const QString& name ) {
    QVariant val;
    if (name == RECH_TEX1) {
      val.setValue(channels[RECH_TEX1]);
    }
    else if (name == RECH_TEX2) {
      val.setValue(channels[RECH_TEX2]);
    }
    else {
      return ReTexture3D::getNamedValue(name);
    }
    return val;    
  };

  inline void serialize( QDataStream& dataStream ) {
    ReTexture3D::serialize(dataStream);
    serializeChannels(dataStream);
  };

  inline void deserialize( QDataStream& dataStream ) {
    ReTexture3D::deserialize(dataStream);
    deserializeChannels(dataStream, parent);    
  };


  virtual QString toString() {
    return "ReCheckers texture";
  };
  
};

// Typedef: CheckersPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReCheckers> ReCheckersPtr;

} // namespace

#endif
