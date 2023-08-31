/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReNodeConverter.h"

#include "ReLogger.h"
#include "ReTools.h"
#include "textures/ReBand.h"
#include "textures/ReClouds.h"
#include "textures/ReColorMath.h"
#include "textures/ReConstant.h"
#include "textures/ReDistortedNoise.h"
#include "textures/ReFBM.h"
#include "textures/ReFresnelColor.h"
#include "textures/ReGrayscale.h"
#include "textures/ReImageMap.h"
#include "textures/ReMath.h"
#include "textures/ReMix.h"
#include "textures/ReWood.h"


using namespace Reality;

ReNodeDictionary ReNodeConverter::nodeCatalog;
QVariantMap ReNodeConverter::nodes;

void ReNodeConverter::init( const QVariantMap& _nodes ) {
  // Erase the node network catalog. This needs to be done at the
  // beginning of each conversion. The network will then be stored in the <Material> 
  // objects.
  nodeCatalog.clear();
  nodes = _nodes;
}

void ReNodeConverter::addNode( ReTexturePtr node ) {
  if (!node.isNull()) {
    nodeCatalog[node->getName()] = node;
  }
}

ReNodeDictionary ReNodeConverter::getNodeCatalog() {
  return nodeCatalog;
}

ReTexture::ReTextureDataType nodeHintToTextureDataType( const ReNodeConverter::NodeHint hint ) {
  return (hint == ReNodeConverter::NumericNode ? ReTexture::numeric : ReTexture::color);
}

ReTexturePtr ReNodeConverter::convertNode( const QString nodeName, const NodeHint hint, const bool makeUnique ) {
  // If the node has been already converted simply return the
  // pointer to it.
  ReTexturePtr nodePtr = nodeCatalog.value(nodeName);
  bool nodeIsShared = false;
  QString nodeKey = nodeName;

  if (!nodePtr.isNull()) {
    nodeIsShared = true;
    if (nodePtr->getDataType() != nodeHintToTextureDataType(hint)) {
      nodeIsShared = false;      
      // Rename the texture to break the conflict. We can't have a texture
      // with different data type but equal name. It will cause errors in the
      // export, where, for example, a float texture will be used where one of
      // type color is expected.
      nodeKey = QString("%1::%2").arg(nodeName).arg(hint == ColorNode ? "color" : "numeric");
    }
  }
  if (nodeIsShared && !makeUnique) {
    return nodePtr;
  }

  // If the node name exists already, and the call to this method requires,
  // to return a stand-alone, unique, instance, then we make a unique name
  // derived from the original.
  if (nodeIsShared) {
    int i = 2;
    while( true ) {
      nodeKey = QString("%1_%2").arg(nodeName).arg(i++);
      if (!nodeCatalog.contains(nodeKey)) {
        break;
      }
    }
  }

  // Retrieve the conversion data using the original node name
  QVariantMap node = nodes[nodeName].toMap();
  quint16 nodeType = node["type"].toInt();

  switch (nodeType) {
    case Band: {
      nodePtr = convertBand(nodeKey, node);
      break;
    }
    case Clouds: {
      nodePtr = convertClouds(nodeKey, node, hint);
      break;
    }    
    case FBM: {
      nodePtr = convertFBM(nodeKey, node, hint);
      break;
    }    
    case ColorMath: {
      nodePtr = convertColorMath(nodeKey, node, hint, makeUnique);
      break;
    }
    case Component: {
      nodePtr = convertComponent(nodeKey, node, hint);
      break;
    }
    case Constant: {
      nodePtr = convertConstant(nodeKey, node);
      break;
    }
    case DistortedNoise: {
      nodePtr = convertDistortedNoise(nodeKey, node, hint);
      break;
    }
    case FresnelColor: {
      nodePtr = convertFresnelColor(nodeKey, node);
      break;
    }
    case ImageMap: {
      nodePtr = convertImageMap(nodeKey, node, hint);
      break;
    }
    case Math: {
      nodePtr = convertMath(nodeKey, node, hint, makeUnique);
      break;
    }    
    case Mix: {
      nodePtr = convertMix(nodeKey, node, hint, makeUnique);
      break;
    }
    case Wood: {
      nodePtr = convertWood(nodeKey, node, hint);
      break;
    }
    default:
      // If the node is not supported we simply return NULL
      RE_LOG_DEBUG() << "Error: node type unknown: " << nodeType << " -- " << QSS(nodeKey);
      return ReTexturePtr();    
  }

  nodeCatalog[nodeKey] = nodePtr;
  return nodePtr;
}

