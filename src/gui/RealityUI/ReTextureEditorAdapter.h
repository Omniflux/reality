/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_TEXTURE_EDITOR_ADAPTER_H
#define RE_TEXTURE_EDITOR_ADAPTER_H

#include <QClipboard>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QPair>
#include <QVariantMap>
#include <QJson/Serializer>

#include "RealityBase.h"
#include "ReGeometryObject.h"
#include "ReMaterial.h"
#include "ReTextureCreator.h"
#include "ReTextureChannelDataModel.h"
#include "actions/ReSetImageMapAction.h"
#include "exporters/qt/ReQtTextureExporterFactory.h"
#include "RealityUI/ReTextureAvatar.h"
#include "textures/ReConstant.h"
#include "textures/ReImageMap.h"


namespace Reality {

typedef QPair<ReTexturePtr, QVariantMap> TextureRestoreValues;

/**
 * This is an adapter that links a given ReTextureAvatar (aka a Texture Preview)
 * with the hosting widget, either a material editor or the Texture Editor. 
 * Texture Previews have a menu that gives the user
 * the ability to perform the same group of actions on the texture:
 * - Add a new texture
 * - Detach an existing texture
 * - Link to an existing texture
 * - Create a new image map
 * - Copy a texture
 * - Paste a texture
 * 
 * To avoid coding the same logic in each material preview this adapter 
 * encapsulates the logic used in performing the above actions
 *
 * Usage:
 * \code
 *  ReTextureEditorAdapterPtr adapter;
 *  adapter = QSharedPointer<ReTextureEditorAdapter>( 
 *              new ReTextureEditorAdapter(
 *                    this, 
 *                    <qtWidget>, 
 *                    currentMaterial, 
 *                    ReTexture::ReTextureDataType::numeric
 *                  )
 *            );
 * \endcode
 */
class ReTextureEditorAdapter : public QObject {

  Q_OBJECT

private:
  //! The editor widget that controls this adapter
  QWidget* owner;
  //! The Texture Preview/Avatar that displays the texture being edited
  ReTextureAvatar* texPreview;

  //! The material that is controlling this texture. This is set explicitly
  //! because an adapter can be created with no initial texture. The 
  //! adapter is capable of creating a new texture for the linked material,
  //! so we have a situation where obtaining the material as the 
  //! container/parent of the texture is not feasible.
  ReMaterial* mat;

  //! The data type, color or numeric, of the texture being edited
  ReTexture::ReTextureDataType textureDataType;

  //! The texture being edited
  ReTexturePtr masterTexture;

  ReTextureChannelDataModelPtr textureChannelModel;

public:
  //! Constructor
  //! \param owner The parent widget
  //! \param texPreview The texture preview widget on screen used to show
  //!                   and edit the texture
  //! \param mat The material that owns the texture
  //! \param textureDataType Whether we allow the creation of color or
  //!                        numeric textures
  //! \param masterTexture If this parameter is not null then it means 
  //!                      that the adapter is used for editing inside 
  //!                      the Texture Editor. If the parameter is Null
  //!                      then it means that the adapter is linked to
  //!                      a texture avatar hosted in a material editor.
  ReTextureEditorAdapter( QWidget* owner,
                          ReTextureAvatar* texPreview, 
                          ReMaterial* mat,
                          ReTexture::ReTextureDataType textureDataType,
                          ReTextureChannelDataModelPtr texChannelModel,
                          ReTexturePtr masterTexture = ReTexturePtr() ) :
    owner(owner),
    texPreview(texPreview),
    mat(mat),
    textureDataType(textureDataType),
    masterTexture(masterTexture),
    textureChannelModel(texChannelModel)
  {
    setConnections();
  };

