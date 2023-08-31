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
class Clouds : public ReProceduralNoise {

private:
  bool usesHardNoiseFlag;
  quint16 noiseDepth;

public:

  // Constructor: Clouds
  Clouds(const QString name, ReTextureContainer* parentMat = 0) ;

  // Texture conversion ctor
  Clouds( const ReTexturePtr srcTex );

  // Destructor: Clouds
 ~Clouds() {

  };

  virtual QString toString() {
    return "Clouds texture";
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

// Typedef: CloudsPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<Clouds> CloudsPtr;



} // namespace


#endif
