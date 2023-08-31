/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_METAL_LUX_EXPORTER_H
#define RE_METAL_LUX_EXPORTER_H

#include <QDesktopServices>
#include <QFile>

#include "reality_lib_export.h"
#include "ReMaterial.h"
#include "ReMetal.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReSceneResources.h"
#include "ReTexture.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "exporters/lux/ReLuxTextureExporter.h"


/*
  Class: ReMetalLuxExporter
 */

namespace Reality {

class REALITY_LIB_EXPORT ReMetalLuxExporter : public ReLuxMaterialExporter {

public:
  QString getNkFileName( ReMetal::MetalType metalType ) {
    QString metalPrefix;
    switch(metalType) {
      case ReMetal::Aluminum:
        metalPrefix = "Al";
        break;
      case ReMetal::Chrome:
        metalPrefix = "Cr";
        break;
      case ReMetal::Cobalt:
        metalPrefix = "Co_2";
        break;
      case ReMetal::Copper:
        metalPrefix = "Cu";
        break;
      case ReMetal::Gold:
        metalPrefix = "Au";
        break;
      case ReMetal::Lead:
        metalPrefix = "Pbs";
        break;
      case ReMetal::Mercury:
        metalPrefix = "Hgcdte0";
        break;
      case ReMetal::Nickel:
        metalPrefix = "Ni";
        break;
      case ReMetal::Platinum:
        metalPrefix = "Pt";
        break;
      case ReMetal::Silver:
        metalPrefix = "Ag";
        break;
      case ReMetal::Titanium:
        metalPrefix = "Ti";
        break;
      case ReMetal::Zinc:
        metalPrefix = "Zncdte0";
        break;
      case ReMetal::Custom:
        break;
    }
    return QString("%1.nk").arg(metalPrefix);
  }

  QString writeNkFile( QString nkFileName ) {
      // The scene resources are initialized only when exporting to Lux, not during
      // the material preview.
      ReSceneResources* sceneRes = ReSceneResources::getInstance();
      QString nkFileDestDir;
      if ( sceneRes->isInitialized() ) {
        nkFileDestDir = sceneRes->getResourcePath();
      }
      else {
        nkFileDestDir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
      }
      QFile metalFile(QString(":/metals/%1").arg(nkFileName));
      if ( !metalFile.open(QIODevice::ReadOnly) ) {
        return "";
      }
      QByteArray nkFileContent = metalFile.readAll();
      metalFile.close();
      QString metalFileName = QString("%1/%2").arg(nkFileDestDir).arg(nkFileName);
      metalFile.setFileName(metalFileName);
      if (!metalFile.open(QIODevice::WriteOnly)) {
        return "";
      }
      metalFile.write(nkFileContent);
      metalFile.close();
      return metalFileName;
  }

  const QString exportBaseMaterial( const ReMetal* mat, 
                                    const QString matName )  
  {
    QString str;

    ReMetal::MetalType mType = mat->getMetalType();
    QString fresnelTextureName = QString("%1_fresnel").arg(matName);

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
    fresnelTextureName.replace('\\', '/');
#endif

    if (mType == ReMetal::Custom) {
      auto Kr = ReLuxTextureExporter::getTextureFromCache(mat->getKr());
      if (!Kr.isNull()) {
        str += QString(
                 "Texture \"%1\" \"fresnel\" \"fresnelcolor\" \"texture Kr\" [\"%2\"]\n"
               )
               .arg(fresnelTextureName)
               .arg(Kr->getUniqueName());
      }
    }
    else {
      QString nkFileName = getNkFileName(mType);
      QString metalFileName = writeNkFile(nkFileName);
      if (metalFileName == "") {
        return "";
      }

      str += QString(
               "Texture \"%1\" \"fresnel\" \"sopra\" \"string filename\" [\"%2\"]\n"
             )
             .arg(fresnelTextureName)
             .arg(metalFileName);
    }
    str += QString("# Mat: %1\n").arg(matName);

    str += QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"metal2\"] "
             "\"texture fresnel\" [\"%2\"] "
           )
           .arg(matName)
           .arg(fresnelTextureName);             

    // The polish is expressed in the UI with a range of 0-10000. Lux uses a "roughness"
    // parameter, opposite in idea, which is in the range of 0.0-1.0. The following
    // formulas convert from one systen to the other
    double hr = 1.0 - mat->getHPolish()/10000.0;
    double vr = 1.0 - mat->getVPolish()/10000.0;
    // If the roughness approaches 0 then Lux will render the surface as black. We apply a small 
    // correction for that case
    if (hr < 0.0005) {
      hr = 0.0008;
    };
    if (vr < 0.0005) {
      vr = 0.0008;
    };
    str += QString(" \"float uroughness\" [%1] \"float vroughness\" [%2]\n")
             .arg(hr)
             .arg(vr);  

    // Bump map.The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    ReTexturePtr bumpMap = mat->getBumpMap();
    QString bmTextures;
    if (!bumpMap.isNull()) {
      QString bumpMapTexName = makeBumpMap(mat,
                                           bumpMap, 
                                           bmTextures, 
                                           mat->getBmNegative(), 
                                           mat->getBmPositive(), 
                                           mat->getBmStrength());
      str += QString(" \"texture bumpmap\" [\"%1\"] ").arg(bumpMapTexName);
    }

    // Displacement map. The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    QString dmTextures;
    if (RealitySceneData->isDisplacementEnabled()) {
      ReTexturePtr dispMap = mat->getDisplacementMap();
      if (!dispMap.isNull()) {
         makeDispMap( mat,
                      dispMap, 
                      dmTextures, 
                      mat->getDmNegative(), 
                      mat->getDmPositive(), 
                      mat->getDmStrength() );
      }
    }

    return bmTextures % dmTextures % str;
  }

  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatMetal) {
      result = QString();
      return;
    }
    auto mat = static_cast<const ReMetal*>(basemat);
    QString str;
    QString matName = mat->getUniqueName();

    // Alpha channel/map handling...
    ReTexturePtr alphaMap = ReLuxTextureExporter::getTextureFromCache(
                              mat->getAlphaMap()
                            );
    float alphaStrength = mat->getAlphaStrength();

    if (!alphaMap.isNull() || alphaStrength < 1.0) {
      str = exportBaseMaterial(mat, QString("%1_BaseMat").arg(matName));
      str += makeAlphaChannel(matName, alphaMap, alphaStrength);
    }
    else {
      str = exportBaseMaterial(mat, matName);
    }
    result = str;
  }
};

}

#endif