  void setConnections() {
    connect(texPreview, SIGNAL(addNewTexture(const QString&, const ReTextureType)), 
            this,       SLOT(addNewTextureHandler(const QString&, const ReTextureType)));
    connect(texPreview, SIGNAL(addNewImageMap(const QString&)), 
            this,       SLOT(addNewImageMapHandler(const QString&)));
    // Linking of texture
    connect(texPreview, SIGNAL(linkTexture(const QString&, const QString&)), 
            this,       SLOT(linkHandler(const QString&, const QString&)));
    // UnLinking of texture
    connect(texPreview, SIGNAL(unlinkTexture(const QString&)), 
            this,       SLOT(unlinkHandler(const QString&)));

    // Forward the editTexture signal to the owner when the user clicks on the
    // Texture Preview
    connect(texPreview, SIGNAL(editTexture(ReTexturePtr, QString&)), 
            owner, SIGNAL(editTexture(ReTexturePtr, QString&)));

    // Triggered when the user creates a new texture or links to an existing texture
    connect(this, SIGNAL(editTexture(ReTexturePtr, QString&)), 
            owner, SIGNAL(editTexture(ReTexturePtr, QString&)));

    //! Forward the directEdit signal to the owner
    //! \params #1 is the pointer to the texture 
    //! \params #2 is the channel identifying the texture in the material
    connect(texPreview, SIGNAL(directEdit(ReTexturePtr, QString&)), 
            this, SLOT(directEditHandler(ReTexturePtr, QString&)));

    if (hasMasterTexture()) {
      // Forward the signal via the parent when a new texture is created for
      // one of the master texture. The new object is a sub-texture.
      //  - objectID
      //  - materialID
      //  - channelName
      //  - masterTextureName
      //  - newTextureName
      //  - textureType
      //  - dataType
      connect(this, SIGNAL(makeNewTexture(const QString& /* objectID    */,
                                          const QString& /* materialID  */, 
                                          const QString& /* channelName */,
                                          const QString& /* masterTextureName */,
                                          const QString& /* newTextureName */,
                                          const ReTextureType /*textureType */,
                                          const ReTexture::ReTextureDataType /* dataType */)), 
              owner, SIGNAL(makeNewTexture(const QString&,
                                           const QString&, 
                                           const QString&, 
                                           const QString&, 
                                           const QString&, 
                                           const ReTextureType,
                                           const ReTexture::ReTextureDataType)));      
    }
    else {
      // Forward the signal via the parent when a new texture is created
      // for one of the channels.
      //  - objectID
      //  - materialID
      //  - channelName
      //  - textureType
      //  - dataType
      connect(this, SIGNAL(makeNewTexture(const QString&,
                                          const QString&, 
                                          const QString&, 
                                          const ReTextureType,
                                          const ReTexture::ReTextureDataType)), 
              owner, SIGNAL(makeNewTexture(const QString&,
                                           const QString&, 
                                           const QString&, 
                                           const ReTextureType,
                                           const ReTexture::ReTextureDataType)));
      // Forward the signal about a texture's property being updated
      connect(this, SIGNAL(materialUpdated(QString, 
                                           QString,
                                           QString,
                                           QVariant)),
              owner, SIGNAL(materialUpdated(QString, 
                                            QString,
                                            QString,
                                            QVariant)));
      // Forward the signal via the parent
      connect(this, SIGNAL(linkTexture(const QString&,
                                       const QString&, 
                                       const QString&, 
                                       const QString&)), 
              owner, SIGNAL(linkTexture(const QString&,
                                        const QString&, 
                                        const QString&, 
                                        const QString&)));
    }

    // Forward the signal via the parent
    if (hasMasterTexture()) {
      connect(this, SIGNAL(unlinkTexture(const QString&,
                                         const QString&, 
                                         const QString&, 
                                         const QString&)),
              owner, SIGNAL(unlinkTexture(const QString&,
                                          const QString&, 
                                          const QString&, 
                                          const QString&)));
      
      // Forward the signal via the parent
      connect(this, SIGNAL(replaceTexture(const QString& /*channelID*/,
                                          const QString& /*jsonData*/, 
                                          const QString& /*masterTextureName*/)),
              owner, SIGNAL(replaceTexture(const QString& /*channelID*/,
                                           const QString& /*jsonData*/,
                                           const QString& /*masterTextureName*/)));
    }
    else {
      #ifndef NDEBUG
      RE_LOG_INFO() << "Connecting the delete signal to " 
                    << owner->objectName()
                    << " for " 
                    << texPreview->getID();
      #endif
      connect(this, SIGNAL(unlinkTexture(const QString&,
                                         const QString&, 
                                         const QString&)),
              owner, SIGNAL(unlinkTexture(const QString&,
                                          const QString&, 
                                          const QString&)));
      // Forward the signal via the parent
      connect(this, SIGNAL(replaceTexture(const QString& /*channelID*/,
                                          const QString& /*jsonData*/)), 
              owner, SIGNAL(replaceTexture(const QString& /*channelID*/,
                                           const QString& /*jsonData*/)));
    }

    // Copy/Paste
    connect(texPreview, SIGNAL(copyTexture(const ReTexturePtr&)), 
            this, SLOT(copyTexture(const ReTexturePtr&)));
    connect(texPreview, SIGNAL(pasteTexture(ReTexturePtr&)), 
            this, SLOT(pasteTexture(ReTexturePtr&)));
  }

  inline void setMaterial( ReMaterial* newMat ) {
    mat = newMat;
  }