/**
 Method: convertBand

 Band
 */
ReTexturePtr ReNodeConverter::convertBand( const QString& nodeName, const QVariantMap& node ) {
  ReBandPtr tex = ReBandPtr( new ReBand( nodeName ) );

  QVariant val;
  val.setValue(ReTexturePtr( new ReConstant( QString("%1_band1").arg(nodeName),
                                             0,
                                             convertFloatColor(node["tex1"].toList())) ));
  tex->setNamedValue("tex1", val);
  // #2
  val.setValue(ReTexturePtr( new ReConstant( QString("%1_band2").arg(nodeName),
                                             0,
                                             convertFloatColor(node["tex2"].toList())) ));
  tex->setNamedValue("tex2", val);
  // #3
  val.setValue(ReTexturePtr( new ReConstant( QString("%1_band3").arg(nodeName),
                                             0,
                                             convertFloatColor(node["tex3"].toList())) ));
  tex->setNamedValue("tex3", val);
  // #4
  val.setValue(ReTexturePtr( new ReConstant( QString("%1_band4").arg(nodeName),
                                             0,
                                             convertFloatColor(node["tex4"].toList())) ));
  tex->setNamedValue("tex4", val);

  QVariantList offsets = node["offsets"].toList();
  tex->setNamedValue("offset1", offsets[0]);
  tex->setNamedValue("offset2", offsets[1]);
  tex->setNamedValue("offset3", offsets[2]);
  tex->setNamedValue("offset4", offsets[3]);

  QString texName = node["amount map"].toString();

  if (texName != "") {
    val.setValue(convertNode(texName, NumericNode, true));
    tex->setNamedValue("map", val);
  }
  else {
    tex->setAmount(node["amount"].toFloat());
  }
  return tex;
}

/**
 * Convert a node of type ImageMap
 */
ReTexturePtr ReNodeConverter::convertImageMap( const QString& nodeName, 
                                               const QVariantMap& node,
                                               const NodeHint hint ) 
{
  auto fn = node["fileName"].toString();
  #if defined(_WIN32)
  fn.replace('\\','/');
  #endif
  ImageMapPtr tex = ImageMapPtr(
    new Reality::ImageMap( nodeName,
                           0, 
                            fn,
                           ( hint == NumericNode ? ReTexture::numeric : ReTexture::color ) )
  );
  tex->setUTile(node["u tile"].toFloat());
  tex->setVTile(node["v tile"].toFloat());
  tex->setUOffset(node["u offset"].toFloat());
  tex->setVOffset(node["v offset"].toFloat());
  tex->setGain(node["gain"].toFloat());
  if (node.contains("isNormalMap")) {
    tex->setNormalMap(node["isNormalMap"].toBool());
  }
  return tex;
}

/*
 Method: convertColorMath

 Color Math
 */
