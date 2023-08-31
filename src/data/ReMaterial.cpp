/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReMaterial.h"

#include <QJson/Parser>

#include "ReAcsel.h"
#include "ReGeometryObject.h"
#include "ReLogger.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReTextureCreator.h"
#include "ReTools.h"
#include "ReVolumes.h"
#include "importers/qt/ReQtTextureImporter.h"
#include "importers/qt/ReQtTextureImporterFactory.h"
#include "textures/ReBand.h"
#include "textures/ReBricks.h"
#include "textures/ReCheckers.h"
#include "textures/ReClouds.h"
#include "textures/ReColorMath.h"
#include "textures/ReComplexTexture.h"
#include "textures/ReConstant.h"
#include "textures/ReDistortedNoise.h"
#include "textures/ReFBM.h"
#include "textures/ReFresnelColor.h"
#include "textures/ReGrayscale.h"
#include "textures/ReImageMap.h"
#include "textures/ReInvertMap.h"
#include "textures/ReMarble.h"
#include "textures/ReMath.h"
#include "textures/ReMix.h"
#include "textures/ReWood.h"


namespace Reality {

QString ReMaterial::typeNames[MatUndefined+1] = {
  QString("Glass"),
  QString("Glossy"),
  QString("Hair"),
  QString("Matte"),
  QString("Metal"),
  QString("Mirror"),
  QString("Mix"),
  QString("Null"),
  QString("Skin"),
  QString("Velvet"),
  QString("Water"),
  QString("Light"),
  QString("Cloth"),
  QString("Undefined"),
};


ReMaterial::ReMaterial( const QString name, const ReGeometryObject* parent ) : 
  name(name), 
  type(MatUndefined),
  originalType(MatUndefined),
  parent(parent),
  nullValue(NULL),
  edited(false),
  visibleInRender(true)
{
    // Nothing
}

// Destructor: ~ReMaterial()
ReMaterial::~ReMaterial() {
}

void ReMaterial::fromMaterial( const ReMaterial* bm ) {
  // nothing
}


void ReMaterial::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "name") {
    name = value.toString();
  }
  else if (vname == "type") {
    type = static_cast<ReMaterialType>(value.toInt());
  }
  else if (vname == "innerVolume") {
    innerVolume = value.toString();
  }
  else if (vname == "outerVolume") {
    outerVolume = value.toString();
  }
  else if (vname == "edited") {
    edited = value.toBool();
  }
  else if (vname == "visibleInRender") {
    visibleInRender = value.toBool();
  }
  // Properties of the linked volume
  else if (vname == "absorptionColor") {
    if (innerVolume != "") {
      ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
      if (!vol.isNull()) {
        vol->setColor(value.value<QColor>());
      }
    }
  }  
  else if (vname == "scatteringColor") {
    if (innerVolume != "") {
      ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
      if (!vol.isNull()) {
        vol->setScatteringColor(value.value<QColor>());
      }
    }
  }  
  else if (vname == "absorptionScale") {
    if (innerVolume != "") {
      ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
      if (!vol.isNull()) {
        vol->setAbsorptionScale(value.toFloat());
      }
    }
  }  
  else if (vname == "scatteringScale") {
    if (innerVolume != "") {
      ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
      if (!vol.isNull()) {
        vol->setScatteringScale(value.toFloat());
      }
    }
  }  
  else if (vname == "clarityAtDepth") {
    if (innerVolume != "") {
      ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
      if (!vol.isNull()) {
        vol->setClarityAtDepth(value.toFloat());
      }
    }
  }
  else if (vname == "acselSetID") {
    acselSetID = value.toString();
  }
  else if (vname == "acselSetName") {
    acselSetName = value.toString();
  }
  else if (vname == "acselID") {
    acselID = value.toString();
  }
}

