/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_CLOUDS_H
#define RE_CLOUDS_H

#include "textures/ReProceduralNoise.h"


namespace Reality {

/**
  See also <ProceduralNoise>
 */
class ReClouds : public ReProceduralNoise {

private:
  bool usesHardNoiseFlag;
  quint16 noiseDepth;

public:

  // Constructor: ReClouds
  ReClouds(const QString name, ReTextureContainer* parentMat = 0) ;

  // Texture conversion ctor
  ReClouds( const ReTexturePtr srcTex );

  // Destructor: ReClouds
 ~ReClouds() {

  };

  virtual QString toString() {
    return "ReClouds texture";
  };

  inline void setHardNoise( const bool yesOrNo ) {
    usesHardNoiseFlag = yesOrNo;
  }
  
  inline bool usesHardNoise() const {
    return usesHardNoiseFlag;
  }
  
  /*
   Method: getNoiseDepth
   */
  inline quint16 getNoiseDepth() const {
    return noiseDepth;
  };
  /*
   Method: setNoiseDepth
   */
  inline void setNoiseDepth( quint16 newVal ) {
    noiseDepth = newVal;
  };

  void setNamedValue( const QString& name, const QVariant& value );

  const QVariant getNamedValue( const QString& name );

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );
  
  //! Copy all the properties from a compatible texture
  virtual void copyProperties( const ReTexturePtr srcTex );
  
};

// Typedef: ReCloudsPtr
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReClouds> ReCloudsPtr;



} // namespace


#endif