ReTexturePtr ReNodeConverter::convertColorMath( const QString& nodeName, 
                                                const QVariantMap& node,
                                                const NodeHint hint,
                                                const bool makeUnique ) 
{
  // Poser treats numeric and color nodes in similar ways.
  // You can find color math plugged into the bump map channel, for example.
  // In those cases we defer to the Math converter if the output is 
  // expected to be a numeric node.
  if ( hint == NumericNode ) {
    return convertMath(nodeName, node, hint, makeUnique);
  }

  // From now on the subtextures must be of type color
  NodeHint newHint = ColorNode;

  Reality::ReColorMath* cmNode = new Reality::ReColorMath(nodeName);

  // What function is the node performing?
  QString func = node["function"].toString();
  if (func == "m") {
    cmNode->setFunction(ReColorMath::multiply);
  }
  else if (func == "a") {
    cmNode->setFunction(ReColorMath::add);
  }
  else {
    cmNode->setFunction(ReColorMath::subtract);
  }

  // Fill the data for Texture 1
  QVariantMap val1 = node["value 1"].toMap();
  // First the color
  QColor color1 = convertFloatColor(val1["color"].toList());

  // Do we have a texture too ?
  QString texName = val1["map"].toString();
  ReTexturePtr texMap;
  bool texture1Present = texName != "";
  if (texture1Present) {
    texMap = convertNode(texName, newHint, makeUnique);
    cmNode->setTexture1(texMap);
    cmNode->setColor1(color1);
  }
  else {
    cmNode->setTexture1( ReConstantPtr( 
      new Reality::ReConstant(QString("%1_tex1").arg(nodeName),0,color1) )
    );
    cmNode->setColor1(RE_PURE_WHITE_COLOR);
    texture1Present = true;
  }

  // Now texture 2...
  QVariantMap val2 = node["value 2"].toMap();
  // First the color
  QColor color2 = convertFloatColor(val2["color"].toList());

  texName = val2["map"].toString();

  // Do we have a texture too ?
  bool texture2Present = texName != "";
  if (texture2Present) {
    texMap = convertNode(texName, newHint, makeUnique);
    cmNode->setTexture2(texMap);
    cmNode->setColor2(color2);
  }
  else {
    cmNode->setTexture2(ReConstantPtr( 
      new Reality::ReConstant(QString("%1_tex2").arg(nodeName),0,color2) )
    );
    cmNode->setColor2(RE_PURE_WHITE_COLOR);
    texture2Present = true;
  }

  // If both the first and second textures are not present then and
  // both colors are the same we can simplify this nodes as a simple
  // constant color node. This situation can happen because of the 
  // conversion of Poser nodes. Two nodes might be added, for example,
  // using a color and a node for each channel. If the node is not 
  // supported or superfluous for Reality then the result will be to
  // have a color and no texture for the channel. Often the color is
  // pure white and/or the same color is used for each channel. 
  if (!texture1Present && !texture2Present) {
    if (color1 == color2) {
      return cmNode->getTexture1();
    }
  }
  // If one texture is present but the other is not then set the operation to be null
  if (!(texture1Present && texture2Present)) {
    cmNode->setFunction(ReColorMath::none);
  }
  return ReTexturePtr(cmNode);
}

/**
 * Returns a boolean whether the input map is a compatible input
 * for the ReGrascale texture.
 */
bool gsIncompatible( const ReTexturePtr tex ) {
  switch( tex->getType() ) {
    case TexMath:
    case TexColorMath:
    case TexFresnelColor:
    case TexInvertMap:
    case TexGrayscale:
      return true;
    default:
      return false;    
  }
}

/**
 Method: convertMath

 Math
 */