const QVariant ReMaterial::getNamedValue( const QString& vname ) const {
  QVariant val;
  if (vname == "name") {
    return name;
  }
  else if (vname == "type") {
    return type;
  }
  else if (vname == "innerVolume") {
    return innerVolume;
  }
  else if (vname == "outerVolume") {
    return outerVolume;
  }
  else if (vname == "edited") {
    return edited;
  }
  else if (vname == "visibleInRender") {
    return visibleInRender;
  }
  // Properties of the linked volume
  else if (vname == "absorptionColor") {
    return getAbsorptionColor();
  }  
  else if (vname == "scatteringColor") {
    return getScatteringColor();
  }  
  else if (vname == "absorptionScale") {
    return getAbsorptionScale();
  }  
  else if (vname == "scatteringScale") {
    return getScatteringScale();
  }  
  else if (vname == "clarityAtDepth") {
    return getClarityAtDepth();
  }   
  else if (vname == "acselSetID") {
    return getAcselSetID();
  }
  else if (vname == "acselSetName") {
    return acselSetName;
  }
  else if (vname == "acselID") {
    return acselID;
  }

  return val;
}


const QColor& ReMaterial::getAbsorptionColor() const {
  if (innerVolume != "") {
    ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      return vol->getColor();
    }
  }
  return RE_WHITE_COLOR;
}

const QColor& ReMaterial::getScatteringColor() const {
  if (innerVolume != "") {
    ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      return vol->getScatteringColor();
    }
  }
  return RE_WHITE_COLOR;
}

float ReMaterial::getAbsorptionScale() const {
  if (innerVolume != "") {
    ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      return vol->getAbsorptionScale();
    }
  }
  return 0.0f;
};

float ReMaterial::getClarityAtDepth() const {
  if (innerVolume != "") {
    ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      return vol->getClarityAtDepth();
    }
  }
  return 0.0f;
};

float ReMaterial::getScatteringScale() const {
  if (innerVolume != "") {
    ReVolumePtr vol = RealitySceneData->getVolume(innerVolume);
    if (!vol.isNull()) {
      return vol->getScatteringScale();
    }
  }
  return 0.0f;    
};


void ReMaterial::findTextureDependencies( ReTexturePtr tex, 
                                          QHash<QString, bool>& visited, 
                                          QStringList& texList ) const
{
  QString texName = tex->getName();
  QStringList deps = tex->getDependencies();
  if (!visited.contains(texName)) {
    visited[texName] = true;
    foreach( QString tName, deps) {
      findTextureDependencies(nodeCatalog[tName], visited, texList);
    }
    texList.append(texName);
  } 
}

void ReMaterial::findDependencies( QStringList& texList ) const 
{
  QHash<QString, bool> visited;

  ReNodeDictionaryIterator i(nodeCatalog);
  while( i.hasNext() ) {
    i.next();
    ReTexturePtr tex = i.value();
    if ( !tex.isNull() ) {
      findTextureDependencies(tex, visited, texList);
    }
  }
}


const QString ReMaterial::getUniqueName() const {
  return QString("%1:%2")
            .arg(sanitizeMaterialName(getParent()->getInternalName()))
            .arg(sanitizeMaterialName(name));
}

void ReMaterial::setNodeCatalog( const ReNodeDictionary newNodeCatalog ) {
  nodeCatalog.clear();
  nodeCatalog = newNodeCatalog;
};

bool ReMaterial::makeNewTexture( const QString& channelName, 
                                 const ReTextureType textureType,
                                 const ReTexture::ReTextureDataType dataType ) 
{
  // Don't do anything if the target channel doesn't exist already
  if (!channels.contains(channelName)) {
    return false;
  }
  // Don't do anything if a texture already exists in the channel
  if (!channels[channelName].isNull()) {
    return false;
  }

  QString texName;
  switch( textureType ) {
    case TexImageMap: {
      texName = QString("%1_imagemap").arg(channelName);
      break;
    }
    case TexConstant: {
      texName = QString("%1_constant").arg(channelName);
      break;
    }
    case TexColorMath: {
      texName = QString("%1_colormath").arg(channelName);
      break;
    }
    case TexMath: {
      texName = QString("%1_math").arg(channelName);
      break;
    }
    case TexBand: {
      texName = QString("%1_band").arg(channelName);
      break;
    }
    case TexBricks: {
      texName = QString("%1_bricks").arg(channelName);
      break;
    }
    case TexCheckers: {
      texName = QString("%1_checkers").arg(channelName);
      break;
    }
    case TexClouds: {
      texName = QString("%1_clouds").arg(channelName); 
      break;
    }
    case TexFBM: {
      texName = QString("%1_fbm").arg(channelName);
      break;
    }
    case TexFresnelColor: {
      texName = QString("%1_fresnel").arg(channelName);
      break;
    }
    case TexMarble: {
      texName = QString("%1_marble").arg(channelName);
      break;
    }
    case TexWood: {
      texName = QString("%1_wood").arg(channelName);
      break;
    }
    case TexMix: {
      texName = QString("%1_mix").arg(channelName);
      break;
    }
    case TexMultiMix: {
      texName = QString("%1_multimix").arg(channelName);
      break;
    }
    case TexDistortedNoise: {
      texName = QString("%1_imagemap").arg(channelName);
      break;
    }
    case TexInvertMap: {
      texName = QString("%1_invertmap").arg(channelName);
      break;
    }
    case TexGrayscale: {
      texName = QString("%1_greyscale").arg(channelName);
      break;
    }
    default:
      break;
  }
  channels[channelName] = ReTexturePtr(
    TextureCreator::createTexture( texName, textureType, this, dataType )
  );

  addTextureToCatalog(channels[channelName]);
  return true;
}

