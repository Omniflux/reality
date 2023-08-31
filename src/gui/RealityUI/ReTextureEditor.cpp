  /**
 * The Reality texture editor.
 */
#include "RealityUI/ReTextureEditor.h"

#include <QMessageBox>

#include "ReLogger.h"


using namespace Reality;

ReTextureEditor::ReTextureEditor(QWidget* parent) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  pageNames[TexImageMap]       = ReTextureTypesForUI[TexImageMap];
  pageNames[TexConstant]       = ReTextureTypesForUI[TexConstant];
  pageNames[TexColorMath]      = ReTextureTypesForUI[TexColorMath];
  pageNames[TexMath]           = ReTextureTypesForUI[TexMath];
  pageNames[TexBand]           = ReTextureTypesForUI[TexBand];
  pageNames[TexBricks]         = ReTextureTypesForUI[TexBricks];
  pageNames[TexCheckers]       = ReTextureTypesForUI[TexCheckers];
  pageNames[TexClouds]         = ReTextureTypesForUI[TexClouds];
  pageNames[TexFBM]            = ReTextureTypesForUI[TexFBM];
  // pageNames[TexFresnelColor]   = ReTextureTypesForUI[TexFresnelColor];
  pageNames[TexMarble]         = ReTextureTypesForUI[TexMarble];
  pageNames[TexMix]            = ReTextureTypesForUI[TexMix];
  // pageNames[TexMultiMix]       = ReTextureTypesForUI[TexMultiMix];
  pageNames[TexDistortedNoise] = ReTextureTypesForUI[TexDistortedNoise];
  pageNames[TexWood]           = ReTextureTypesForUI[TexWood];
  // pageNames[TexInvertMap]      = ReTextureTypesForUI[TexInvertMap];
  pageNames[TexGrayscale]      = ReTextureTypesForUI[TexGrayscale];

  cbTextureType->addItems(pageNames.values());

  // Handle the change of texture type
  connect(cbTextureType, SIGNAL(activated(const QString&)), 
          this,          SLOT(changeTextureType(const QString&)));

  ReAbstractTextureEditor* editorList[] = { teImageMap, 
                                            teColorMath,
                                            teMath,
                                            teBand,
                                            teBricks,
                                            teMix,
                                            teWood };
  int editorListLength = sizeof(editorList) / sizeof(ReAbstractTextureEditor*);
  for (int i = 0; i < editorListLength; i++) {
    connect(editorList[i], SIGNAL(replaceTexture(const QString&,const QString&,const QString&)),
             this, SIGNAL(replaceTexture(const QString&,const QString&,const QString&)));
  }
  /**
   * ColorMath texture
   */

  // Recursive editing of ColorMath textures. The signal will be used
  // to switch the texture edit to edit the embedded color texture
  // in the ColorMath texture
  connect(teColorMath, SIGNAL(editTexture( ReTexturePtr, QString&)),
          this,        SLOT(emitEdit( ReTexturePtr, QString&)));

  // Forward signals to the reality panel class
  connect(teColorMath, SIGNAL(makeNewTexture( const QString&, 
                                              const QString&, 
                                              const QString&,
                                              const QString&,
                                              const QString&,
                                              const ReTextureType ,
                                              const ReTexture::ReTextureDataType )),
                this,  SIGNAL(makeNewTexture( const QString&, 
                                              const QString&, 
                                              const QString&,
                                              const QString&,
                                              const QString&,
                                              const ReTextureType ,
                                              const ReTexture::ReTextureDataType )));
  // Forward signals to the reality panel class
  // Parameters:
  //   - objectID
  //   - materialName
  //   - channelName
  //   - masterTexture
  //   - subTexture
  connect(teColorMath, SIGNAL(linkTexture( const QString&, 
                                           const QString&, 
                                           const QString&,
                                           const QString&,
                                           const QString& )),
                this,  SIGNAL(linkTexture( const QString&, 
                                           const QString&, 
                                           const QString&,
                                           const QString&,
                                           const QString& )));
  //! Delete the texture when the user selects to do so from
  //! the ColorMath editor.
  connect(teColorMath, SIGNAL(unlinkTexture( const QString&, 
                                             const QString&, 
                                             const QString&,
                                             const QString& )),
          this,  SIGNAL(unlinkTexture( const QString&, 
                                       const QString&, 
                                       const QString&,
                                       const QString& )));

                                       
  connect(teMath, SIGNAL(editTexture( ReTexturePtr, QString&)),
          this, SLOT(emitEdit( ReTexturePtr, QString&)));

  /**
   * Band texture
   */
  connect(teBand, SIGNAL(editTexture( ReTexturePtr, QString&)),
          this,   SLOT(emitEdit( ReTexturePtr, QString& )));
  // Forward signals to the reality panel class
  connect(teBand, SIGNAL(makeNewTexture( const QString&, 
                                         const QString&, 
                                         const QString&,
                                         const QString&,
                                         const QString&,
                                         const ReTextureType ,
                                         const ReTexture::ReTextureDataType )),
          this,  SIGNAL(makeNewTexture( const QString&, 
                                        const QString&, 
                                        const QString&,
                                        const QString&,
                                        const QString&,
                                        const ReTextureType ,
                                        const ReTexture::ReTextureDataType )));
  //! Delete the texture when the user selects to do so from
  //! the ColorMath editor.
  connect(teBand, SIGNAL(unlinkTexture( const QString&, 
                                             const QString&, 
                                             const QString&,
                                             const QString& )),
          this,  SIGNAL(unlinkTexture( const QString&, 
                                       const QString&, 
                                       const QString&,
                                       const QString& )));

  /**
   * Mix texture
   */
  connect(teMix, SIGNAL(editTexture( ReTexturePtr, QString&)),
          this,  SLOT(emitEdit( ReTexturePtr, QString& )));

  //! Forward signals to the reality panel class
  //! This signal is fired when a texture editor creates a new sub-texture
  //! Meaning of the parameters:
  //! - objectID
  //! - materialName
  //! - channelID
  //! - materTextureName
  //! - newTextureName
  //! - textureType
  //! - textureDataType

  connect(teMix, SIGNAL(makeNewTexture( const QString&, 
                                        const QString&, 
                                        const QString&,
                                        const QString&,
                                        const QString&,
                                        const ReTextureType ,
                                        const ReTexture::ReTextureDataType )),
          this,  SIGNAL(makeNewTexture( const QString&, 
                                        const QString&, 
                                        const QString&,
                                        const QString&,
                                        const QString&,
                                        const ReTextureType ,
                                        const ReTexture::ReTextureDataType )));

  // Forward signals to the reality panel class
  //! This signal is fired when a texture editor links a subtexture to an
  //! existing texture
  //! Parameters:
  //!   - objectID
  //!   - materialName
  //!   - channelName
  //!   - masterTexture
  //!   - subTexture
  connect(teMix, SIGNAL(linkTexture( const QString&, 
                                     const QString&, 
                                     const QString&,
                                     const QString&,
                                     const QString& )),
          this,  SIGNAL(linkTexture( const QString&, 
                                     const QString&, 
                                     const QString&,
                                     const QString&,
                                     const QString& )));


  //! Delete the texture when the user selects to do so from
  //! the ColorMath editor.
  connect(teMix, SIGNAL(unlinkTexture( const QString&, 
                                       const QString&, 
                                       const QString&,
                                       const QString& )),
          this,  SIGNAL(unlinkTexture( const QString&, 
                                       const QString&, 
                                       const QString&,
                                       const QString& )));

  /**
   * Grayscale texture
   */
  // Forward signals to the reality panel class
  //! This signal is fired when a texture editor removes a subtexture from an
  //! existing texture
  //! Parameters:
  //! - objectID,
  //! - materialID
  //! - textureName
  //! - channelID 
  connect(teGrayscale, SIGNAL(unlinkTexture( const QString&, 
                                             const QString&, 
                                             const QString&,
                                             const QString& )),
          this,  SIGNAL(unlinkTexture( const QString&, 
                                       const QString&, 
                                       const QString&,
                                       const QString& )));

  connect(teGrayscale, SIGNAL(editTexture( ReTexturePtr, QString&)),
          this,  SLOT(emitEdit( ReTexturePtr, QString&)));

  //! Forward signals to the reality panel class
  //! This signal is fired when a texture editor creates a new sub-texture
  //! Meaning of the parameters:
  //! - objectID
  //! - materialName
  //! - channelID
  //! - materTextureName
  //! - newTextureName
  //! - textureType
  //! - textureDataType
  connect(teGrayscale, SIGNAL(makeNewTexture( const QString&, 
                                              const QString&, 
                                              const QString&,
                                              const QString&,
                                              const QString&,
                                              const ReTextureType ,
                                              const ReTexture::ReTextureDataType )),
          this,  SIGNAL(makeNewTexture( const QString&, 
                                        const QString&, 
                                        const QString&,
                                        const QString&,
                                        const QString&,
                                        const ReTextureType ,
                                        const ReTexture::ReTextureDataType )));
  /**
   * Bricks texture
   */
  connect(teBricks, SIGNAL(editTexture( ReTexturePtr, QString&)),
          this,  SLOT(emitEdit( ReTexturePtr, QString&)));
  /**
   * Checkers texture
   */
  connect(teCheckers, SIGNAL(editTexture( ReTexturePtr, QString&)),
          this,  SLOT(emitEdit( ReTexturePtr, QString&)));

  /**
   * ImageMap texture
   */
  //! Forward the updateModifiers() signal when a texture is flagged/unflagged
  //! as being a normal map
  connect(teImageMap, SIGNAL(updateModifiers()), this, SIGNAL(updateModifiers()));

  // Breadcrumbs: pressing the back button
  // connect(breadCrumbs, SIGNAL(back(ReTexturePtr)), this, SLOT(startEditor(ReTexturePtr)));
  connect(breadCrumbs, SIGNAL(back(ReTexturePtr)), this, SIGNAL(previousBreadcrumb(ReTexturePtr)));

  // Breadcrumbs: setting the right page when backing up past the last texture
  connect(breadCrumbs, SIGNAL(atHome()), this, SLOT(showHelpPage()));
}