ReTexturePtr ReNodeConverter::convertMath( const QString& nodeName, 
                                           const QVariantMap& node, 
                                           const NodeHint hint,
                                           const bool makeUnique ) 
{
  // If we need to return a color node then convert the texture to 
  // a B&W texture of type color
  if (hint == ColorNode) {
    QString texName = node["value 1"].toMap().value("map").toString();
    ReTexturePtr texMap;
    if (texName == "") {
      texName = node["value 2"].toMap().value("map").toString();
    }
    if (texName != "") {
      texMap = convertNode(texName, NumericNode, makeUnique);
      // If the map is incompatible with the Grayscale node then
      // we just return a generic medium gray constant
      if (gsIncompatible(texMap)) {
        return ReTexturePtr( new ReConstant(nodeName, 0, RE_MEDIUM_GRAY_COLOR) );
      }

      ReGrayscalePtr gs = ReGrayscalePtr( new ReGrayscale(nodeName) );
      gs->setTextureDataType(ReTexture::color);
      gs->setTexture(texMap);
      return gs;
    }
    else {
      // Last ditch effort to return anything meaningful...
      return ReTexturePtr( new ReConstant(nodeName, 0, RE_WHITE_COLOR) );
    }
  }

  Reality::ReMath* mathNode = new Reality::ReMath(nodeName);  

  mathNode->setTextureDataType(ReTexture::numeric);

  // What function is the node performing?
  QString func = node["function"].toString();
  if (func == "m") {
    mathNode->setFunction(ReMath::multiply);
  }
  else if (func == "a") {
    mathNode->setFunction(ReMath::add);
  }
  else {
    mathNode->setFunction(ReMath::subtract);
  }

  // Fill the data for Texture 1
  QVariantMap val1 = node["value 1"].toMap();
  mathNode->setAmount1(val1["value"].toFloat());
  QString texName = val1["map"].toString();
  ReTexturePtr texMap;
  if (texName != "") {
    texMap = convertNode(texName, NumericNode, makeUnique);
    mathNode->setTexture1(texMap);
  }
  // Now texture 2...
  QVariantMap val2 = node["value 2"].toMap();
  texName = val2["map"].toString();
  mathNode->setAmount2(val2["value"].toFloat());

  // Do we have a texture too ?
  if (texName != "") {
    texMap = convertNode(texName, NumericNode, makeUnique);
    mathNode->setTexture2(texMap);
  }
  return ReTexturePtr(mathNode);
}

/**
 Method: convertMix

 Mix
 */
