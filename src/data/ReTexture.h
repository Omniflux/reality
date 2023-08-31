/**
 * File: ReTexture.h
 *
 * These classes define the texture types handled by Reality. The base class simply defines the common properties for tiling 
 * and the texture type. We handle only two basic texture types: 
 *
 *  - Color: a texture that uses RGB values for its pixels
 *  - Float: a texture that uses floating point values for its pixels
 *    This comes in handy when using a texture to drive another value, like displacement maps. 
 * 
 */

/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RETEXTURE_H
#define RETEXTURE_H

#include <QSharedPointer>
#include <QStringList>
#include <QVariant>

#include "reality_lib_export.h"
#include "ReDefs.h"


namespace Reality {

/**
 Base class for all textures
 */

// Forward declaration for storage of the parent of the texture.
class ReTextureContainer;

// Forward declaration for friendship
class ReTextureCreator;

class ReTexture;
//! Smart pointer to a texture. Used to link textures together.
typedef QSharedPointer<ReTexture> ReTexturePtr;

class REALITY_LIB_EXPORT ReTexture {

public:
  enum ReTextureDataType {
    color, numeric, fresnel
  };

protected:
  static QString dataTypeNames[fresnel+1];

  QString name;
  ReTextureType type;

  ReTextureDataType textureDataType;

  // ReMaterial* parent;
  ReTextureContainer* parent;

  //! Value of any property for this texture returned as a QVariant.
  //! See <getNamedvalue()>
  QVariant val;

public:
  //! Creates a texture with name and parented to the material passed in parent 
  ReTexture(const QString& name, ReTextureContainer* parent = 0);

  //! Copy ctor
  ReTexture( const ReTexture& t2 ) :
    name(t2.name),
    type(t2.type),
    textureDataType(t2.textureDataType),
    parent(t2.parent)
  {

  }

  ReTexture( ReTexturePtr srcTex );

  virtual ~ReTexture() {
  }


  inline const QString& getName() const {
    return name;
  }

  inline void setName( QString newName ) {
    name = newName;
  }

  /**
   Creates a unique texture name for this texture by concatenating the name of the object, the material
   and the texture
   */
  const QString getUniqueName() const;

  //! Returns a Globally Unique ID for this texture for the purpose of
  //! caching.
  virtual const QString getGUID() {
    return getUniqueName();
  };

  inline ReTextureDataType getDataType() const {
    return textureDataType;
  }

  virtual void setTextureDataType( ReTextureDataType newVal ) {
    textureDataType = newVal;
  }

  inline ReTextureType getType() const {
    return type;
  }

  const QString getTypeAsString() const;

  static const QString getTypeAsString( const ReTextureType texType );

  inline const QString getDataTypeAsString() const {
    return(dataTypeNames[textureDataType]);
  }

  static const QString getDataTypeAsString( const ReTextureDataType dataType );

  virtual void setNamedValue( const QString& vname , const QVariant&  value ) {
    if ( vname == "name" ) {
      name = value.toString();
    }
    else if ( vname == "type" ) {
      type = static_cast<ReTextureType>(value.toInt());
    }
    else if ( vname == "textureDataType" ) {
      textureDataType = static_cast<ReTextureDataType>(value.toInt());
    }
  }

  virtual const QVariant getNamedValue( const QString& vname ) {
    if ( vname == "name" ) {
      return name;
    }
    else if ( vname == "type" ) {
      return type;
    }
    else if ( vname == "textureDataType" ) {
      return textureDataType;
    }
    return val;
  }
  /**
   * Function: getDependencies
   * 
   * Returns a string list with the names of textures that are
   * linked by this texture. The list can be empty.
   */ 
  virtual QStringList getDependencies() {
    return QStringList();
  };

  const QString findImageMap() {
    switch( type ) {
      case TexImageMap:
        return getNamedValue("fileName").toString();
      case TexColorMath:
      case TexMix: {
        ReTexturePtr tmpTex = getNamedValue("texture1").value<ReTexturePtr>();
        if (!tmpTex.isNull()) {
          QString im = tmpTex->findImageMap();
          if (im != "") {
            return im;
          }
        }
        tmpTex = getNamedValue("texture2").value<ReTexturePtr>();
        if (!tmpTex.isNull()) {
          return tmpTex->findImageMap();
        }
      }
      default:
        return "";
    }
    return "";
  }


  /**
   Connects the parent material to the texture and add the texture to the material's
   nodeCatalog.
   */
  virtual void reparent( ReTextureContainer* newParent );

  inline ReTextureContainer* getParent() const {
    return parent;
  }

  virtual QString toString() = 0;

  virtual void serialize( QDataStream& dataStream );

  virtual void deserialize( QDataStream& dataStream );

  // Method: replaceTexture
  // Some texture include references to other textures. This is the case
  // for Color Math, Mix, Bricks etc. 
  // The user can change the type of those texture and so the master texture
  // must have the ability to replace the old reference with a new one.
  virtual void replaceInnerTexture( const QString /* textureName */, 
                                    ReTexturePtr /* newTexture */) {
    // nothing.
  }