  inline void setMasterTexture( ReTexturePtr tex ) {
    masterTexture = tex;
  }

  inline bool hasMasterTexture() {
    return !masterTexture.isNull();
  }

  inline void setTextureChannelModel( ReTextureChannelDataModelPtr tcm ) {
    textureChannelModel = tcm;    
  }

signals:
  //! Emitted when the user adds a new texture to a material channel
  void makeNewTexture( const QString& objectID,
                       const QString& materialID, 
                       const QString& channelName, 
                       const ReTextureType textureType,
                       const ReTexture::ReTextureDataType dataType );

  //! Emitted when the user adds a new texture to a master texture
  void makeNewTexture( const QString& objectID,
                       const QString& materialID, 
                       const QString& channelName, 
                       const QString& masterTextureName,
                       const QString& newTextureName,
                       const ReTextureType textureType,
                       const ReTexture::ReTextureDataType dataType );

  //! Emitted when the user starts editing a texture
  void editTexture( ReTexturePtr tex, QString& breadcrumb );

  //! Emitted when the user links an existing texture to a material's channel
  void linkTexture( const QString& objectID,
                    const QString& materialID, 
                    const QString& channelID, 
                    const QString& textureName );
  //! Emitted when the user detaches a texture from a material's channel
  void unlinkTexture( const QString& objectID,
                      const QString& materialID, 
                      const QString& channelID );

  //! Emitted when the user detaches a texture from a material's channel
  void unlinkTexture( const QString& objectID,
                      const QString& materialID, 
                      const QString& textureName, 
                      const QString& channelID );
  //! Emitted when a component of a material has been edited
  void materialUpdated( QString objectID, 
                        QString materialName, 
                        QString propertyName, 
                        QVariant value );

  //! Emitted every time a change to the texture is applied. This is used by
  //! the client to post-process changes to the texture. For example, the
  //! material preview can be updated in response to this signal.
  void textureModified();

  //! Emitted when the user pastes a texture and the same change needs to be 
  //! applied to the host-side copy of the same material and texture.
  void replaceTexture(const QString& channelID, const QString& jsonTextureData);

  //! Version for replacing a subtexture
  void replaceTexture(const QString& channelID, 
                      const QString& jsonTextureData,
                      const QString& masterTextureName);

private slots:

  void addNewTextureHandler(const QString& channelID, 
                            const ReTextureType texType) {
    QString breadCrumb;
    ReTexturePtr newTexture;
    // Is this a sub-texture?
    if (!masterTexture.isNull()) {
      QString masterTextureName = masterTexture->getName();
      newTexture = ReTexturePtr(
                     ReTextureCreator::createTexture(
                       QString("%1_%2")
                         .arg(masterTextureName)
                         .arg(channelID),
                       texType,
                       mat,
                       textureDataType
                     )
                   );
      mat->addTextureToCatalog(newTexture);
      masterTexture.dynamicCast<ReComplexTexture>()->setChannel(channelID, newTexture);

      texPreview->setTexture(newTexture, mat);
      breadCrumb = texPreview->getBreadcrumb();      
      // This signal goes to the host-side part of Reality to apply the
      // change to that portion of the data
      emit makeNewTexture( mat->getParent()->getInternalName(), 
                           mat->getName(), 
                           channelID, 
                           masterTextureName,
                           newTexture->getName(),
                           texType,
                           textureDataType);
    }
    else {
      // We are adding a texture to one of the material's channels
      if ( mat->makeNewTexture( channelID, texType, textureDataType ) ) {
        newTexture = mat->getChannel(channelID);
        texPreview->setTexture(newTexture, mat);
        breadCrumb = texPreview->getBreadcrumb();      
        // This signal goes to the host-side part of Reality to apply the
        // change to that portion of the data
        emit makeNewTexture( mat->getParent()->getInternalName(), 
                             mat->getName(), 
                             channelID, 
                             texType,
                             textureDataType);
      }
    }
    // This signal tells the texture editor to start editing
    emit editTexture(newTexture, breadCrumb);
    // This signal tells the client to do whatever is necessary to 
    // post-process the event.
    emit textureModified();    
  }

