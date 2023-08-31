/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LUX_MATERIAL_EXPORTER_H
#define RE_LUX_MATERIAL_EXPORTER_H

#include <QStringBuilder>
#include <QStringList>

#include "reality_lib_export.h"
#include "ReDisplaceableMaterial.h"
#include "ReMaterial.h"
#include "ReLogger.h"
#include "ReTexture.h"
#include "ReTools.h"
#include "exporters/ReMaterialExporter.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"
#include "textures/ReMath.h"
#include "textures/ReMix.h"


namespace Reality {

/**
  Specializes the abstract <ReMaterialExporter> to handle Lux export functions.
  All specialized Lux material exporters are descendants of this class.
 */

class REALITY_LIB_EXPORT ReLuxMaterialExporter : public ReMaterialExporter {

public:

  // Constructor: ReLuxMateriaExporter
  ReLuxMaterialExporter() {
  };
  // Destructor: ReLuxMateriaExporter
  virtual ~ReLuxMaterialExporter() {
  };
  
  /**
   * Creates the parameter for the amount of alpha map. If there is an alpha map 
   * and the amount is different from 1.0 then it creates an intermediate texture
   * used to provide the degree of alpha map requested
   */
  const QString getAlphaAmount( const ReTexturePtr alphaMap,
                                const float alphaStrength,
                                QString& extraTextures ) {
    QString str;
    QString amountTexName;
    if (alphaMap.isNull()) {
      return QString("\"texture amount\" [\"%1\"]").arg(alphaStrength);
    }
    else {      
      if (alphaStrength < 1.0) {
        amountTexName = QString("%1_amount").arg(alphaMap->getUniqueName());
        extraTextures += QString(
                            "Texture \"%1\" \"float\" \"scale\" \"texture tex1\" [\"%2\"] "
                            "\"float tex2\" [%3]\n"
                         )
                         .arg(amountTexName)
                         .arg(alphaMap->getUniqueName())
                         .arg(alphaStrength);
      }
      else {
        amountTexName = alphaMap->getUniqueName();
      }
      return QString("\"texture amount\" [\"%1\"]").arg(amountTexName);
    }
  }

  /**
   *   Creates an alpha channel based on the alpha map, the alpha strength
   *   or a combination of both.
   * Parameters:
   *   - matName: the name to be assigned to the Mix material that implements the
   *              alpha channel. The name is also used for naming the two linked
   *              materials. The null is named "[matName].Null" and the base material
   *              is named "[matName].BaseMat".
   *   - alphaMap: Pointer to a texture used as a alpha map. It can be Null. In that case
   *               the alpha strength will be used instead.
   *   - alphaStrength: the opacity level.
   * 
   * Returns:
   *   A string with the definition of the material that defines the alpha channel.
   *   The material definition includes a Null defined for this material but it assumes
   *   that the base material has already been defined elsewhere.
   */
  const QString makeAlphaChannel( const QString matName, 
                                  const ReTexturePtr alphaMap,
                                  const float alphaStrength ) {
    // If we need to generate extra textures then they go here.
    QString extraTextures;

    QString str = QString("#\n# Mat: %1\n#\n").arg(matName);
    str += QString("MakeNamedMaterial \"%1_Null\" \"string type\" [\"null\"] \n").arg(matName);
    if (!alphaMap.isNull()) {
      str += QString(
            "MakeNamedMaterial \"%1\" \"string type\" [\"mix\"] %2"
          )
          .arg(matName)
          .arg(getAlphaAmount(alphaMap, alphaStrength, extraTextures));
    }
    else {
      str += QString(
               "MakeNamedMaterial \"%1\" \"string type\" [\"mix\"] "
               "\"float amount\" [%2] "
             )
             .arg(matName)
             .arg(alphaStrength);    
    }
    str += QString(
             "\"string namedmaterial1\" [\"%1_Null\"] "
             "\"string namedmaterial2\" [\"%1_BaseMat\"] \n"
           )
           .arg(matName);
    return extraTextures % str;
  }

