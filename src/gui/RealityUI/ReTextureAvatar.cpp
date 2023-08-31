/**
 * The Reality texture manager.
 */

#include "RealityUI/ReTextureAvatar.h"

#include <QClipboard>
#include <QMenu>
#include <QFileInfo>
#include <QMessageBox>
#include <QMouseEvent>
#include <QJson/Parser>

#include "ReDefs.h"
#include "ReLogger.h"
#include "ReTexture.h"
#include "ReTextureContainer.h"
#include "RealityUI/ReTextureSelector.h"
#include "textures/ReBand.h"
#include "textures/ReColorMath.h"
#include "textures/ReConstant.h"
#include "textures/ReDistortedNoise.h"
#include "textures/ReGrayscale.h"
#include "textures/ReMath.h"
#include "textures/ReMix.h"
#include "textures/ReFresnelColor.h"
#include "textures/ReImageMap.h"


#define TA_MENU_EDIT      tr("Edit")
#define TA_MENU_EXISTING  tr("Use Existing...")
#define TA_MENU_NEW       tr("New")
#define TA_MENU_DELETE    tr("Detach")
#define TA_MENU_IMAGE_MAP tr("New Image Map")
#define TA_MENU_COPY      tr("Copy")
#define TA_MENU_PASTE     tr("Paste")

using namespace Reality;
// Initialization of static variables

QMenu* ReTextureAvatar::contextMenu      = NULL;
QMenu* ReTextureAvatar::newTextureMenu   = NULL;
QAction* ReTextureAvatar::editAction     = NULL;
QAction* ReTextureAvatar::deleteAction   = NULL;
QAction* ReTextureAvatar::linkAction     = NULL;
QAction* ReTextureAvatar::imageMapAction = NULL;
QAction* ReTextureAvatar::copyAction     = NULL;
QAction* ReTextureAvatar::pasteAction    = NULL;
QPixmap* ReTextureAvatar::textureColor   = NULL;
QPixmap* ReTextureAvatar::textureGS      = NULL;
QHash<QString, ReTextureType> ReTextureAvatar::textureTable;
ReTextureSelector* ReTextureAvatar::textureSelector;

// end static vars

ReTextureAvatar::ReTextureAvatar( QWidget* parent ) : 
  QWidget(parent),
  ID(""),
  breadcrumb(""),
  container(NULL),
  isBlank(true),
  deleteEnabled(false)
{
  setupUi(this);
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
  setContextMenuPolicy(Qt::DefaultContextMenu);

  // We want to allocate only one instance of the menu associated with
  // the edit (gear) button. In this way we save a lot of resources.
  // In a single material there can be several texture avatars and allocating
  // the same menu to each one is simply a wast and it slows down the program
  if (!contextMenu) {
    createMenu();
  }
  // Get notifications when the user clicks on the edit (gear) menu
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(gearMenuClicked()));
  if (textureColor == NULL) {
    textureColor = new QPixmap(":/images/texture-clr.png");
    textureGS = new QPixmap(":/images/texture-gs.png");
  }  
  // By default we allow all texture data types
  allowedDataTypes = RE_TEX_DATATYPE_ALL;
}

ReTextureAvatar::~ReTextureAvatar() {
}

QSize ReTextureAvatar::sizeHint() const {
  return QSize(330, 67);    
}

void ReTextureAvatar::setDeleteEnabled( const bool status ) {
  deleteEnabled = status;
}

bool ReTextureAvatar::hasDeleteEnabled() {
  return deleteEnabled;
}


void ReTextureAvatar::mouseReleaseEvent( QMouseEvent* event ) {
  if (texture.isNull()) {
    return;
  }
  if (event->button() == Qt::LeftButton) {
    // If the user clicked on the texture preview and the texture is
    // of type Constant or ImageMap, and the Alt/Opt key is not 
    // pressed, then direct-edit it.
    switch(texture->getType()) {
      case TexConstant:
      case TexImageMap:
        if (QApplication::keyboardModifiers() & Qt::AltModifier ) {
          emit editTexture(texture, breadcrumb);          
        }
        else {
          emit directEdit(texture, breadcrumb);
        }
        break;
      default:
        emit editTexture(texture, breadcrumb);
    }
  }
}

