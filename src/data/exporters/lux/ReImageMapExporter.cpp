/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReImageMapExporter.h"

#include "ReTexture.h"
#include "ReSceneResources.h"
#include "textures/ReImageMap.h"


namespace Reality {

const QString ReLuxImageMapExporter::exportTexture( ReTexturePtr texbase,
                                                    const QString& assignedName,
                                                    bool isForPreview )
{
  ReImageMapPtr tex = texbase.staticCast<ReImageMap>();
  // This should never happen but sometimes it does so it's pointless
  // to export a texture definition that has no file name and that will 
  // cause a Lux error message
  if (tex->getFileName().isEmpty()) {
    return "";
  }
  QString dataFormat;
  if (tex->getRgbChannel() != RGB_Mean) {
    dataFormat = "float";
  }
  else {
    dataFormat = tex->getDataTypeAsString();
  }

  QString mappingStr;
  switch( tex->getMapping() ) {
    case Re2DTexture::UV:
      mappingStr = "uv";
      break;
    case Re2DTexture::CYLINDRICAL: 
      mappingStr = "cylindrical";
      break;
    case Re2DTexture::SPHERICAL: 
      mappingStr = "spherical";
      break;
    case Re2DTexture::PLANAR: 
      mappingStr = "planar";
      break;
  }

  QString imFileName = tex->getFileName();

#if defined(_WIN32)
  imFileName.replace('\\', '/');
#endif
  if (!isForPreview && RealitySceneData->hasTextureCollection()) {
    imFileName = ReSceneResources::getInstance()->collectTexture(
                   imFileName, RealitySceneData->getTextureSize()
                 );
  }
  QString mapType;
  if (tex->isNormalMap()) {
    mapType = "normalmap";
  }
  else {
    mapType = "imagemap";
  }
  QString str = QString("# From channel %1\n").arg(findChannel(tex));
  str += QString(                  
           "Texture \"%1\" \"%2\" \"%3\" \"float uscale\" [%4] \"float vscale\" [%5]\n" 
           " \"float udelta\" [%6] \"float vdelta\" [%7]\n"
           " \"string filename\" [\"%8\"] \"string wrap\" [\"repeat\"]\n"
           " \"string mapping\" [\"%9\"] \"string filtertype\" [\"bilinear\"]\n"
           " \"float gain\" [%10] \"float gamma\" [%11]\n"
         )
         .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  // #1
         .arg(dataFormat)                 // #2
         .arg(mapType)
         .arg(tex->getUTile())            // #3
         // For Poser or Studio we need the texture to be flipped
         
         // vertically
         .arg(-tex->getVTile())           // #4
         .arg(tex->getUOffset())          // #5
         .arg(tex->getVOffset())          // #6 
         .arg(imFileName)                 // #7
         .arg(mappingStr)
         .arg(tex->getGain())
         .arg(tex->getGamma());
  RGBChannel ch = tex->getRgbChannel();
  if (ch != RGB_Mean) {
    str += QString(" \"string channel\" [\"%1\"]\n")
             .arg((ch == RGB_Red ? "red" : ch == RGB_Green ? "green" : "blue"));
  }

  return str;
}

}