void ReMaterial::replaceTexture( const QString channelID, 
                                 const QString jsonTextureData, 
                                 ReTexturePtr _masterTexture ) {

  ReComplexTexturePtr masterTexture;
  // We will need to test for the master texture a few time, better
  // use a boolean than calling the isNull() function a lot.
  bool hasMasterTexture = !_masterTexture.isNull();

  // Check if the channel exist in the material. If _masterTexture
  // is not null then the channel ID refers to a subtexture, instead of
  // a material's channel
  if (hasMasterTexture) {
    masterTexture = _masterTexture.dynamicCast<ReComplexTexture>();
    // If the texture is a complex one and it doesn't contain the
    // channel then we can't continue.
    if (masterTexture && !masterTexture->hasChannel(channelID)) {
      return;
    }
  }
  else {
    // If the channel doesn't exist in the material then we can't continue
    if (!channels.contains(channelID)) {
      return;
    }    
  }

  // Convert the texture data to a QVariantMap
  QJson::Parser jsonParser;
  bool parseOk;
  // The JSON data passed is expected to be structured as follows:
  // {
  //   "deps": [
  //     { <first sub-texture> },
  //     { <n-th subtexture> },
  //   ],
  //   "texture": {
  //     < texture definition>
  //   }
  // }
  QVariantMap textureBundle = jsonParser.parse(jsonTextureData.toUtf8(), &parseOk).toMap();
  if (!parseOk) {
    RE_LOG_WARN() << "Error: could not parse the texture data: " << jsonTextureData;
    return;
  }
  if (!(textureBundle.contains("deps") && textureBundle.contains("texture"))) {
    RE_LOG_WARN() << "JSON data passed to ReMaterial::replaceTexture() does not define a texture bundle";
    return;
  }

  auto restoreTexture = [](QVariantMap& textureData, ReMaterial* mat) -> ReTexturePtr 
  {
    // Find if the a texture with the same name already exists in the material
    QString originalName = textureData.value("name").toString();
    QRegExp nameParser("^(\\.+)\\[(\\d+)\\]");
    nameParser.setMinimal(true);
    int version = 1;
    if (nameParser.indexIn(originalName) != -1) {
      originalName = nameParser.cap(1);
      version = nameParser.cap(2).toInt();
    }
    while (!mat->getTexture(textureData.value("name").toString()).isNull()) {
      textureData["name"] = QString("%1[%2]").arg(originalName).arg(++version);
    }
    auto texType = static_cast<ReTextureType>(textureData.value("type").toInt());
    auto texImporter = ReQtTextureImporterFactory::getImporter(texType);
    ReTexturePtr newTexture = ReTexturePtr(
      TextureCreator::createTexture(
        textureData.value("name").toString(), 
        texType,
        mat,
        static_cast<ReTexture::ReTextureDataType>(
          textureData.value("dataType").toInt()
        )
      ) 
    );
    texImporter->restoreTexture(newTexture, textureData);
    mat->addTextureToCatalog(newTexture);
    newTexture->reparent(mat);
    return newTexture;
  };

  auto subTextures = textureBundle.value("deps").toList();
  auto textureData = textureBundle.value("texture").toMap();
  auto texChannels = textureData.value("channels").toMap();

  // When a subtexture needs to be renamed we need to update the same name 
  // in the parent's texture channels directory. This function performs
  // that task
  auto renameChannel = [&texChannels](QString original, QString newName) {
    if (original == newName) {
      return;
    }
    QMapIterator<QString, QVariant> i(texChannels);
    while(i.hasNext()) {
      i.next();
      auto key = i.key();
      if (texChannels[key].toString() == original) {
        texChannels[key] = QVariant(newName);
      }
    }
  };

  // First create the sub-textures for the main texture, if any
  for (int i = 0; i < subTextures.count(); i++) {
    auto oneTextureData = subTextures[i].toMap();
    auto subTextureOriginalName = oneTextureData.value("name").toString();
    ReTexturePtr newT = restoreTexture(oneTextureData, this);
    subTextures[i] = oneTextureData;
    // Rename the texture in the master's texture channels directory, if 
    // necessary.
    renameChannel(subTextureOriginalName, newT->getName());
  }
  textureData["channels"] = texChannels;
  // Now we create the texture proper
  ReTexturePtr tex;
  if (hasMasterTexture) {
    tex = masterTexture->getChannel(channelID);
  }
  else {
    tex = channels.value(channelID);
  }
  if (!tex.isNull()) {
    tex->getParent()->deleteTexture(tex->getName());
  }
  tex = restoreTexture(textureData, this);
  if (hasMasterTexture) {
    masterTexture->setChannel(channelID, tex);
  }
  else {
    setChannel(channelID, tex->getName());    
  }
  setEdited(true);
}

