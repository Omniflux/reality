#ifndef MATERIAL_EDITOR_H
#define MATERIAL_EDITOR_H

#include <QWidget>

#include "ReGeometryObject.h"
#include "ReMaterial.h"
#include "ReTextureChannelDataModel.h"
#include "RealityUI/ReTimedEditor.h"


using namespace Reality;

/**
 * Virtual base class for all Material Editors.
 */  
class ReMaterialEditor : public QWidget,
                         public ReTimedEditor

{
  
  Q_OBJECT

public:
  ReMaterialEditor( QWidget* parent = 0 ) : 
    QWidget(parent), 
    ReTimedEditor("mats")
  {
    initTimer(RE_CFG_UPDATE_INACTIVITY, this, SLOT(fireUpdate()));
    currentMaterial = NULL;
  };
  
  virtual void setData( ReMaterial* newMat, 
                        ReTextureChannelDataModelPtr tcm ) = 0;

protected:
  QVariantMap updatedQueue;

  //! The material being edited
  ReMaterial* currentMaterial;

  //! The model used to signal changes in the textures
  ReTextureChannelDataModelPtr textureChannelModel;

  /**
   * Used to queue an update to be delivered when the timer expires.
   * The updates are queued so that multiple updates to a single value
   * are merged into a single update.
   */
  void signalUpdate(QString valueName, QVariant value)  {
    updatedQueue[valueName] = value;
    resetTimer();
  };


public slots:
  //! Sends the signal that the values queued have been updated. This event
  //! happens when the timer expires. It can be called explicitly to flush
  //! the queue.
  void fireUpdate() {
    QStringList keys = updatedQueue.keys();
    QListIterator<QString> i(keys);
    while( i.hasNext() ) {
      QString key = i.next();
      if (currentMaterial) {
        emit materialUpdated( currentMaterial->getParent()->getInternalName(), 
                              currentMaterial->getName(),
                              key,
                              updatedQueue.value(key) );
      }
      updatedQueue.remove(key);
    }
  };

signals:
  //! Emitted when the user starts editing a texture
  void editTexture( ReTexturePtr tex, QString& channelID );
  
  //! Emitted when the user clicks on the Color or ImageMap texture
  //! preview
  void directEdit( ReTexturePtr tex, QString& channelID );

  //! Emitted when the user adds a new texture to a material channel
  void makeNewTexture( const QString& objectID,
                       const QString& materialID, 
                       const QString& channelName, 
                       const ReTextureType textureType,
                       const ReTexture::ReTextureDataType dataType );

  void makeNewTexture(const QString& objectID,
                      const QString& materialID, 
                      const QString& channelName, 
                      const QString& masterTextureName, 
                      const QString& newTextureName, 
                      const ReTextureType textureType,
                      const ReTexture::ReTextureDataType dataType);

  //! Emitted when the user links an existing texture to a material's channel
  void linkTexture( const QString& objectID,
                    const QString& materialID, 
                    const QString& channelID, 
                    const QString& textureName );

  //! Emitted when the user detaches a texture from a material's channel
  void unlinkTexture( const QString& objectID,
                      const QString& materialID, 
                      const QString& channelID );
  //! Emitted when a component of a material has been edited
  void materialUpdated( QString objectID, 
                        QString materialName, 
                        QString propertyName, 
                        QVariant value );

  void volumeAdded( const QString& volumeName );
  void volumeUpdated( const QString& volName );

  //! Emitted when the user pastes a texture and the same change needs to be 
  //! applied to the host-side copy of the same material and texture.
  void replaceTexture(const QString& channelID, 
                      const QString& jsonTextureData);
};


#endif