ReTexturePtr ReNodeConverter::convertMix( const QString& nodeName, 
                                          const QVariantMap& node,
                                          const NodeHint hint,
                                          const bool makeUnique ) 
{
  /*
   Node format:
   {
    'mix': float,
    'mix map': '',
    'tex1 color': list of floats,   // example: (1.0, 1.0, 1.0),
    'tex1 map': string,             // example: "Edge_Blend::fresnel1"
    'tex2 color': list of floats,
    'tex2 map': string',
   */
  Reality::ReMixTexture* mixNode = new Reality::ReMixTexture(nodeName);
  // We loop through our two possible channels, texture 1 and texture 2
  for (int i = 1; i <= 2 ; i++) {
    // First the color
    QColor color = convertFloatColor(node[QString("tex%1 color").arg(i)].toList());
    // Do we have a texture too ?
    QString texName = node[QString("tex%1 map").arg(i)].toString();
    ReTexturePtr texMap;
    if (texName != "") {
      texMap = convertNode(texName, hint, makeUnique);
    }
    // if the color is pure white then we don't need to mix it with the 
    // texture, we fetch the texture directly
    if (isPureWhite(color)) {
      if (texName != "") {
        i == 1 ? mixNode->setTexture1(texMap) : mixNode->setTexture2(texMap);
      }
      // Otherwise let's just create a Constant node
      else {
        QString paramName = QString("texture%1").arg(i);
        ReConstantPtr ctex = Reality::ReConstantPtr( 
            new Reality::ReConstant(
              QString("%1_color%2").arg(nodeName).arg(i),
              0,
              color 
            ) 
        );
        if (i == 1) {
          mixNode->setTexture1(ctex);          
        }
        else {
          mixNode->setTexture2(ctex);          
        }
      }
    }
    else {
      // If we have a color that is not pure white and a texture then we
      // "join" them together via a color multiplication node and then we
      // link this node to the multiply node.
      if (texName != "") {
        QString multNodeName = QString("%1_mix%2").arg(nodeName).arg(i);        
        if (hint == NumericNode) {
          Reality::ReMath* multNode = new Reality::ReMath(multNodeName);
          multNode->setAmount1(1.0);
          multNode->setTexture1(texMap);
          ReTexturePtr multNodePtr(multNode);
          nodeCatalog[multNodeName] = multNodePtr;
          i == 1 ? mixNode->setTexture1(multNodePtr) : mixNode->setTexture2(multNodePtr);
        }
        else {
          Reality::ReColorMath* multNode = new Reality::ReColorMath(multNodeName);
          multNode->setColor1(color);
          multNode->setTexture1(texMap);
          ReTexturePtr multNodePtr(multNode);
          nodeCatalog[multNodeName] = multNodePtr;
          i == 1 ? mixNode->setTexture1(multNodePtr) : mixNode->setTexture2(multNodePtr);
        }
      }
      else {
        // We set the color via a constant texture        
        QString clrNodeName = QString("%1_clr%2").arg(nodeName).arg(i);
        // If this is a grayscale texture then we force the color to be grayscale
        if (hint == ColorNode) {
          int g = qGray(color.rgb());
          color.setRgb(g, g, g);
        }
        ReTexturePtr colorNode = ReTexturePtr(new Reality::ReConstant(clrNodeName, 0, color));
        colorNode->setTextureDataType(hint == NumericNode ? ReTexture::numeric : ReTexture::color);
        nodeCatalog[clrNodeName] = colorNode;
        i == 1 ? mixNode->setTexture1(colorNode) : mixNode->setTexture2(colorNode);
      }
    }
  }
  float mixAmount = node["mix"].toDouble();
  mixNode->setMixAmount(mixAmount);
  ReTexturePtr mixTexture;
  QString mixTextureStr = node["mix map"].toString();
  if (mixTextureStr != "") {
    mixTexture = convertNode(mixTextureStr, NumericNode, makeUnique);
    mixNode->setMixTexture(mixTexture);
    mixNode->setMixAmount(1.0);
  }
  // Sanity check. If we end up with mixing two identical colors,
  // it can happen, just delete the node created this far and replace
  // it with one constant texture using the same color.
  ReTexturePtr tex1 = mixNode->getTexture1();
  ReTexturePtr tex2 = mixNode->getTexture2();
  // This should never happen but just in case...
  if (tex1.isNull()) {
    tex1 = Reality::ReConstantPtr( 
             new Reality::ReConstant(
               QString("%1_color1").arg(nodeName),
               0,
               RE_WHITE_COLOR
             ) 
           );
    mixNode->setTexture1(tex1);
  }
  if (tex2.isNull()) {
    tex2 = Reality::ReConstantPtr( 
             new Reality::ReConstant(
               QString("%1_color2").arg(nodeName),
               0,
               RE_WHITE_COLOR
             ) 
           );
    mixNode->setTexture2(tex2);
  }
  if (
       tex1->getType() == TexConstant && tex2->getType() == TexConstant &&
       *tex1.staticCast<ReConstant>() == *tex2.staticCast<ReConstant>()
     ) {
    ReTexturePtr newCol(new ReConstant(
                              QString("%1_color").arg(nodeName),
                              0,
                              tex1.staticCast<ReConstant>()->getColor()
                        )
    );
    delete mixNode;
    return newCol;
  }
  // Align the data type. The mix texture must have the same data type (color, float, fresnel)
  // of the textures mixed. Currently we don't have a good solution in case we end up
  // with two textures of different data type. This situation should be caught before 
  // arriving here. 
  // In any case, we use the first texture as the main driver of the decision here. If the
  // second texture has different data type then it is forced to assume the data type
  // of the first texture. This will probably generate an "texture not found" error in the
  // Lux log. 
  // Lastly, the mix texture is set to have the same data type of the first texture.
  if ( hint != NoHint ) {
    if (hint == NumericNode) {
      mixNode->setTextureDataType(ReTexture::numeric);
      tex1->setTextureDataType(ReTexture::numeric);
      tex2->setTextureDataType(ReTexture::numeric);
    }
    else {
      mixNode->setTextureDataType(ReTexture::color);
      tex1->setTextureDataType(ReTexture::color);
      tex2->setTextureDataType(ReTexture::color);
    }
  }
  else {
    ReTexture::ReTextureDataType dt1, dt2;
    dt1 = tex1->getDataType();
    dt2 = tex2->getDataType();
    if (dt1 != dt2) {
      tex2->setTextureDataType(dt1);
    }
    mixNode->setTextureDataType(dt1);
  }
  return ReTexturePtr(mixNode);
}