  void addNewImageMapHandler(const QString& channelID) {
    // Ask the user for the file name for the image map
    auto config = RealityBase::getConfiguration();
    QString lastBroswedDir = config->value(RE_CFG_LAST_BROWSED_DIR).toString();
    QString imageFileName = QFileDialog::getOpenFileName(
                              owner, 
                              tr("Select an image map"),
                              lastBroswedDir,
                              tr("Images (*.png *.jpg *.tif *.tiff)")
                            );
    if (imageFileName.isEmpty()) {
      return;
    }
    QString breadCrumb;
    QString objectID = mat->getParent()->getInternalName();
    QString matName = mat->getName();

    ImageMapPtr newTex;
    // Is this a sub-texture?
    if (!masterTexture.isNull()) {
      QString masterTextureName = masterTexture->getName();
      newTex = ReTexturePtr(
                 ReTextureCreator::createTexture(
                   QString("%1_%2")
                     .arg(masterTextureName)
                     .arg(channelID),
                   TexImageMap,
                   static_cast<ReTextureContainer*>(mat),
                   textureDataType
                 )
               ).staticCast<ImageMap>();
      newTex->setFileName(imageFileName);
      QString newTextureName = newTex->getName();
      mat->addTextureToCatalog(newTex);
      auto tex = masterTexture.dynamicCast<ReComplexTexture>();
      tex->setChannel(channelID, newTex);
      texPreview->setTexture(newTex, mat);
      breadCrumb = texPreview->getBreadcrumb();
      // This signal goes to the host-side part of Reality to apply the
      // change to that portion of the data
      emit makeNewTexture( objectID, 
                           matName, 
                           channelID,
                           masterTextureName,
                           newTextureName,
                           TexImageMap,
                           textureDataType);

    }
    else {
      // This is a texture attached to a channel
      if ( mat->makeNewTexture( channelID, TexImageMap, textureDataType ) ) {
        newTex = mat->getChannel(channelID).staticCast<ImageMap>();
        newTex->setFileName(imageFileName);
        texPreview->setTexture(newTex, mat);
        breadCrumb = texPreview->getBreadcrumb();      
        emit makeNewTexture( objectID, 
                             matName, 
                             channelID, 
                             TexImageMap,
                             textureDataType);
      }
    }
    // Communicate to the host-side the change
    textureChannelModel->setTexture(newTex);
    textureChannelModel->setNamedValue("fileName", imageFileName);

    // This signal tells the texture editor to start editing
    emit editTexture(newTex, breadCrumb);      
    // This signal tells the client to do whatever is necessary to 
    // post-process the event.
    emit textureModified();
  }

  void linkHandler(const QString& channelID, const QString& textureName) {
    QString breadCrumb;
    auto newTexture = mat->getTexture(textureName);
    if (newTexture.isNull()) {
      return;
    }
    if (!masterTexture.isNull()) {
      masterTexture.dynamicCast<ReComplexTexture>()->setChannel(
        channelID, newTexture
      );
    }
    else {
      mat->setChannel(channelID, textureName);
    }
    texPreview->setTexture(newTexture, mat);
    breadCrumb = texPreview->getBreadcrumb();      
    emit linkTexture( mat->getParent()->getInternalName(), 
                      mat->getName(), 
                      channelID, 
                      textureName );
    emit editTexture(newTexture, breadCrumb);
    // This signal tells the client to do whatever is necessary to 
    // post-process the event.
    emit textureModified();
  }

  void unlinkHandler( const QString& channelID ) {
    if (hasMasterTexture()) {
      // If we get the message to unlink a texture when a master texture
      // is present then the master texture is a compound texture.
      auto compTex = masterTexture.dynamicCast<ReComplexTexture>();
      auto goneTex = compTex->getChannel(channelID);
      if (!goneTex.isNull()) {
        compTex->deleteChannelTexture(channelID);
        emit unlinkTexture( mat->getParent()->getInternalName(), 
                            mat->getName(),
                            masterTexture->getName(),
                            channelID );        
      }
      texPreview->setBlank();
    }
    else {
      // Disconnect the texture from the channel
      mat->setChannel(channelID, "");
      // Blank the texture preview
      texPreview->setTexture(ReTexturePtr(), mat);
      emit unlinkTexture( mat->getParent()->getInternalName(), 
                          mat->getName(), 
                          channelID );
    }
    // This signal tells the client to do whatever is necessary to 
    // post-process the event.
    emit textureModified();
  }
  
  bool isGreyScale( const QColor& c ) {
    int r,g,b;
    c.getRgb(&r, &g, &b);
    return (r == g && g == b);
  }