const QString ReMaterial::identifyChannel( const ReTexturePtr tex ) const {
  ReNodeDictionaryIterator i(channels);
  while( i.hasNext() ) {
    i.next();
    QString chName = i.key();
    // Skip empty channels
    if (i.value().isNull()) {
      continue;
    }
    ReTexturePtr channelTexture = i.value();
    if (tex->getName() == channelTexture->getName()) {
      return chName;
    }
    else {
      // If we get here it means that the texture is not connected to one of the
      // channels directly. So, we need to test each channel and see which texture
      // contains this sub-texture and what is the path to it
      auto compTex = channelTexture.dynamicCast<ReComplexTexture>();
      if (!compTex.isNull()) {
        QString texChannel = compTex->identifyChannel(tex);
        if (!texChannel.isEmpty()) {
          return QString("%1|%2").arg(chName).arg(texChannel);
        }
      }
    }
  }
  return "";
}

ReTexturePtr ReMaterial::findTextureByChannelPath( const QString channelPathStr ) {
  QStringList channelPath = channelPathStr.split('|');
  ReTexturePtr theTex = channels[channelPath[0]];
  for (int i = 1; i < channelPath.count(); i++) {
    auto compTex = theTex.dynamicCast<ReComplexTexture>();
    if (!compTex.isNull()) {
      theTex = compTex->getChannel(channelPath[i]);
    }
  }

  return theTex;
}

bool ReMaterial::textureIsUnused( const QString& texName, 
                                  const QString ownerTexture ) const 
{
  // First test if the texture is linked to one of the channels
  ReNodeDictionaryIterator i(channels);
  while( i.hasNext() ) {
    i.next();
    ReTexturePtr tex = i.value();
    if (tex.isNull()) {
      continue;
    }
    if ( tex->getName() == texName ) {
      return false;
    }    
  }

  // Then search if it's used by any of the other textures in the catalog
  ReNodeDictionaryIterator in(nodeCatalog);
  while( in.hasNext() ) {
    in.next();
    QStringList deps;
    ReTexturePtr tex = in.value();
    if (tex.isNull()) {
      continue;
    }
    // Skip the texture that we are testing
    QString tName = tex->getName();
    if (tName == texName || tName == ownerTexture) {
      continue;
    }
    deps = tex->getDependencies();
    foreach( QString depName, deps ) {
      if (depName == texName) {
        return false;
      }
    }
  }
  return true;
}