void ReTextureAvatar::contextMenuEvent( QContextMenuEvent* /* event */) {
  if (!texture.isNull()) {
    RE_LOG_INFO() << "Node name: " << texture->getName().toUtf8().data();
  }
}

void ReTextureAvatar::createMenu() {
  contextMenu = new QMenu(this);
  linkAction     = contextMenu->addAction(TA_MENU_EXISTING);
  editAction     = contextMenu->addAction(TA_MENU_EDIT);
  imageMapAction = contextMenu->addAction(TA_MENU_IMAGE_MAP);
  newTextureMenu = contextMenu->addMenu(TA_MENU_NEW);
  contextMenu->addSeparator();
  copyAction = contextMenu->addAction(TA_MENU_COPY);
  pasteAction = contextMenu->addAction(TA_MENU_PASTE);
  contextMenu->addSeparator();
  deleteAction = contextMenu->addAction(TA_MENU_DELETE);
  // Initializes a lookup table that maps the menu options to
  // the corresponding texture type
  for (int i = TexImageMap; i <= TexUndefined; i++) {
    if ( i == TexMultiMix     || i == TexUndefined || 
         i == TexFresnelColor || i == TexInvertMap ) {
      continue;
    }
    newTextureMenu->addAction(ReTextureTypesForUI[i]);
    textureTable[ReTextureTypesForUI[i]] = static_cast<ReTextureType>(i);
  }  

  // Create the texture selector dialog box
  textureSelector = new ReTextureSelector( this );
}

void ReTextureAvatar::filterMenuOptions() {
  QListIterator<QAction*> actions = newTextureMenu->actions();
  while(actions.hasNext()) {
    QAction* action = actions.next();
    QString actionText = action->text();
    if (textureTable.contains(actionText)) {
      // Retrieve the data type of the texture associated with the action
      ReTextureType acType = textureTable.value(actionText);
      // Obtain the map of data types supported by the texture and compare it
      // against the data types supported by the texture connected to this 
      // avatar. If it matches then the menu is enabled, otherwise it's disabled 
      action->setEnabled(ReTextureDataTypes[acType] & allowedDataTypes);
    }
  }
}

void ReTextureAvatar::gearMenuClicked() {
  // Position the menu just below the button. To do this we
  // need to first get the dimensions of the button...
  QRect r = btnEdit->geometry();

  QPoint origin;
  // Obtain the global screen coordinates relative to the
  // button. 
  origin = btnEdit->mapToGlobal(QPoint(0,0 + r.height()));

  // Adapt the menu to the specific type of texture. If it's empty 
  // then the Edit and Delete items are not necessary. If it links
  // to a texture then the New item is not necessary.
  newTextureMenu->setEnabled(true);
  imageMapAction->setEnabled(true);
  editAction->setEnabled(true);
  deleteAction->setEnabled(deleteEnabled);
  linkAction->setEnabled(container != NULL);

  if (texture.isNull()) {
    editAction->setEnabled(false);
    deleteAction->setEnabled(false);
  }
  else {
    newTextureMenu->setEnabled(false);
    imageMapAction->setEnabled(false);
  }
  // Enable/disable the copy menu depending on whether there is a texture
  // connected with the widget.
  copyAction->setEnabled(hasTexture());

  // Enable/disable the Paste option based on the presence of application/reality-texture
  // data in the clipboard
  auto clipboard = QApplication::clipboard();
  auto mimeData = clipboard->mimeData();
  QStringList formats = mimeData->formats();
  bool mimeTypeFound = false;
  foreach(const QString format, formats) {
    if (format == RE_TEX_CLIPBOARD_MIME_TYPE) {
      mimeTypeFound = true;
      break;
    }
  }
  // Test if the texture data type is compatible with the data type of the 
  // receiving texture slot
  QJson::Parser jsonParser;
  bool parseOk;
  QVariantMap textureBundle = jsonParser.parse(
                                mimeData->data(RE_TEX_CLIPBOARD_MIME_TYPE),
                                &parseOk
                              ).toMap();
  auto textureData = textureBundle.value("texture").toMap();
  bool isDataTypeCompatible = false;
  switch(static_cast<ReTexture::ReTextureDataType>(textureData["dataType"].toInt())) {
    case ReTexture::color:
      if (allowedDataTypes & RE_TEX_DATATYPE_COLOR) {
        isDataTypeCompatible = true;
      };
      break;
    case ReTexture::numeric:
      if (allowedDataTypes & RE_TEX_DATATYPE_NUMERIC) {
        isDataTypeCompatible = true;
      };
      break;
    case ReTexture::fresnel:
      if (allowedDataTypes & RE_TEX_DATATYPE_FRESNEL) {
        isDataTypeCompatible = true;
      };
      break;
  }
  // RE_LOG_INFO() << "Allowed data types: " << (int)allowedDataTypes
  //               << ", " << textureData.value("dataType").toInt();
  pasteAction->setEnabled(mimeTypeFound && isDataTypeCompatible);


  // Get notification when the user selects from then contextual menu
  // We must set the connection here or otherwise the signal will be sent
  // to all the instances of TextureAvatar
  connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(handleContextMenu(QAction*)));

  // Filter the texture that are incompatible
  filterMenuOptions();

  contextMenu->setEnabled(true);
  contextMenu->exec(origin);
  // unplug the signal or we will get notifications when the menu is used for 
  // other instances of texture avatar
  disconnect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(handleContextMenu(QAction*)));
}