  friend class ReTextureCreator;
};

//! A node dictionary is used to keep track of all the nodes
//! associated with a given material. The dictionary is keyed by
//! node name and holds pointers to node, which are subclasses
//! of ReTexture.
typedef QHash<QString, ReTexturePtr> ReNodeDictionary;
typedef QHashIterator<QString, ReTexturePtr> ReNodeDictionaryIterator;


/*
  Class: ReTexture3D

  A texture with a scale factor and 3D mapping
 */

class REALITY_LIB_EXPORT ReTexture3D;
typedef QSharedPointer<ReTexture3D> ReTexture3DPtr;

class ReTexture3D : public ReTexture {
public:
  enum Mapping3D {
    UV,
    GLOBAL,
    GLOBAL_NORMAL,
    LOCAL,
    LOCAL_NORMAL
  };

protected:
  // Although Lux allows scaling on all three axis we only implement
  // uniform scaling
  float scale;

  float xRot,
        yRot,
        zRot;

  Mapping3D mapping;

public:
  // Constructor: ReTexture3D
  ReTexture3D( const QString& name, ReTextureContainer* parent = 0 ) :
    ReTexture( name, parent ),
    scale(1.0),
    xRot(0),
    yRot(0),
    zRot(0),
    mapping(UV)
  {
  };

  ReTexture3D( const ReTexturePtr srcTex ) :
    ReTexture(srcTex),
    scale(1.0),
    xRot(0),
    yRot(0),
    zRot(0),
    mapping(UV)
  {
    ReTexture3DPtr t2 = srcTex.dynamicCast<ReTexture3D>();
    if (!t2.isNull()) {
      scale   = t2->scale;
      xRot    = t2->xRot;
      yRot    = t2->yRot;
      zRot    = t2->zRot;
      mapping = t2->mapping;
    }
  }
  // Destructor: ReTexture3D
  virtual ~ReTexture3D() {
  };

  /*
   Method: getScale
   */
  inline float getScale() const {
    return scale;
  };
  /*
   Method: setScale
   */
  inline void setScale( float newVal ) {
    scale = newVal;
  };    

  /*
   Method: getMapping
   */
  Mapping3D getMapping() const {
    return mapping;
  };
  
  /*
   Method: setMapping
   */
  void setMapping( Mapping3D newVal ) {
    mapping = newVal;
  };

  void getRotation( float& x, float& y, float& z) const {
    x = xRot;
    y = yRot;
    z = zRot;
  }

  void setRotation( float x, float y, float z) {
    xRot = x;
    yRot = y;
    zRot = z;
  }

  virtual void setNamedValue( const QString& name , const QVariant& value ) {
    if (name == "scale") {
      scale = value.toFloat();
    }
    else if (name == "xRot") {
      xRot = value.toInt();
    }
    else if (name == "yRot") {
      yRot = value.toInt();
    }
    else if (name == "zRot") {
      zRot = value.toInt();
    }
    else if (name == "mapping") {
      mapping = static_cast<Mapping3D>(value.toInt());
    }
  }

  virtual const QVariant getNamedValue( const QString& vname ) {
    if (vname == "scale") {
      return scale;
    }
    else if (vname == "xRot") {
      return xRot;
    }
    else if (vname == "yRot") {
      return yRot;
    }
    else if (vname == "zRot") {
      return zRot;
    }
    else if (vname == "mapping") {
      return mapping;
    }
    return val;
  }

  //! Copy all the properties from a compatible texture
  virtual void copyProperties( const ReTexturePtr srcTex ) {
    setNamedValue("scale", srcTex->getNamedValue("scale"));
    setNamedValue("xRot", srcTex->getNamedValue("xRot"));
    setNamedValue("yRot", srcTex->getNamedValue("yRot"));
    setNamedValue("zRot", srcTex->getNamedValue("zRot"));
    setNamedValue("mapping", srcTex->getNamedValue("mapping"));
  }

  virtual void serialize( QDataStream& dataStream );

  virtual void deserialize( QDataStream& dataStream );
};



/*
  Class: MultiMix

  See also <ReTexture>.

 */

class REALITY_LIB_EXPORT MultiMix : public ReTexture {

public:
  // Constructor: MultiMix
  MultiMix(const QString name, ReTextureContainer* parentMat = 0) : 
    ReTexture(name, parentMat) {

    type = TexMultiMix;

  };

  // Conversion ctor
  MultiMix( const ReTexturePtr srcTex ) :
    ReTexture(srcTex)
  {

  }

  // Destructor: MultiMix
 ~MultiMix() {
  };

  // const QString getStringAvatar( const quint16  avatarNumber = 1 ) {
  //   return "Multimix";
  // };

  virtual QString toString() {
    return "MultiMix texture";
  };

  void setNamedValue( const QString& /* name */, const QVariant& /* value */) {
    //!TODO! finish
  }

  const QVariant getNamedValue( const QString& /* name */) {
    //!TODO! finish
    return val;
  }
  
};

// Typedef: MultiMixPtr 
//   Convenient definition of a shared pointer
typedef QSharedPointer<MultiMix> MultiMixPtr;


} // namespace Reality

// Support for QVariant
Q_DECLARE_METATYPE(Reality::ReTexturePtr)


#endif