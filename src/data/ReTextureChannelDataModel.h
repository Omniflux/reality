/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.    
 */


#ifndef RETEXTURECHANNELDATAMODEL_H
#define RETEXTURECHANNELDATAMODEL_H

#include <QObject>

#include "ReTexture.h"

namespace Reality {

/**
 * A simple model class that provides a simple way to operate transformations 
 * on a texture and send signals to the listeners, usually widgets, when those
 * modifications happen. This helps keeping in sync different parts of the UI.
 * For example, the Texture Editor is passed this model when editing starts.
 * The T.E. in turn defers all its modifications to the model and the model
 * notifies the Reality panel when necessary.
 *
 * The model interfaces one texture at the time, with one instance of the model
 * being kept application-wide. This means that the model's context needs to be 
 * set to the current texture being affected. To do this we need to call the
 * setTexture() method before the method operates any transformation on the 
 * texture.
 */
class ReTextureChannelDataModel : public QObject {

  Q_OBJECT
  
protected:
  ReTexturePtr texture;

public:

  //! Constructor
  ReTextureChannelDataModel( const ReTexturePtr tex ) {
    setTexture(tex);
  };

  //! Destructor
  ~ReTextureChannelDataModel() {
    // nothing
  };

  //! Sets the texture affected by the model
  inline void setTexture( const ReTexturePtr tex ) {
    texture = tex;
  }

  inline QVariant getNamedValue( const QString& name ) {
    if (texture.isNull()) {
      return "";
    }
    return texture->getNamedValue(name);
  }
  
  inline void setNamedValue( const QString& name, const QVariant& value ) {
    if (texture.isNull()) {
      return;
    }
    texture->setNamedValue(name,value);
    emit textureUpdated(texture, name, value);
  }

  inline ReTextureType getTextureType() {
    if (texture.isNull()) {
      return TexUndefined;
    }
    return texture->getType();
  }

  inline ReTexture::ReTextureDataType getTextureDataType() {
    if (texture.isNull()) {
      return ReTexture::color;
    }
    return texture->getDataType();
  }


  inline const ReTexturePtr getTexture() {
    return texture;
  }

  //! Convenience method to change the type of the texture handled by the
  //! model. 
  //! Returns true if the conversion was successful. If that's the case, the
  //! new texture becomes also the texture handled by the model. This method
  //! does **not** send any signal. If notifications need to be propagated
  //! then the caller of this method needs to take care of that.
  bool changeTextureType( const ReTextureType newType );

  //! When a user of the model wants to communicate to the "view" that the 
  //! model has changed this is the method to call.
  inline void reset() {
    emit modelChanged();
  }

signals:
  
  void textureUpdated(ReTexturePtr texture, const QString& paramName, const QVariant& value);

  //! Signal: modelChanged
  //!   Emitted when the <reset()> method is called  
  void modelChanged();
};

typedef QSharedPointer<ReTextureChannelDataModel> ReTextureChannelDataModelPtr;
  
} // namespace

#endif
