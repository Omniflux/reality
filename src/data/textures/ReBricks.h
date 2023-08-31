  /*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REBRICKS_H
#define REBRICKS_H

#include "reality_lib_export.h"
#include "ReTexture.h"
#include "textures/ReComplexTexture.h"


namespace Reality {

#define RE_BRICK_BRICK_CHANNEL  "brick"
#define RE_BRICK_MORTAR_CHANNEL "mortar"
#define RE_BRICK_MODULATION_CHANNEL "mod"

/**
 * Implementation of the LuxRender brick texture
 *
 * See also <ReTexture>.
 */
class REALITY_LIB_EXPORT ReBricks : public ReTexture3D,
                             public ReComplexTexture 
{

public:
  enum BrickType {
    STACKED,
    FLEMISH,
    ENGLISH,
    HERRINBONE,
    BASKET,
    CHAINLINK
  };

private:
  float width,
        height,
        depth,
        bevel,
        offset,
        mortarSize;

  BrickType brickType;
  // for getDependencies()
  QStringList deps;

  void initChannels();
public:
  // LookUp Table (LUT) that converts the enum values to
  // their string equivalent. 
  static QString lutTypesToStrings[CHAINLINK+1];

  ReBricks( const QString name,
          ReTextureContainer* parentMat = 0,
          const ReTextureDataType dataType = color ) ;

 ~ReBricks();

  // Conversion ctor
  ReBricks( const ReTexturePtr srcTex );

  /*
   Method: getBrickType
   */
  const BrickType getBrickType() const;

  /*
   Method: setBrickType
   */
  void setBrickType( BrickType newVal );

  
  QString getBrickTypeAsString();

  /*
   Method: getBrickTexture
   */
  const ReTexturePtr getBrickTexture() const;

  //! Sets the texture for the brick surface
  void setBrickTexture( ReTexturePtr newVal );

  //! Sets the texture for the mortar 
  void setMortarTexture( ReTexturePtr newVal ) ;

  //! Sets the texture for the mortar 
  void setBrickModulationTexture( ReTexturePtr newVal );

  /*
   Method: getWidth
   */
  const float getWidth() const;

  /*
   Method: setWidth
   */
  void setWidth( float newVal );

  /*
   Method: getHeight
   */
  const float getHeight() const;

  /*
   Method: setHeight
   */
  void setHeight( float newVal );

  /*
   Method: getDepth
   */
  const float getDepth() const;

  /*
   Method: setDepth
   */
  void setDepth( float newVal );

  /*
   Method: getBevel
   */
  const float getBevel() const;

  /*
   Method: setBevel
   */
  void setBevel( float newVal );

  /*
   Method: getOffset
   */
  const float getOffset() const;

  /*
   Method: setOffset
   */
  void setOffset( float newVal );

  /*
   Method: getMortarSize
   */
  const float getMortarSize() const;

  /*
   Method: setMortarSize
   */
  void setMortarSize( float newVal );

  const ReTexturePtr getBrickModulationTexture() const;
  const ReTexturePtr getMortarTexture() const;

  QStringList getDependencies();

  // Method: replaceTexture
  // Some texture include references to other textures. This is the case
  // for Color Math, Mix, Bricks etc. 
  // The user can change the type of those texture and so the master texture
  // must have the ability to replace the old reference with a new one.
  void replaceInnerTexture( const QString textureName, ReTexturePtr newTexture );

  void setNamedValue( const QString& name , const QVariant& value );
  const QVariant getNamedValue( const QString& name );

  void serialize( QDataStream& dataStream );

  void deserialize( QDataStream& dataStream );

  QString toString();
};

// Typedef: ReBricksPtr
//   Convenient definition of a shared pointer
typedef QSharedPointer<ReBricks> ReBricksPtr;


} // namespace

#endif