QSize ReTextureEditor::sizeHint() const {
  return QSize(404, 677);
}

void ReTextureEditor::showHelpPage() const {
  breadCrumbs->hide();
  frTextureType->hide();
  pages->setCurrentWidget(helpPage);
}

void ReTextureEditor::resetBreadcrumbs() {
  breadCrumbs->clearTrail();
}

void ReTextureEditor::emitEdit(ReTexturePtr tex, QString& breadcrumb) {
  emit editTexture(tex, breadcrumb, false);
}


void ReTextureEditor::showPage( const ReTextureType pType ) const {
  breadCrumbs->show();
  frTextureType->show();
  switch( pType ) {
    case Reality::TexImageMap:
      pages->setCurrentWidget(imageMapPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexImageMap]));
      break;
    case Reality::TexConstant:
      pages->setCurrentWidget(colorPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexConstant]));
      break;
    case Reality::TexMath:
      pages->setCurrentWidget(mathPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexMath]));      
      break;
    case Reality::TexColorMath:
      pages->setCurrentWidget(colorMathPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexColorMath]));      
      break;
    case Reality::TexClouds:
      pages->setCurrentWidget(cloudsPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexClouds]));      
      break;
    case Reality::TexBand:
      pages->setCurrentWidget(bandPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexBand]));      
      break;
    case Reality::TexBricks:
      pages->setCurrentWidget(bricksPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexBricks]));      
      break;
    case Reality::TexFBM:
      pages->setCurrentWidget(fbmPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexFBM]));
      break;
    case Reality::TexMarble:
      pages->setCurrentWidget(marblePage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexMarble]));
      break;
    case Reality::TexWood:
      pages->setCurrentWidget(woodPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexWood]));
      break;
    case Reality::TexCheckers:
      pages->setCurrentWidget(checkersPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexCheckers]));
      break;
    case Reality::TexDistortedNoise:
      pages->setCurrentWidget(distortedNoisePage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexDistortedNoise]));
      break;
    case Reality::TexMix:
      pages->setCurrentWidget(mixPage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexMix]));      
      break;
    case Reality::TexGrayscale:
      pages->setCurrentWidget(grayscalePage);
      cbTextureType->setCurrentIndex(cbTextureType->findText(pageNames[Reality::TexGrayscale]));      
      break;
    case TexMultiMix:
    case TexFresnelColor:
    case TexInvertMap:
    case TexUndefined:
      // Nothing
      break;
  }
}

