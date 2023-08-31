/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_BAND_H
#define RE_BAND_H

#include "reality_lib_export.h"
#include "ReTexture.h"
#include "textures/ReComplexTexture.h"


// default number of sub-textures that we hold in this texture
#define REBT_DEFAULT_NUM_ENTRIES 4

namespace Reality {

typedef QMap<QString, float> ReBandOffsets;
typedef QMapIterator<QString, float> ReBandOffsetsIterator;

/**
 * Support for the LuxRender Band texture, which defined a multi-color gradient
 *  See also <ReTexture>.
 */
class REALITY_LIB_EXPORT ReBand : public ReTexture,
                             public ReComplexTexture
{

private:
  //! Number of bands, gradient points, in this texture.
  //! At the moment this number is fixed at 4 because of time 
  //! constraints for Reality 3.0. In future release it will
  //! need to be a variable number between 2 and 6
  quint16 numBands;

  //! The Band texture handles an array of textures...
  ReBandOffsets bandOffsets;
  float amount;

  QStringList deps;

  // Initializes the input channels 
  void initChannels( const qint16 numEntries );

public:

  ReBand(const QString name, ReTextureContainer* parentMat = 0);

  // texture conversion ctor
  ReBand( const ReTexturePtr srcTex );

  ~ReBand();

  QString toString();

  /*
   Method: getNumBands
   */
  int getNumBands() const;
  void setNumBands( int newVal );

  ReBandOffsets getOffsets() const;

  void reparent( ReTextureContainer* parentMat );

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  void setNamedValue( const QString& /* name */, const QVariant& /* value */);

  const QVariant getNamedValue( const QString& /* name */);

  /*
   Method: getAmount
   */
  float getAmount() const;

  /*
   Method: setAmount
   */
  void setAmount( float newVal );

  QStringList getDependencies();

  // Method: replaceTexture
  // Some texture include references to other textures. This is the case
  // for Color Math, Mix, Bricks etc. 
  // The user can change the type of those texture and so the master texture
  // must have the ability to replace the old reference with a new one.
  void replaceInnerTexture( const QString textureName, ReTexturePtr newTexture );

};

typedef QSharedPointer<ReBand> ReBandPtr;

} // namespace

#endif
