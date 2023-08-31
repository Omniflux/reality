/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_TEXTURE_CONTAINER_H
#define RE_TEXTURE_CONTAINER_H

#include "reality_lib_export.h"
#include "Retexture.h"

class QString;


namespace Reality {

/**
  Basic interface that is required by any class that needs to hold
  textures. A texture needs to be able to ask basic services
  from the object that contains it. To avoid strict coupling with
  materials we use this class, which can be implemented by <ReMaterial>,
  <ReLight> or other objects that need to use textures.
 */
class REALITY_LIB_EXPORT ReTextureContainer {

public:
  // Constructor: ReTextureContainer
  ReTextureContainer() {
  };
  // Destructor: ReTextureContainer
  virtual ~ReTextureContainer() {
  };

  //! Returns the name of the container. Typically this is the
  //! name of the material.
  virtual QString getName() const = 0;

  //! Returns a pointer to a given texture held by this container.
  virtual ReTexturePtr& getTexture( const QString& textureID ) = 0;

  //! Returns the list of all the textures handled by this container
  virtual ReNodeDictionary getTextures() const = 0;

  //! Add a texture to the container.
  virtual void addTextureToCatalog( ReTexturePtr tex ) = 0;

  //! Returns a string containing a name that is unique for this
  //! container/material. This is used to have a unique ID
  //! to this container/material. Very useful when exporting to 
  //! Lux or other programs that need to identify the material.
  virtual const QString getUniqueName() const = 0;

  //! Given a texture it returns the channel (Kd, Ks etc. ) that is 
  //! using this texture. 
  //! \return
  //!   The name of the channel or an empty string if the texture is 
  //!   not used in any channel.
  virtual const QString identifyChannel( const ReTexturePtr tex ) const = 0;

  virtual void setChannel( const QString& channelName, const QString& textureName ) = 0;

  virtual bool textureIsUnused( const QString& texName, const QString ownerTexture = "" ) const = 0;

  //! Removes a texture from the catalog and deletes it if not in use anymore
  virtual bool deleteTexture( const QString& tex ) = 0;

};


} // namespace


#endif