void ReTextureEditor::changeTextureType( const QString& newTextureType ) {

  // bool ok = false;
  ReTextureType newType;
  if (newTextureType == pageNames[Reality::TexImageMap]) {
    newType = TexImageMap;
  }
  else if (newTextureType == pageNames[Reality::TexConstant]) {
    newType = TexConstant;
  }
  else if (newTextureType == pageNames[Reality::TexMath]) {
    newType = TexMath;
  }
  else if (newTextureType == pageNames[Reality::TexColorMath]) {
    newType = TexColorMath;
  }
  else if (newTextureType == pageNames[Reality::TexClouds]) {
    newType = TexClouds;
  }
  else if (newTextureType == pageNames[Reality::TexBand]) {
      newType = TexBand;
  }
  else if (newTextureType == pageNames[Reality::TexBricks]) {
    newType = TexBricks;
  }
  else if (newTextureType == pageNames[Reality::TexFBM]) {
    newType = TexFBM;
  }
  else if (newTextureType == pageNames[Reality::TexMarble]) {
    newType = TexMarble;
  }
  else if (newTextureType == pageNames[Reality::TexWood]) {
    newType = TexWood;
  }
  else if (newTextureType == pageNames[Reality::TexCheckers]) {
    newType = TexCheckers;
  }
  else if (newTextureType == pageNames[Reality::TexDistortedNoise]) {
    newType = TexDistortedNoise;
  }
  else if (newTextureType == pageNames[Reality::TexMix]) {
    newType = TexMix;
  }
  else if (newTextureType == pageNames[Reality::TexGrayscale]) {
    newType = TexGrayscale;
  }
  else{
    return;
  }

  ReTexture::ReTextureDataType oldType = model->getTextureDataType();

  quint8 oldTypeMask;
  switch( oldType ) {
    case ReTexture::color:
      oldTypeMask = RE_TEX_DATATYPE_COLOR;
      break;
    case ReTexture::numeric:
      oldTypeMask = RE_TEX_DATATYPE_NUMERIC;
      break;
    case ReTexture::fresnel:
      oldTypeMask = RE_TEX_DATATYPE_FRESNEL;
      break;
  }

  bool compatible = (ReTextureDataTypes[newType] & oldTypeMask);

  if (!compatible) {
    QMessageBox::information(this, 
                             tr("Information"),
                             tr("The selected new texture type is incompatible") );
    update();
    return;
  }

  if (model->changeTextureType(newType)) {
    startEditor();
    emit refreshMaterialView();    
    // Notify the host-app side
    emit textureTypeHasChanged(model->getTexture()->getName(), newType);
  }
  else {
    RE_LOG_DEBUG() << QString("Error: failed to convert texture %1 to %2") 
                        .arg(model->getTexture()->getName())
                        .arg(ReTexture::getTypeAsString(newType))
                        .toStdString();
  }
}