  /**
   *   Creates a series of textures to define a bump map based on Positive, Negative 
   *   and Strength parameters. These are not defined in Poser but they are a good
   *   way to add control in the hands of the user without having to resort to complex
   *   modifications of the base texture.
   * Parameters:
   *   bumpMap - Input. The base bump map texture to export
   *   str - Output. The string buffer where the method writes the bump map textures
   *   bmNegative - Input. The minimum value of bump map allowed.
   *   bmPositive - Input. The maximum value of bump map allowed.
   *   bmStrength - Input. The maximum strength of the bump map.
   */
  const QString makeBumpMap( const ReMaterial* mat,
                             const ReTexturePtr bumpMap, 
                             QString& str, 
                             const float bmNegative, 
                             const float bmPositive, 
                             const float bmStrength )
  {
    ReTexturePtr actualTex;
    // If the bump map texture is not a float texture then write a float
    // version of it and obtain the name of this variant
    if ( bumpMap->getDataType() != ReTexture::numeric ) {
      actualTex = ReLuxTextureExporter::writeFloatVersion( bumpMap, str );
    }
    else {
      actualTex = bumpMap;
    }
    QString actualTexName = actualTex->getUniqueName();
    // If this is a normal map then we simply export the map and possibly
    // scale its intensity
    // if (actualTex->getType() == TexImageMap &&
    //     actualTex.staticCast<ImageMap>()->isNormalMap()) {
    if (containsNormalMap(actualTex)) {
      if (bmStrength != 1.0) {
        // The following texture applies the strength of the bump map using 
        // a multiplier.
        QString bmTextureName = QString("%1_nmAmplitude")
                                  .arg(mat->getUniqueName());
        auto nmScale = ReMathPtr( new ReMath(bmTextureName) );
        nmScale->setTexture1(actualTex);
        nmScale->setTexture2(ReTexturePtr());
        nmScale->setAmount2(bmStrength);
        nmScale->setFunction(ReMath::multiply);
        if (!ReLuxTextureExporter::isTextureInCache(bmTextureName)) {
          auto textureExporter = ReLuxTextureExporterFactory::getExporter(
                                   nmScale
                                 );
          LUX_DEBUG(str)
          str += textureExporter->exportTexture(nmScale);
          ReLuxTextureExporter::addTextureToCache(nmScale);
        }
        return bmTextureName;
      }
      // otherwise we simply use the normal map as it is
      // return actualTexName;
      return ReLuxTextureExporter::getTextureFromCache(bumpMap)->getUniqueName();
    }
    // The following texture implements a clamping system. The Negative and 
    // Positive values in the bump map texture designate the minimum and 
    // maximum limits of the bump map "displacement". Using a Mix texture 
    // we clamp the levels to what the user defined.
    // Please note that Lux uses bump maps that are the opposite of what 
    // Poser/Studio use.
    // For this reason the order of Positive/Negative is inverted in the 
    // following definition.
    //
    // The organization of the texture sequence is:
    //
    // Texture "OBJ:MAT:TEXNAME.bumpmap" "scale" [LIMITS_TEXTURE] [AMPLITUDE]
    // LIMITS_TEXTURE => Mix Texture [NEGATIVE] [POSITIVE] [BUMP IMAGE MAP]
    // [BUMP IMAGE MAP] => Image Map texture

    // Every material has its own bump map settings, which can be different
    // from any other material
    QString bmClampTexname = QString("BM_%1_limits")
                               .arg(mat->getUniqueName());
    ReMixTexturePtr clampTex = ReMixTexturePtr( 
                                 new ReMixTexture(bmClampTexname,
                                                  0, 
                                                  ReTexture::numeric) 
                               );
    clampTex->setMixTexture(actualTex);
    clampTex->setTexture1(bmNegative);
    clampTex->setTexture2(bmPositive);

    auto textureExporter = ReLuxTextureExporterFactory::getExporter(clampTex);
    str += textureExporter->exportTexture(clampTex);
    
    // The following texture applies the strength of the bump map using 
    // a multiplier.
    QString bmTextureName = QString("%1_bumpmap").arg(mat->getUniqueName());
    auto bumpScale = ReMathPtr( new ReMath(bmTextureName) );
    bumpScale->setTexture1(clampTex);
    bumpScale->setTexture2(ReTexturePtr());    
    bumpScale->setAmount2(bmStrength);
    bumpScale->setFunction(ReMath::multiply);
    if (!ReLuxTextureExporter::isTextureInCache(bumpScale)) {
      auto textureExporter = ReLuxTextureExporterFactory::getExporter(bumpScale);
      str += textureExporter->exportTexture(bumpScale, bmTextureName);
      ReLuxTextureExporter::addTextureToCache(bumpScale);
    }
    return bmTextureName;
  }