void ReTextureAvatar::handleContextMenu(QAction* action) {
  QString actionText = action->text();
  if (actionText == TA_MENU_EDIT) {
    emit editTexture(texture, breadcrumb);    
  }
  else if (actionText == TA_MENU_DELETE) {
    if ( QMessageBox::question( 
           this, 
           tr("Detaching the texture from material"),
           tr("Do you really want to detach this texture from the channel?"),
           QMessageBox::Yes | QMessageBox::No
         ) == QMessageBox::Yes ) {
      texture.clear();
      isBlank = true;
      update();
      emit unlinkTexture(ID);
    }
  }
  else if (actionText == TA_MENU_EXISTING) {
    // ReTextureContainer* cont = texture->getParent();
    ReNodeDictionary textures = container->getTextures();
    QString excludeName = texture.isNull() ? "" : texture->getName();
    textureSelector->setTextureList(textures, excludeName);
    if ( textureSelector->exec() ) {
      QString texName = textureSelector->getSelection();
      emit linkTexture(ID, texName);
    }
  }
  else if (actionText == TA_MENU_IMAGE_MAP) {
    emit addNewImageMap(ID);
  }
  else if (actionText == TA_MENU_COPY) {
    emit copyTexture(texture);
  }
  else if (actionText == TA_MENU_PASTE) {
    emit pasteTexture(texture);
  }
  else if (textureTable.contains(actionText)) {
    emit addNewTexture(ID, textureTable.value(actionText));
  }
}

void ReTextureAvatar::setTexture( const ReTexturePtr newTex, ReTextureContainer* tCont ) {
  isBlank = false;

  if (newTex.isNull()) {
    // qDebug() << "Passed NULL pointer to ReTextureAvatar::setTexture() for " << objectName();
    // return;
    isBlank = true;
  }

  texture = newTex;
  container = tCont;
  update();
};

void ReTextureAvatar::setMixAvatar( QLabel* lbl, QLabel* lbl2, ReMixTexturePtr tex ) {
  ReTexturePtr t1 = tex->getTexture1();
  ReTexturePtr t2 = tex->getTexture2();
  if (!t1.isNull()) {
    setLabelAvatar(t1, lbl);
  }
  if (!t2.isNull() && lbl2) {
    setLabelAvatar(t2, lbl2);
  }
}


void ReTextureAvatar::setDistortedNoiseAvatar( QLabel* lbl , ReDistortedNoisePtr tex ) {
  // lbl->setText(QString("%1/%2")
  //              .arg(tex->getNoiseBasisAsString())
  //              .arg(tex->getDistortionTypeAsString())
  // );
  lbl->setText(tr("Distorted Noise"));
}