void ReMaterial::setChannel( const QString& channelName, const QString& textureName ) {
  // The method can be used to clear a channel by passing an empty string 
  // for the texture name
  if (textureName.isEmpty()) {
    if (channels.contains(channelName) && !channels[channelName].isNull()) {
      QString texName = channels[channelName]->getName();
      channels[channelName] = ReTexturePtr();
      // Now check if the texture is unused, if so then we can remove it from 
      // the catalog and delete it.
      if ( textureIsUnused(texName) ) {
        deleteTexture(texName);
      }
      else {
        RE_LOG_INFO() << "Texture " << texName << " is used, cannot delete it";
      }
    }
    // we're done here
    return;
  }

  if (!(nodeCatalog.contains(textureName) && channels.contains(channelName))) {
      QString reason;
      if (!nodeCatalog.contains(textureName)) {
        reason = "texture not present in the catalog";
      }
      else {
        reason = "channel is not present.";
      }
      RE_LOG_WARN() 
           << QString(
                  "Warning: cannot set the channel %1 with texture name %2 for material %3. Reason: %4"
                )
                .arg(channelName)
                .arg(textureName)
                .arg(name)
                .arg(reason);
    return;
  }
  // Garbage collection. Dispose of the previous texture if it is attached to 
  // this channel.  
  ReTexturePtr testTex = channels.value(channelName);
  if (channels.contains(channelName) && !testTex.isNull()) {
    // Make sure that we are not trying to remove the same 
    // texture that we are linking. Extra caution.
    if (testTex->getName() != textureName) {
      removeTextureFromCatalog(channels.value(channelName));
      channels[channelName].clear();
    }
  }
  channels[channelName] = nodeCatalog[textureName];
}

ReTexturePtr ReMaterial::changeTextureType( const QString& name, 
                                            const ReTextureType newType ) {
  ReTexturePtr oldTex;
  if (nodeCatalog.contains(name)) {
    oldTex = nodeCatalog.value(name);
  }
  else {
    RE_LOG_INFO() << "Texture " << QSS(name) << " cannot be converted to " 
                  << QSS(ReTexture::getTypeAsString(newType)) << "\n"
                  << "The texture was not found in the material's catalog.";
    return ReTexturePtr();
  }

  ReTexture* newTex = NULL;
  switch( newType ) {
    case TexImageMap: {
      newTex = new ImageMap( oldTex );
      break;
    }
    case TexMath: {
      newTex = new ReMath( oldTex );
      break;
    }
    case TexColorMath: {
      newTex = new ReColorMath( oldTex );
      break;
    }
    case TexBand: {
      newTex = new ReBand( oldTex );
      break;
    }
    case TexBricks: {
      newTex = new Bricks( oldTex );
      break;
    }
    case TexCheckers: {
      newTex = new ReCheckers( oldTex );
      break;
    }
    case TexClouds: {
      newTex = new Clouds( oldTex );
      break;
    }
    case TexConstant: {
      newTex = new ReConstant( oldTex );
      break;
    }
    case TexFBM: {
      newTex = new ReFBM( oldTex );
      break;
    }
    case TexFresnelColor: {
      newTex = new ReFresnelColor( oldTex );
      break;
    }
    case TexMarble: {
      newTex = new ReMarble( oldTex );
      break;
    }
    case TexMix: {
      newTex = new ReMixTexture( oldTex );
      break;
    }
    case TexMultiMix: {
      newTex = new MultiMix( oldTex );
      break;
    }
    case TexDistortedNoise: {
      newTex = new ReDistortedNoise( oldTex );
      break;
    }
    // case TexInvertMap: {
    //   newTex = new InvertMap( oldTex );
    //   break;
    // }
    case TexGrayscale: {
      newTex = new ReGrayscale( oldTex );
      break;
    }
    case TexWood: {
      newTex = new ReWood( oldTex );
      break;
    }
    default:
      // shut up the compiler
      break;
  }
  ReTexturePtr newTexPtr = ReTexturePtr(newTex);
  // Remove the old texture, if not referenced anymore
  removeTextureFromCatalog(oldTex);
  addTextureToCatalog(newTexPtr);

  QString key;
  ReNodeDictionaryIterator i(channels);
  // Replace the texture in the material's channels
  // Sequential scans. We only have 3-4 textures per material, it's hardly something
  // worth loosing sleep about.
  while(i.hasNext()) {
    i.next();
    ReTexturePtr tex = i.value();
    if (tex.isNull()) {
      continue;
    }
    if (tex->getName() == name ) {
      // oldTex = tex;
      key = i.key();

      channels[key].clear();
      channels[key] = newTexPtr;
      break;
    }
  }

  // Now update all the textures that referenced the old texture
  ReNodeDictionaryIterator oneNode(nodeCatalog);
  while( oneNode.hasNext() ) {
    oneNode.next();
    ReTexturePtr nodeTex = oneNode.value();
    if (!nodeTex.isNull()) {
      oneNode.value()->replaceInnerTexture( name, newTexPtr );    
    }
  }

  return newTexPtr;
};