/*
 Method: convertConstant

 Constant
 */

ReTexturePtr ReNodeConverter::convertConstant( const QString& nodeName, const QVariantMap& node ) {
  QColor clr = convertFloatColor(node["value"].toList());
  double gain = node["gain"].toDouble();
  // Poser uses a gain factor for the Diffuse node, that number
  // can be used to attenuate the brightness of the color. 
  if (gain < 1.0) {
    clr.setRgbF(clr.redF()*gain, clr.greenF()*gain, clr.blueF()*gain);
  }
  return ReTexturePtr(new Reality::ReConstant(nodeName, 0, clr));
}

/*
 Method: convertFresnelColor

 Fresnel
 */

ReTexturePtr ReNodeConverter::convertFresnelColor( const QString& nodeName, const QVariantMap& node ) {
  QColor clr = convertFloatColor(node["color"].toList());
  QString mapName = node["map"].toString();
  if (mapName != "") {
    ReTexturePtr tex = convertNode(mapName);
    return ReFresnelColorPtr(new ReFresnelColor(nodeName, 0, clr, tex));
  }
  return ReFresnelColorPtr(new ReFresnelColor(nodeName, 0, clr));
}

// If we need to use a numeric texture as a color one we wrap it 
// using a mixer texture 
ReTexturePtr makeNumericTextureColor( ReTexturePtr origTex ) {
  ReMixTexturePtr mixer = ReMixTexturePtr( new ReMixTexture(QString("%1_mixer").arg(origTex->getName())) );

  mixer->setMixTexture(origTex);
  return mixer;
}

/*
 Method: convertDistortedNoise

 Distorted Noise
 */

ReTexturePtr ReNodeConverter::convertDistortedNoise( const QString& nodeName, 
                                                     const QVariantMap& node,
                                                     const NodeHint hint ) 
{
  ReDistortedNoisePtr tex = Reality::ReDistortedNoisePtr(new Reality::ReDistortedNoise(nodeName));
  tex->setBrightness(node["brightness"].toDouble());
  tex->setContrast(node["contrast"].toDouble());
  tex->setDistortionAmount(node["amount"].toDouble());
  tex->setNoiseSize(node["size"].toDouble());
  QString val = node["noise basis"].toString();
  if (val == "improved perlin") {
    tex->setNoiseBasis(ReDistortedNoise::IMPROVED_PERLIN);
  }
  else {
    tex->setNoiseBasis(ReDistortedNoise::ORIGINAL_PERLIN);        
  }
  val = node["noise distortion"].toString();
  if (val == "improved perlin") {
    tex->setDistortionType(ReDistortedNoise::IMPROVED_PERLIN);
  }
  else {
    tex->setDistortionType(ReDistortedNoise::ORIGINAL_PERLIN);        
  }

  if (hint == ColorNode) {
    return makeNumericTextureColor(tex);
  }

  return tex;
}


/**
 * Clouds
 */
ReTexturePtr ReNodeConverter::convertClouds( const QString& nodeName, 
                                             const QVariantMap& node,
                                             const NodeHint hint ) {

  ReCloudsPtr tex = Reality::ReCloudsPtr(new Reality::ReClouds(nodeName));
  tex->setBrightness(node["brightness"].toDouble());
  tex->setContrast(node["contrast"].toDouble());
  tex->setNoiseSize(node["size"].toDouble());
  tex->setNoiseDepth(node["depth"].toInt());
  QString val = node["noise basis"].toString();
  if (val == "improved perlin") {
    tex->setNoiseBasis(ReClouds::IMPROVED_PERLIN);
  }
  else {
    tex->setNoiseBasis(ReClouds::ORIGINAL_PERLIN);
  }
  tex->setHardNoise(node["style"].toString() == "hard");
  val = node["noise distortion"].toString();

  if (hint == ColorNode) {
    return makeNumericTextureColor(tex);
  }
  return tex;
}