void ReTextureAvatar::getColorValues( const QColor& color, QString& colorString, QString& styleSheet ) {
  int r,g,b;
  color.getRgb(&r,&g,&b);  
  colorString = QString("%1:%2:%3")
                  .arg(r,3,10,QLatin1Char('0'))
                  .arg(g,3,10,QLatin1Char('0'))
                  .arg(b,3,10,QLatin1Char('0'));;


  // lbl->setText(clrStr);
  // Calculate the color for the text to be visible given
  // the color of the background
  QColor bgColor = color.toHsv();
  QString textColor;
  if (bgColor.value() < 130) {
    textColor = "#FFF";
  }
  else {
    textColor = "#000";
  }
  styleSheet = QString("background-color: %1; color: %2;")
                     .arg(bgColor.name())
                     .arg(textColor);  
}

void ReTextureAvatar::setColorMathAvatar( QLabel* lbl, QLabel* lbl2, ReColorMathPtr tex ) {
  ReTexturePtr t1 = tex->getTexture1();
  ReTexturePtr t2 = tex->getTexture2();
  // Flag to signal if the second label has been set already. It's tested in the
  // second half of this method.
  bool label2IsSet = false;
  bool functionIsSet = false;
  if (!t1.isNull()) {
    QColor color = tex->getColor1();
    QString clrStr, styleSheet;
    getColorValues(color, clrStr, styleSheet);
    // If the second texture is null then we show the texture as being
    // a mix of the color, in the left label, and the texture1 in the
    //  right label
    if (t2.isNull() && lbl2) {
      lbl->setText(clrStr);
      lbl->setStyleSheet(styleSheet);
      lbl->setAlignment(Qt::AlignCenter);
      cmSign->setText(RE_CM_NONE);
      setLabelAvatar(t1, lbl2);
      // Avoid further modifications to the second label
      label2IsSet = true;
      functionIsSet = true;
    }
    else {
      lbl->setStyleSheet(styleSheet);
      setLabelAvatar(t1, lbl);
    }
  }
  else {
    lbl->setText(tr("<Empty>"));
  }
  // Do we have a second texture?
  if ( !t2.isNull() ) {
    if ( lbl2 ) {
      setLabelAvatar(t2, lbl2);
    }
  }
  else if ( !label2IsSet && lbl2 ) {
    lbl2->setText(tr("<Empty>"));
  }
  if (!functionIsSet) {
    switch (tex->getFunction()) {
      case ReColorMath::add:
        cmSign->setText(RE_CM_ADD);
        break;
      case ReColorMath::subtract:
        cmSign->setText(RE_CM_SUB);
        break;
      case ReColorMath::multiply:
        cmSign->setText(RE_CM_MULT);
        break;
      case ReColorMath::none:
        cmSign->setText("");
        break;
    }
  }
}

void ReTextureAvatar::setMathAvatar( QLabel* lbl, QLabel* lbl2, ReMathPtr tex ) {
  ReTexturePtr t1 = tex->getTexture1();
  ReTexturePtr t2 = tex->getTexture2();
  if (!t1.isNull()) {
    setLabelAvatar(t1, lbl);
  }
  if (!t2.isNull() && lbl2) {
    setLabelAvatar(t2, lbl2);
  }
}

void ReTextureAvatar::setFresnelAvatar( QLabel* lbl , ReFresnelColorPtr tex ) {
  QColor color = tex->getColor();
  ReTexturePtr clrTex = tex->getTexture();
  QString texName;
  if (!clrTex.isNull()) { 
    if (clrTex->getType() == TexImageMap) {
      QString fName = clrTex.staticCast<ReImageMap>()->getFileName();
      texName = QFileInfo(fName).fileName();
      lbl->setToolTip(fName);
    }
    else {
      texName = clrTex->getTypeAsString();
    }
  }
  QString clrStr, styleSheet;
  getColorValues(color, clrStr, styleSheet);

  lbl->setText(QString("Frl: %1").arg(texName.isEmpty() ? clrStr : texName));
  lbl->setStyleSheet(styleSheet);
}

void ReTextureAvatar::setConstantAvatar( QLabel* lbl , ReConstantPtr tex ) {
  QColor color = tex->getColor();
  QString clrStr, styleSheet;
  getColorValues(color, clrStr, styleSheet);

  lbl->setText(clrStr);
  lbl->setStyleSheet(styleSheet);
}  

