/**
 * \file
 * Reality plug-in
 * Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_MATERIAL_EXPORTER_H
#define RE_QT_MATERIAL_EXPORTER_H

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReAlphaChannelMaterial.h"
#include "ReDisplaceableMaterial.h"
#include "ReMaterial.h"
#include "ReQtTextureExporterFactory.h"
#include "exporters/ReMaterialExporter.h"


namespace Reality {

/**
 * Base class for all the material exporters to the \ref QVariantMap
 * format
 */

class REALITY_LIB_EXPORT ReQtMaterialExporter : public ReMaterialExporter
{

protected:
  QVariantMap matData;

public:
  ReQtMaterialExporter() {
  };
  virtual ~ReQtMaterialExporter() {
  };

  void addBasicInformation( const ReMaterial* baseMat ) {
    matData["type"]            = baseMat->getTypeAsString();
    matData["name"]            = baseMat->getName();
    matData["edited"]          = baseMat->isEdited();
    matData["innerVolume"]     = baseMat->getInnerVolume();
    matData["outerVolume"]     = baseMat->getOuterVolume();
    matData["visibleInRender"] = baseMat->isVisibleInRender();
    matData["acselSetID"]      = baseMat->getAcselSetID();
    matData["acselID"]         = const_cast<ReMaterial*>(baseMat)->getAcselID();
  }

  void exportAlphaChannel( const ReAlphaChannelMaterial* mat ) {
    ReTexturePtr am = mat->getAlphaMap();
    matData["alphaMap"]      = am.isNull() ? "" : am->getName();
    matData["alphaStrength"] = mat->getAlphaStrength();
  }

  //! Exports the bump map, disp.map and other data found in the
  //! Modifiers tab
  void exportModifiers( const DisplaceableMaterial* mat ) {
    matData["isNormalMap"]    = mat->hasNormalMap();
    matData["bmNegative"]     = mat->getBmNegative();
    matData["bmPositive"]     = mat->getBmPositive();
    matData["bmStrength"]     = mat->getBmStrength();
    matData["dmNegative"]     = mat->getDmNegative();
    matData["dmPositive"]     = mat->getDmPositive();
    matData["dmStrength"]     = mat->getDmStrength();
    matData["subdivision"]    = mat->getSubdivisions();
    matData["useMicrofacets"] = mat->usesMicrofacets();
    matData["smoothness"]     = mat->isSmooth();
    matData["keepSharpEdges"] = mat->keepsSharpEdges();
    matData["emitsLight"]     = mat->isEmittingLight();
    matData["lightGain"]      = mat->getLightGain();
  }

  /**
   * The textures need to be serialized in a specific order, with the
   * basic texture listed before the compound ones that refer to the
   * basic textures. Otherwise we risk to restore a compound texture
   * before the texture it refers to is available.
   * The <texNames> list holds the sorted list of texture names to be
   * exported.
   */
  void exportTextures( const ReMaterial* mat, 
                       boost::any& result, 
                       bool isForPreview = false ) {
    QVariantMap texData;
    QStringList texNames;
    // This method returns a list of texture names sorted so that the compound
    // textures are listed after the basic ones.
    mat->findDependencies(texNames);

    QVariantList textures;

    int numTextures = texNames.count();
    ReNodeDictionary matTextures = mat->getTextures();
    for (int i = 0; i < numTextures; i++) {
      ReTexturePtr tex = matTextures.value(texNames[i]);
      if (tex.isNull()) {
        continue;
      }
      auto tExp = ReQtTextureExporterFactory::getExporter(tex);
      textures += tExp->exportTexture(tex.data());
    }
    texData["textures"] = textures;

    QVariantMap channels;
    ReNodeDictionaryIterator ic(mat->getChannels());
    while(ic.hasNext()) {
      ic.next();
      
      ReTexturePtr tex = ic.value();
      // Name of the channel and a flag to declare if the texture exists
      bool texPresent = !tex.isNull();
      channels[ic.key()] = (texPresent ? tex->getName() : "");
    }
    texData["channels"] = channels;
    result = texData;
  }

};

}

#endif