void ReMaterial::serialize( QDataStream& dataStream ) const { 
  dataStream << type            << name 
             << edited          << innerVolume  << outerVolume  
             << visibleInRender << acselID     << acselSetID
             << acselSetName    << (quint16) originalType;
  /*
   * Serialize the texture catalog
   */
  // We need to serialize the textures in the right order
  // with the ones included in compound textures before the
  // texture that contains them or the deserialization will
  // not work. 
  QStringList texNames;
  findDependencies(texNames);

  // Number of textures in the catalog
  int numTextures = texNames.count();
  dataStream << (quint16) nodeCatalog.count();
  for (int i = 0; i < numTextures; i++) {
    nodeCatalog.value(texNames[i])->serialize(dataStream);
  }

  /*
   * Serialize the channels
   * The channels simply refer to existing textures in the catalog
   * so, we simply serialize the names of those textures. The name will
   * be then references by the deserialize method to link to the textures
   */
  // Number of channels
  dataStream << (quint16) channels.count();
  ReNodeDictionaryIterator ic(channels);  
  while(ic.hasNext()) {
    ic.next();
    
    ReTexturePtr tex = ic.value();
    // Name of the channel and a flag to declare if the texture exists
    bool texPresent = !tex.isNull();
    dataStream << ic.key() << texPresent;
    // Texture
    if (texPresent) {
      dataStream << tex->getName();
    }
  }
};


void ReMaterial::deserialize( QDataStream& dataStream ) {
  quint16 numTextures,numChannels, numOriginalType;
  // Only to read the value from the stream, we don't 
  // actually use it because, at this point, the material is 
  // already created and typified.
  int matType;

  dataStream >> matType            >> name            
             >> edited             >> innerVolume
             >> outerVolume        >> visibleInRender 
             >> acselID            >> acselSetID
             >> acselSetName       >> numOriginalType    
             >> numTextures;

  originalType = static_cast<ReMaterialType>(numOriginalType);

  for (int i = 0; i < numTextures; ++i) {
    ReTexturePtr tex = TextureCreator::deserialize(dataStream, this);
    addTextureToCatalog(tex);
  }  

  dataStream >> numChannels;

  for (int i = 0; i < numChannels; ++i) {
    QString channelName;
    bool texPresent;
    dataStream >> channelName >> texPresent;
    if (texPresent) {
      QString texName;
      dataStream >> texName;
      setChannel(channelName, texName);
    }
    else {
      channels[channelName] = ReTexturePtr(); 
    }
  }
}


ReTexturePtr& ReMaterial::getTexture( const QString& textureID ) {
  if (nodeCatalog.contains(textureID)) {
    return nodeCatalog[textureID];
  }
  return nullValue;
}

const QString ReMaterial::getTypeAsString() const {
  return(typeNames[type]);
}

const QString ReMaterial::getTypeAsString( const ReMaterialType matType ) {
  return(typeNames[matType]);
}

ReMaterialType ReMaterial::typeFromName( const QString& matTypeName ) {
  QString matTypeStr = matTypeName.toLower();
  if (matTypeStr == "cloth") {
    return MatCloth;
  }
  else if (matTypeStr == "glass") {
    return MatGlass;
  }
  else if (matTypeStr == "glossy") {
    return MatGlossy;
  }
  else if (matTypeStr == "hair") {
    return MatHair;
  }
  else if (matTypeStr == "matte") {
    return MatMatte;
  }
  else if (matTypeStr == "metal") {
    return MatMetal;
  }
  else if (matTypeStr == "mirror") {
    return MatMirror;
  }
  else if (matTypeStr == "mix") {
    return MatMix;
  }
  else if (matTypeStr == "null") {
    return MatNull;
  }
  else if (matTypeStr == "skin") {
    return MatSkin;
  }
  else if (matTypeStr == "velvet") {
    return MatVelvet;
  }
  else if (matTypeStr == "water") {
    return MatWater;
  }
  else if (matTypeStr == "light") {
    return MatLight;
  }
  return MatUndefined;
}