void ReTextureAvatar::setImageMapAvatar( QLabel* lbl, ReImageMapPtr tex ) {
  QString fName = tex->getFileName();
  if (fName.isEmpty()) {
    lbl->setText(tr("<Empty>"));
  }
  else {
    lbl->setText(QFileInfo(fName).fileName());
  }
  lbl->setToolTip(fName);
}

void ReTextureAvatar::setLabelAvatar( ReTexturePtr tex, QLabel* lbl, QLabel* lbl2 ) {
  // lbl->setStyleSheet("background-color: transparent;");
  if (lbl2) {
    lbl2->setStyleSheet("background-color: transparent;");
  }
  switch( tex->getType() ) {
    case TexImageMap: {
      setImageMapAvatar(lbl, tex.staticCast<ReImageMap>());
      break;
    }
    case TexConstant: {
      setConstantAvatar(lbl, tex.staticCast<ReConstant>());
      break;
    }
    case TexMath:
      setMathAvatar(lbl, lbl2, tex.staticCast<ReMath>());
      break;
    case TexColorMath: {       
      setColorMathAvatar(lbl, lbl2, tex.staticCast<ReColorMath>());
      break;
    }
    case TexBand:{
      const int numBands = 4;
      QColor cols[numBands];
      cols[0].setRgb(149,52,0);
      cols[1].setRgb(251,114,7);
      cols[2].setRgb(254,215,39);
      cols[3].setRgb(254,248,237);

      auto bandTex = tex.staticCast<ReBand>();
      ReTexturePtr colTex;
      for (int i = 0; i < numBands; i++) {
        colTex = bandTex->getNamedValue(QString("tex%1").arg(i+1)).value<ReTexturePtr>();
        if (colTex->getType() == TexConstant) {
          cols[i] = colTex.staticCast<ReConstant>()->getColor();
        }        
      }
      auto offsets = bandTex->getOffsets();
      lbl->setText("Band");
      lbl->setStyleSheet(
        QString("* {background: qlineargradient(spread:pad, x1:0 y1:0, x2:1 y2:0, "
                "stop:%1 %2,"
                "stop:%3 %4,"
                "stop:%5 %6,"
                "stop:%7 %8 )}")
          .arg(offsets["tex1"]).arg(cols[0].name())
          .arg(offsets["tex2"]).arg(cols[1].name())
          .arg(offsets["tex3"]).arg(cols[2].name())
          .arg(offsets["tex4"]).arg(cols[3].name())
      );
      break;
    }
    case TexBricks:{
      lbl->setText("Bricks");
      break;
    }
    case TexCheckers:{
      lbl->setText("Checkers");
      break;
    }
    case TexClouds: {
      if (!lbl) {
        return;
      }
      lbl->setText("Clouds");
      break;
    }
    case TexFBM: {
      lbl->setText("Fractal Noise");
      break;
    }
    case TexFresnelColor: {
      setFresnelAvatar(lbl, tex.staticCast<ReFresnelColor>());
      break;
    }
    case TexMarble: {
      lbl->setText("Marble");
      break;
    }
    case TexMix: {
      setMixAvatar(lbl, lbl2, tex.staticCast<ReMixTexture>());
      break;
    }
    case TexMultiMix: {
      lbl->setText("Multi-Mix");
      break;
    }
    case TexDistortedNoise: {
      setDistortedNoiseAvatar(lbl, tex.staticCast<ReDistortedNoise>());
      break;
    }
   case TexWood: {
     lbl->setText("Wood");
     break;
   }
    case TexInvertMap: {
      lbl->setText("InvertMap");
      break;
    }
    case TexGrayscale: {
      QString fName("<Empty>");
      ReTexturePtr innerTex = tex.staticCast<ReGrayscale>()->getTexture();
      if (!innerTex.isNull()) {
        fName = innerTex->getNamedValue("fileName").toString();
      }
      lbl->setText(QString("B&W: %1").arg(QFileInfo(fName).fileName()));
      break;
    }
    default:
      lbl->setText("<Empty>");
      break;
  }
}

