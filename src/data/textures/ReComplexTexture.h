/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RECOMPLEX_TEXTURE_H
#define RECOMPLEX_TEXTURE_H

#include "ReTexture.h"
#include "ReTextureContainer.h"
#include <QSharedPointer>
#include <QDataStream>
#include "reality_lib_export.h"

namespace Reality {

/**
  This class defines the interface and serialization for textures that
  contain other textures.
 */
class REALITY_LIB_EXPORT ReComplexTexture {

protected:
  ReNodeDictionary channels;

public:
  // Constructor: ComplexTexture
  ReComplexTexture() {
  };

  // Destructor: ComplexTexture
  ~ReComplexTexture() {
  };

  //! Returns the texture linked by the channel channelID
  ReTexturePtr getChannel( const QString channelID ) const;
  
  bool hasChannel( const QString& channelID ) const;

  const ReNodeDictionary getChannels() const;

  //! Stream serialization methods
  void serializeChannels( QDataStream& dataStream );
  void deserializeChannels( QDataStream& dataStream, ReTextureContainer* cont );

  //! Set texture for a given channel. If the containing texture is connected
  //! to a material then the new texture passed in the parameter is also
  //! added to the material.
  void setChannel( const QString channelName, ReTexturePtr texture );

  //! Deletes the texture stored in the channel without removing the channel itself.
  void deleteChannelTexture( const QString& channelName );

  //! Given a sub-texture, this method returns the name of the channel that links
  //! to that texture. If the texture is not linked by the master texture then
  //! this method returns an empty string.
  //!
  //! The format used for the string returned is to identify every component of the
  //! path to the channel by separating the path elements with a |. So, for example,
  //! a path could be "tex1|tex2".
  const QString identifyChannel( const ReTexturePtr tex ) const;
};

typedef QSharedPointer<ReComplexTexture> ReComplexTexturePtr;

} // namespace


#endif
