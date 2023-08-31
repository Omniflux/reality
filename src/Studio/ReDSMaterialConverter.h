/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#ifndef REDSMATERIALCONVERT_H
#define REDSMATERIALCONVERT_H

#include <boost/function.hpp>
#include <QVariantMap>

class DzFloatProperty;
class DzMaterial;
class DzProperty;
class DzTexture;


namespace Reality {

/**
  Convert the DAZ Studio material to the table of properties needed
  by Reality to send to ReGeometryObject.
  This class is a singleton accessed via the method getInstance()
 */

class ReDSMaterialConverter {
private:
  //! Factor by which we divide the positive/negative bump map values from
  //! Studio. 
  const int bumpScale;

  //! Factor by which we divide the positive/negative displacement map values 
  //! from Studio. 
  const int displacementScale;

  static ReDSMaterialConverter* instance;

  /**
   * Properties are scanned in a random order. The texture tiling and 
   * offset might be received before or after the image map that causes
   * the image map node to be created. To make the system work we store
   * the H/V tiling and the H/V offsets in the class, using default values.
   * If the tiling and offset data are received first they will update the
   * default values and so, when we receive the image map property the
   * image map node will be created with the correct tiling and offset
   * values.
   * If the image map is received first then we will create a node with 
   * the default values. Later on, when the tiling and offset properties
   * are found we will scan the set of node and update each one of the
   * image map nodes. In LuxRender the tiling and offset of image maps
   * is calculated on a per-texture basis. In Studio the same tiling and
   * offsets are use for all the textures in a material
   */
  float uTile, vTile;
  float uOffset, vOffset;

  //! Whether or not the shader that we are processing is for iRay.
  bool isiRayShader;

  //! The material that we are currently converting
  DzMaterial* dsMat;

  QVariantMap captureBumpMapData(const DzFloatProperty* bmProp, 
                                 DzTexture* bumpMap );

  QVariantMap captureNormalMapData( DzTexture* normalMap );

  void mixBumpMapData( QVariantMap& bmData, QVariantMap& nmData );

  //! Constructor: ReDSMaterialConverter
  ReDSMaterialConverter() : bumpScale(10), displacementScale(100) 
  {
    isiRayShader = false;
  };

  //! Specialized function to convert the specular channel of iRay
  void convertIRaySpecular(const DzProperty* prop);

  //! Specialized function to convert to the speculative channel of 3DLight
  void convert3DLSpecular(const DzProperty* prop);

public:
  void convertDiffuse( const DzProperty* diffProp );
  void convertBumpMap( const DzProperty* bumpProp );
  void convertNormalMap( const DzProperty* nmProp );
  void convertDispMap( const DzProperty* dispProp );
  void convertAmbient( const DzProperty* ambProp );
  void convertSpecular( const DzProperty* diffProp );
  void convertAlpha( const DzProperty* prop );
  void convertTiling( const DzProperty* prop );

private:
  //! Assigns a color as a list of three RGB values to the list contained in var 
  QVariantList setColor( const QColor& clr );

  /**
   * Several properties have multiple names. UV tiling, displacement and 
   * others have been known to use different names with different shaders.
   * In addition, it's faster to look up a property in a set than to run
   * a series of ifs(). For these reasons we keep a LUT of properties
   * identified by simpler names:
   *
   *  - kd          => Diffuse
   *  - kd_value    => Diffuse strength
   *  - ks          => Specular
   *  - ks_value    => Specular strength
   *  - ks2         => Specular 2
   *  - ks2_value   => Specular 2 strength
   *  - htile       => Horizontal tiling
   *  - vtile       => Vertical tiling
   *  - gloss       => glossiness
   *  - alpha_value => Alpha strength
   *  - alpha       => Alpha map
   *  - bm          => Bump map
   *  - bm_low      => Negative bump
   *  - bm_high     => Positive bump
   *  - bm_value    => Bump strength
   *  - dm          => Displacement map
   *  - dm_low      => Negative disp.
   *  - dm_high     => Positive disp.
   *  - dm_value    => Disp. strength   
   *  - nm          => Normal map
   *  - amb         => Ambient Color
   *  - amb_on      => Ambient Active
   *  - amb_value   => Ambient Strength
   */
  typedef QHash<QString, QString> PropertyLUT_t;

  //! Callback prototype
  typedef boost::function<void (ReDSMaterialConverter*, 
                                const DzProperty* )> ConverterCallback;

  //! This type defines a table that holds a callback to convert a
  //! given property. The key is the name of the Studio property and
  //! the value is a pointer to a callback that converts that property
  typedef QHash<QString, ConverterCallback> PropertyCallback_t;

  //! The Property Look Up Table that converts from Studio properties
  //! to the Reality labels used to refer to them
  static PropertyLUT_t propertyLUT;

  static PropertyCallback_t propertyConverters;

  static void initLUT();

  //! Checks if the given DS property is recognized as one
  //! of the properties handled by Reality
  inline bool isProperty( const QString& dsPropName, 
                          const QString& rePropName ) 
  {    
    return ( propertyLUT.value(dsPropName,"") == rePropName );
  };

  // void convertProperty( const DzProperty* prop, const DzMaterial* dsMat );

  void processMaterialTags(const QString& tagStr);

public:
  //! Destructor: ReDSMaterialConverter
 ~ReDSMaterialConverter() {
    if (instance) {
      delete instance;
    }
  };
  
  //! Singleton implementation. This is the only this class
  //! can be accessed.
  static ReDSMaterialConverter* getInstance();

  //! This method takes a reference to a Studio material and returns
  //! a map with all the components defined using the Reality data
  //! structure needed by ReGeometryObject for the final conversion
  QVariantMap* convertMaterial( DzMaterial* dsMat );

};

} // namespace

#endif