/**
 * FBM
 */
ReTexturePtr ReNodeConverter::convertFBM( const QString& nodeName, 
                                          const QVariantMap& node,
                                          const NodeHint hint ) 
{
  auto tex = ReFBMPtr(new ReFBM(nodeName));
  tex->setOctaves(node["octaves"].toInt());
  tex->setRoughness(node["roughness"].toFloat());
  tex->setScale(node["scale"].toFloat());

  if (hint == ColorNode) {
    return makeNumericTextureColor(tex);
  }
  return tex;
}


/**
 Method: convertWood

 Wood
 */
ReTexturePtr ReNodeConverter::convertWood( const QString& nodeName, 
                                           const QVariantMap& node,
                                           const NodeHint hint ) 
{

  ReWoodPtr tex = ReWoodPtr(new ReWood(nodeName));
  QString val = node["noise basis"].toString();
  if (val == "improved perlin") {
    tex->setNoiseBasis(ReClouds::IMPROVED_PERLIN);
  }
  else {
    tex->setNoiseBasis(ReClouds::ORIGINAL_PERLIN);
  }
  tex->setNoiseSize(node["size"].toDouble());
  tex->setTurbulence(node["turbulence"].toDouble());
  tex->setBrightness(node["brightness"].toDouble());
  tex->setContrast(node["contrast"].toDouble());
  tex->setSoftNoise(node["soft noise"].toBool());
  val = node["pattern"].toString();
  if (val == "BAND_NOISE") {
    tex->setWoodPattern(ReWood::BAND_NOISE);
  }
  else if (val == "RING_NOISE") {
    tex->setWoodPattern(ReWood::RING_NOISE);
  }
  else if (val == "RING_NOISE") {
    tex->setWoodPattern(ReWood::RING_NOISE);
  }
  else if (val == "RINGS") {
    tex->setWoodPattern(ReWood::RINGS);
  }
  else if (val == "BANDS") {
    tex->setWoodPattern(ReWood::BANDS);
  }
  val = node["vein wave"].toString();
  if (val == "SIN") {
    tex->setVeinWave(ReWood::SIN);
  }
  else if (val == "SAWTOOTH") {
    tex->setVeinWave(ReWood::SAWTOOTH);
  }
  else if (val == "TRIANGLE") {
    tex->setVeinWave(ReWood::TRIANGLE);
  }
  return tex;
}



/**
 Method: convertComponent

 Component, an RGB channel
 */
ReTexturePtr ReNodeConverter::convertComponent( const QString& nodeName, 
                                                const QVariantMap& node,
                                                const NodeHint hint ) {
  QColor compColor = convertFloatColor(node["color"].toList()); 
  QString mapName = node["map"].toString();
  ReGrayscale* gsTex;
  if (mapName != "") {
    gsTex = new ReGrayscale( nodeName, 0, node["channel"].toInt(), convertNode(mapName) );
  }
  else {
    // Sometimes there are Poser comp nodes that are not connected to anything. In that
    // case we simply get a GS version of the main color
    compColor = qGray(compColor.rgb());
    return( ReTexturePtr( new ReConstant(nodeName, 0, compColor, ReTexture::numeric) ) );
  } 
  gsTex->setColor(compColor);
  if (hint != NoHint) {
    if (hint == ColorNode) {
      gsTex->setTextureDataType(ReTexture::color);
    }
    else {
      gsTex->setTextureDataType(ReTexture::numeric);
    }
  }
  return ReTexturePtr( gsTex );
}