  /**
   *   Creates a series of textures to define a disp map based on Positive, Negative 
   *   and Strength parameters. These are not defined in Poser but they are a good
   *   way to add control in the hands of the user without having to resort to complex
   *   modifications of the base texture.
   * Parameters:
   *   dispMap - Input. The base disp map texture to export
   *   str - Output. The string buffer where the method writes the disp map textures
   *   bmNegative - Input. The minimum value of disp map allowed.
   *   bmPositive - Input. The maximum value of disp map allowed.
   *   bmStrength - Input. The maximum strength of the disp map.
   */
  const QString makeDispMap( const ReMaterial* mat,
                             const ReTexturePtr dispMap, 
                             QString& str, 
                             const float dmNegative, 
                             const float dmPositive, 
                             const float dmStrength )  {

    ReTexturePtr actualTex;
    // If the disp map texture is not a float texture then write a float
    // version of it and obtain the name of this variant
    if ( dispMap->getDataType() != ReTexture::numeric ) {
      actualTex = ReLuxTextureExporter::writeFloatVersion( dispMap, str );
    }
    else {
      actualTex = dispMap;
    }
    QString baseMapName = actualTex->getUniqueName();

    // The following texture implements a clamping system. The Negative and Positive values
    // in the disp map texture designate the minimum and maximum limits of the disp map
    // "displacement". Using a Mix texture we clamp the levels to what the user defined.
    // Please note that Lux uses disp maps that are the opposite of what Poser/Studio use.
    // For this reason the order of Positive/Negative is inverted in the following 
    // definition.
    QString dmClampTexname = QString("DM_%1_limits")
                               .arg(mat->getUniqueName());
    ReMixTexturePtr clampTex = ReMixTexturePtr( 
                                 new ReMixTexture(dmClampTexname,
                                                  0, 
                                                  ReTexture::numeric) 
                               );
    clampTex->setMixTexture(actualTex);
    clampTex->setTexture1(dmNegative);
    clampTex->setTexture2(dmPositive);
    auto textureExporter = ReLuxTextureExporterFactory::getExporter(clampTex);
    str += textureExporter->exportTexture(clampTex);

    // The following texture applies the strength of the disp map using a 
    // multiplier.
    QString dmTextureName = QString("%1_dispmap").arg(baseMapName);
    ReMathPtr dmTex = ReMathPtr( new ReMath(dmTextureName) );
    dmTex->setTexture1(clampTex);
    dmTex->setTexture2(ReTexturePtr());    
    dmTex->setAmount2(dmStrength);
    dmTex->setFunction(ReMath::multiply);
    if (!ReLuxTextureExporter::isTextureInCache(dmTex)) {
      auto textureExporter = ReLuxTextureExporterFactory::getExporter(dmTex);
      str += textureExporter->exportTexture(dmTex);
      ReLuxTextureExporter::addTextureToCache(dmTex);
    }
    return dmTextureName;
  }

  //! Export all the textures connected to this object. The idea is to 
  //! have a list of all the textures exported before the list of materials.
  void exportTextures( const ReMaterial* mat, 
                       boost::any& result, 
                       bool isForPreview = false) 
  {
    QStringList texList;

    mat->findDependencies(texList);

    QString str;
    int numTexs = texList.count();
    for (int i = 0; i < numTexs; ++i) {
      QString texName = texList[i];
      ReTexturePtr tex = const_cast<ReMaterial*>(mat)->getTexture(texName);
      // Just being paranoid, this should not happen...
      if (tex.isNull()) {
        RE_LOG_DEBUG() << "Error in exportTextures for " 
                       << QSS(mat->getName()) 
                       << ": could not find " 
                       << QSS(texName);
        ReNodeDictionaryIterator i(mat->getTextures());
        while(i.hasNext()) {
          i.next();
          RE_LOG_DEBUG() << "  - " << QSS(i.key());
        }
        continue;
      }
      // Check if the texture has been cached already.
      bool exported = ReLuxTextureExporter::isTextureInCache(tex);
      ReLuxTextureExporter::addTextureToCache(tex);
      if (!exported) {
        auto exporter = ReLuxTextureExporterFactory::getExporter(tex);
        str += exporter->exportTexture(tex, "", isForPreview) + "\n";      
      }
    }
    result = str;
  }

  QString getSubdivision( ReMaterial* mat ) {
    // Test to see if the material has modifiers
    auto dmat = dynamic_cast<DisplaceableMaterial*>(mat);
    if (!dmat) {
      return "";
    }

    int subdivisions = dmat->getSubdivisions();
    ReTexturePtr dm = dmat->getDisplacementMap();
    if ( ( subdivisions > 0) || !dm.isNull() ) {
      if ( dmat->usesMicrofacets() ) {
      return QString(
                      "\"string subdivscheme\" [\"microdisplacement\"] "
                      "\"integer nsubdivlevels\" [%1] \"bool dmnormalsmooth\" [\"false\"] "
                      "\"bool dmsharpboundary\" [\"%2\"] "
                    )
                    .arg(subdivisions)
                    .arg(dmat->keepsSharpEdges() ? "true" : "false");        
      }
      else  {
        return QString(
                        "\"string subdivscheme\" [\"loop\"]  \"integer nsubdivlevels\" "
                        "[%1] \"bool dmnormalsmooth\" [\"%2\"]  \"bool dmsharpboundary\" "
                        "[\"%2\"] \"bool dmnormalsplit\" [\"%3\"] "
                      )
                      .arg(subdivisions)
                      .arg(dmat->isSmooth() ? "true" : "false")
                      .arg(dmat->keepsSharpEdges() ? "true" : "false");
      }
    }
    return "";
  };

  QString getDisplacementClause( ReMaterial* mat ) {
    if (!RealitySceneData->isDisplacementEnabled()) {
      return "";
    }
    // Test to see if the material has modifiers
    auto dmat = dynamic_cast<DisplaceableMaterial*>(mat);
    if (!dmat) {
      return "";
    }

    ReTexturePtr dm = dmat->getDisplacementMap();
    if (!dm.isNull()) {
      QString dispMapName = ReLuxTextureExporter::getFloatTextureUniqueName(dm);
      return QString( "\"texture displacementmap\" [\"%1_dispmap\"]"
                      " \"float dmscale\" [1] \"float dmoffset\" [0.0]\n")
               .arg(dispMapName);        
    }
    return ""; 
  }
};

} // namespace

#endif