void ReTextureAvatar::updateAvatar() {
  if (isBlank) {
    setTextureTypeLabel("<Empty>");
    textures->setCurrentWidget(generic);
    gtLabel->setText("");
    gtLabel->setStyleSheet("background-color: transparent;");
    // lblTextureName->setVisible(false);
    lblTextureName->setText("");
    return;
  }

  Reality::ReTextureType textureType = texture->getType();

  switch(texture->getDataType()) {
    case ReTexture::color:
      taTextureDataType->setPixmap(*textureColor);
      break;
    case ReTexture::numeric:
      taTextureDataType->setPixmap(*textureGS);
      break;
    case ReTexture::fresnel:
      taTextureDataType->setPixmap(*textureGS);
      break;
  }

  lblTextureName->setVisible(true);
  lblTextureName->setText(texture->getName());
  switch(textureType) {
    case Reality::TexImageMap: {
      textures->setCurrentWidget(imageMap);
      setTextureTypeLabel(tr("Image Map"));
      setLabelAvatar(texture, imText);
      break;
    }
    case Reality::TexMath: {
      textures->setCurrentWidget(colorMath);
      setTextureTypeLabel(tr("Math"));          
      ReMathPtr mt = texture.staticCast<Reality::ReMath>();
      switch(mt->getFunction()) {
        case Reality::ReMath::add:
          cmSign->setText(RE_CM_ADD);
          break;
        case Reality::ReMath::subtract:
          cmSign->setText(RE_CM_MULT);
          break;
        case Reality::ReMath::multiply:
          cmSign->setText(RE_CM_MULT);
          break;
      }
      setLabelAvatar(mt, cmLeftTexture, cmRightTexture);
      break;
    }
    case Reality::TexColorMath: {       
      textures->setCurrentWidget(colorMath);
      setTextureTypeLabel(tr("Texture Combine"));
      ReColorMathPtr mt = texture.staticCast<Reality::ReColorMath>();
      
      switch(mt->getFunction()) {
        case Reality::ReMath::add:
          cmSign->setText(RE_CM_ADD);
          break;
        case Reality::ReMath::subtract:
          cmSign->setText(RE_CM_MULT);
          break;
        case Reality::ReMath::multiply:
          cmSign->setText(RE_CM_MULT);
          break;
        default:
          break;
      }
      setLabelAvatar(mt, cmLeftTexture, cmRightTexture);
      break;
    }
    case Reality::TexBand:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Band"));
      setLabelAvatar(texture, gtLabel);
      break;

    case Reality::TexBricks:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Bricks"));
      setLabelAvatar(texture, gtLabel);
      break;

    case Reality::TexCheckers:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Checkers"));
      setLabelAvatar(texture, gtLabel);
      break;

    case Reality::TexClouds:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Clouds"));
      setLabelAvatar(texture, gtLabel);
      break;

    case Reality::TexConstant:{
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Color"));
      setLabelAvatar(texture, gtLabel);
      break;
    }
    case Reality::TexFBM:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("FBM"));
      setLabelAvatar(texture, gtLabel);
      break;

    case Reality::TexFresnelColor:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Fresnel"));
      setLabelAvatar(texture, gtLabel);
      break;

    case Reality::TexMarble:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Marble"));
      setLabelAvatar(texture, gtLabel);
      break;

    case Reality::TexMix:
      textures->setCurrentWidget(mix);
      setTextureTypeLabel(tr("Mix"));
      setLabelAvatar(texture, mixTextureA, mixTextureB);
      break;

    case Reality::TexMultiMix:
      textures->setCurrentWidget(mix);
      setTextureTypeLabel(tr("Multi-Mix"));
      setLabelAvatar(texture, mixTextureA, mixTextureB);
      break;

    case Reality::TexDistortedNoise:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Distorted noise"));
      setLabelAvatar(texture, gtLabel);
      break;

   case Reality::TexWood:
     textures->setCurrentWidget(generic);
     setTextureTypeLabel(tr("Wood"));
     setLabelAvatar(texture, gtLabel);
     break;

    case Reality::TexInvertMap:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Inverted map"));
      setLabelAvatar(texture, gtLabel);
      break;

    case Reality::TexGrayscale:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Grayscale"));
      setLabelAvatar(texture, gtLabel);
      break;

    default:
      textures->setCurrentWidget(generic);
      setTextureTypeLabel(tr("Unknown"));
      gtLabel->setText("???");
  }

}