  void directEditHandler( ReTexturePtr tex, QString& /* channelID */ ) {
    if (tex.isNull()) {
      return;
    }
    if (textureChannelModel.isNull()) {
      RE_LOG_INFO() << "The texture model is null!";
      return;
    }
    textureChannelModel->setTexture(tex);
    switch(tex->getType()) {
      case TexConstant: {
        QColor tmpColor = QColorDialog::getColor(
                            tex.staticCast<ReConstant>()->getColor(),
                            owner,
                            tr("Select the new color for the texture")
                          );
        if (tmpColor.isValid()) {
          if (textureDataType == ReTexture::numeric && !isGreyScale(tmpColor)) {
            // Convert the color to a weighted average: R*0.21, G*0.72, B*0.7
            float newRGB = (tmpColor.red()*0.21 + tmpColor.green()*0.72 + tmpColor.blue()*0.7) /3;
            tmpColor.setRgb(newRGB,newRGB,newRGB);
          }
          tex->setNamedValue("color", tmpColor);
          textureChannelModel->setNamedValue("color", tmpColor);
        }
        break;
      }
      case TexImageMap: {
        ReSetImageMapAction setImageMap(
          tex->getNamedValue("fileName").toString()
        );
        setImageMap.execute();
        if (setImageMap.hasNameChanged()) {
          tex->setNamedValue("fileName", setImageMap.getFileName());
          textureChannelModel->setNamedValue("fileName", setImageMap.getFileName());
        }
        break;
      }
      default: {
        RE_LOG_INFO() << QString("Direct editing of texture %1 not supported") 
                           .arg(tex->getName()).toStdString();
      }
    }
    owner->update();
    emit textureModified();    
  }

  void copyTexture(const ReTexturePtr& texture) {
    auto exporter = ReQtTextureExporterFactory::getExporter(texture);
    auto clipBoard = QApplication::clipboard();
    // Find if this texture depends on other sub-textures. If that's the case
    // then export all those textures and add them to the "deps" element.
    auto depList = texture->getDependencies();
    auto textureData = exporter->exportTexture(texture.data());
    QVariantList deps;
    for (int i = 0; i < depList.count(); i++) {
      auto depTex = mat->getTexture(depList[i]);
      if (depTex.isNull()) {
        continue;
      }
      auto depExporter = ReQtTextureExporterFactory::getExporter(depTex);
      deps << depExporter->exportTexture(depTex.data());
    }
    QVariantMap textureBundle;
    // The texture itself
    textureBundle["texture"] = textureData;
    // The sub-textures
    textureBundle["deps"] = deps;
    textureBundle["sourceMat"] = texture->getParent()->getName();
    textureBundle["sourceObj"] = static_cast<ReMaterial*>(texture->getParent())->getParent()->getInternalName();

    QJson::Serializer json;
    QString textTexture = json.serialize(textureBundle);

    QMimeData* mimeData = new QMimeData();
    // This is the real texture data that we will use for pasting
    mimeData->setData(RE_TEX_CLIPBOARD_MIME_TYPE, textTexture.toUtf8());
    // This is to make the data available to other apps, like text editors
    mimeData->setText(textTexture);
    clipBoard->setMimeData(mimeData);
    #ifndef NDEBUG
      RE_LOG_INFO() << "Texture: " << textTexture;
    #endif
  };


  void pasteTexture(ReTexturePtr& texture) {
    // Retrieve the data form the clipboard, check if there is a Reality texture
    // in it and then convert it into the QVariantMap format, ready to be 
    // added to the current material.
    auto clipboard = QApplication::clipboard();
    auto mimeData = clipboard->mimeData();
    QString format = mimeData->formats()[0];
    if (format != RE_TEX_CLIPBOARD_MIME_TYPE) {
      return;
    }
    QString objectID = mat->getParent()->getInternalName();
    QString matName = mat->getName();
    QString channelID = texPreview->getID();
    if (!texture.isNull()) {
      auto choice = QMessageBox::question(
                      QApplication::activeWindow(),
                      tr("Texture overwriting confirmation"), 
                      tr("Are you sure that you want to replace"
                         " the current texture with the one in the clipboard?"),
                      QMessageBox::Yes | QMessageBox::No,
                      QMessageBox::No
                    );
      if (choice == QMessageBox::No) {
        return;
      }
    }
    QString jsonTextureData = mimeData->data(RE_TEX_CLIPBOARD_MIME_TYPE);
    if (hasMasterTexture()) {
      mat->replaceTexture(channelID, jsonTextureData, masterTexture);
      emit replaceTexture(channelID, jsonTextureData, masterTexture->getName());
    }
    else {
      mat->replaceTexture(channelID, jsonTextureData);
      emit replaceTexture(channelID, jsonTextureData);
    }
    texPreview->setTexture(hasMasterTexture() 
                           ? masterTexture.dynamicCast<ReComplexTexture>()->getChannel(channelID) 
                           : mat->getChannel(channelID), 
                           mat);
    emit textureModified();    
  }
};


//! Convenience Shared pointer definition
typedef QSharedPointer<ReTextureEditorAdapter> ReTextureEditorAdapterPtr;

} // namespace

#endif