ReMaterialType ReMaterial::typeFromShortName( const QString& matTypeStr ) {
  if (matTypeStr == RE_MAT_TYPE_CODE_CLOTH) {
    return MatCloth;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_GLASS) {
    return MatGlass;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_GLOSSY) {
    return MatGlossy;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_HAIR) {
    return MatHair;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_MATTE) {
    return MatMatte;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_METAL) {
    return MatMetal;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_MIRROR) {
    return MatMirror;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_MIX) {
    return MatMix;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_NULL) {
    return MatNull;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_SKIN) {
    return MatSkin;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_VELVET) {
    return MatVelvet;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_WATER) {
    return MatWater;
  }
  else if (matTypeStr == RE_MAT_TYPE_CODE_LIGHT) {
    return MatLight;
  }
  return MatUndefined;
}
  

  
QString ReMaterial::toString() {
  return QString("%1: %2").arg(getTypeAsString()).arg(name);
}

void ReMaterial::addTextureToCatalog( ReTexturePtr tex ) {
  if (tex.isNull()) {
    RE_LOG_INFO() << "Warning: texture passed to addTextureToCatalog is null";
    return;
  }
  nodeCatalog.insert(tex->getName(), tex);
}

bool ReMaterial::removeTextureFromCatalog( ReTexturePtr tex ) {
  QString texName = tex->getName();
  if (nodeCatalog.contains(texName)) {
    nodeCatalog.remove(texName);
    return true;
  }
  return false;
}

void ReMaterial::removeAllTextures() {
  // First remove all the references to texture form the channels
  QStringList channelNames = channels.keys();
  foreach( QString chName, channelNames ) {
    channels[chName] = ReTexturePtr();
  }

  QStringList texNames = nodeCatalog.keys();

  foreach( QString texName, texNames ) {
    deleteTexture(texName);
  }
}


bool ReMaterial::deleteTexture( const QString& texName ) {
  if (nodeCatalog.contains(texName)) {
    // Before deleting the texture check if it has dependencies.
    // If it does then we need to delete those first or we might
    // end up with orphaned textures
    QStringList deps = nodeCatalog.value( texName )->getDependencies();
    int numDeps = deps.count();
    for (int i = 0; i < numDeps; i++) {
      if ( textureIsUnused( deps[i], texName ) ) {
        deleteTexture( deps[i] );
      }
    }
    nodeCatalog[texName].clear();
    nodeCatalog.remove( texName );
    return true;
  }
  return false;
}


void ReMaterial::setTextureGamma( ReTexturePtr tex, const float newGamma ) {
  if (!tex.isNull() && tex->getType() == TexImageMap)  {
    tex.staticCast<ImageMap>()->setGamma(newGamma);
  }
  else {
    QStringList deps = tex->getDependencies();
    foreach( QString texName, deps) {
      if (!nodeCatalog.contains(texName)) {
        RE_LOG_WARN() << "Warning: texture " << QSS(texName) 
                      << " reported as dependency of " << QSS(tex->getName()) 
                      << " but it's not in the node catalog.";
        continue;
      }
      setTextureGamma(nodeCatalog[texName], newGamma);
    }
  }
}

void ReMaterial::volumeRenamed( const QString oldName, const QString newName ) {
  if (innerVolume == oldName) {
    innerVolume = newName;
  }
  else if (outerVolume == oldName) {
    outerVolume = newName;
  }
};

void ReMaterial::unlinkVolume( const QString volumeName ){
  if (innerVolume == volumeName) {
    innerVolume = "";
  }
  else if (outerVolume == volumeName) {
    outerVolume = "";
  }
};

QString ReMaterial::computeAcselID( const bool forDefaultShader ) {
  QStringList acselTextures;
  auto acsel = ReAcsel::getInstance();
  if (!forDefaultShader) {
    QVariantMap origShaderData;

    if ( acsel->getOriginalShader(parent->getInternalName(), name, origShaderData) ) {
      getImageMapsOrColors(origShaderData, acselTextures);
    }    
  }
  return acsel->getAcselID(parent->getGeometryFileName(), name, acselTextures);
}

} // namespace