void ReTextureEditor::setModel( ReTextureChannelDataModelPtr _model, 
                                const QString& breadcrumb, 
                                const bool isRoot ) 
{

  // If the parameter isRoot is true then it means that we are editing a texture
  // from the Material Editor. In this case reset the trail of breadcrumbs to the
  // beginning
  if (isRoot) {
    breadCrumbs->clearTrail();
  }
  model = _model;

  if (!breadcrumb.isEmpty()) {
    breadCrumbs->addCrumb(breadcrumb, model->getTexture());
  }
  startEditor();
}

void ReTextureEditor::startEditor() {
  Reality::ReTextureType tType = model->getTextureType();
  textureName->setText(model->getTexture()->getName());

  ReMaterial* mat = static_cast<ReMaterial*>(model->getTexture()->getParent());
  switch( tType ) {
    case Reality::TexImageMap: {
      showPage(Reality::TexImageMap);
      teImageMap->setDataModel(model, mat);
      break;
    }
    case Reality::TexGrayscale: {
      showPage(Reality::TexGrayscale);
      teGrayscale->setDataModel(model, mat);
      break;
    }
    case Reality::TexMath:
      showPage(Reality::TexMath);
      teMath->setDataModel(model, mat);
      break;
    case Reality::TexColorMath: {
      showPage(Reality::TexColorMath);
      teColorMath->setDataModel(model, mat);
      break;
    }
    case Reality::TexConstant: {
      showPage(Reality::TexConstant);
      teColor->setDataModel(model, mat);
      break;
    }
    case Reality::TexClouds: {
      showPage(Reality::TexClouds);
      teClouds->setDataModel(model, mat);
      break;
    }
    case Reality::TexBand: {
      showPage(Reality::TexBand);
      teBand->setDataModel(model, mat);
      break;
    }
    case Reality::TexBricks: {
      showPage(Reality::TexBricks);
      teBricks->setDataModel(model, mat);
      break;
    }
    case Reality::TexFBM: {
      showPage(Reality::TexFBM);
      teFBM->setDataModel(model, mat);
      break;
    }
    case Reality::TexMarble: {
      showPage(Reality::TexMarble);
      teMarble->setDataModel(model, mat);
      break;
    }
    case Reality::TexWood: {
      showPage(Reality::TexWood);
      teWood->setDataModel(model, mat);
      break;
    }
    case Reality::TexCheckers: {
      showPage(Reality::TexCheckers);
      teCheckers->setDataModel(model, mat);
      break;
    }
    case Reality::TexDistortedNoise: {
      showPage(Reality::TexDistortedNoise);
      teDistortedNoise->setDataModel(model, mat);
      break;
    }
    case Reality::TexMix: {
      showPage(Reality::TexMix);
      teMix->setDataModel(model, mat);
      break;
    }
    case Reality::TexMultiMix:
    case Reality::TexInvertMap:
    case Reality::TexUndefined:
    case Reality::TexFresnelColor:
      QMessageBox::information(this,"Information", "Sorry Paolo, I'm afraid I can't do that");
      break;

  }

}

