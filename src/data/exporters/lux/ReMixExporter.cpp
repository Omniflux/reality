/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/


#include "ReLuxMaterialExporterFactory.h"
#include "ReMix.h"
#include "ReMixExporter.h"

using namespace Reality;

void ReMixLuxExporter::exportMaterial( const ReMaterial* basemat,
                                       boost::any& result ) 
{
  if (basemat->getType() != MatMix) {
    result = QString();
    return;
  }
  auto mat = static_cast<const ReMix*>(basemat);
  QString str;
  QString matName = mat->getUniqueName();

  // The Mix material is responsible for exporting the 
  // linked materials if it has ownership of the linked
  // materials
  if (mat->hasOwnership()) {
    ReMaterialPtr mat1 = mat->getMaterial1();
    ReMaterialPtr mat2 = mat->getMaterial2();
    ReLuxMaterialExporterPtr exporter = ReLuxMaterialExporterFactory::getExporter(
      mat1.data()
    );
    boost::any textureData;
    exportTextures(mat1.data(), textureData);
    str += boost::any_cast<QString>(textureData);

    exportTextures(mat2.data(), textureData);
    str += boost::any_cast<QString>(textureData);

    boost::any matData1, matData2;
    exporter->exportMaterial(mat1.data(), matData1);
    exporter->exportMaterial(mat2.data(), matData2);
    str += boost::any_cast<QString>(matData1);
    str += boost::any_cast<QString>(matData2);
  }

  str += QString("#\n# Mat: %1\n#\n").arg(matName);
  if (mat->hasMixTexture()) {
    str += QString(
          "MakeNamedMaterial \"%1\" \"string type\" [\"mix\"] "
          "\"texture amount\" [\"%2\"] "
        )
        .arg(matName)
        .arg(mat->getMixTexture()->getUniqueName());
  }
  else {
    str += QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"mix\"] "
             "\"float amount\" [\"%2\"] "
           )
           .arg(matName)
           .arg(mat->getMixAmount());
  }
  str += QString(
           "\"string namedmaterial1\" [\"%1\"] "
           "\"string namedmaterial2\" [\"%2\"] \n"
         )
         .arg(mat->getMaterial1()->getUniqueName())
         .arg(mat->getMaterial2()->getUniqueName());
  result = str;
}
