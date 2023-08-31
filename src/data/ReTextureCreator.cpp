/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReTextureCreator.h"

#include "importers/qt/ReQtTextureImporter.h"
#include "importers/qt/ReQtTextureImporterFactory.h"
#include "textures/ReBand.h"
#include "textures/ReBricks.h"
#include "textures/ReCheckers.h"
#include "textures/ReClouds.h"
#include "textures/ReColorMath.h"
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

ReTexture* ReTextureCreator::createTexture( const QString texName,
                                          const ReTextureType texType,
                                          ReTextureContainer* parentMat,
                                          const ReTexture::ReTextureDataType dataType ) 
{
  ReTexture* tex = NULL;
  switch(texType) {
    case TexImageMap: {
      tex = new ReImageMap(texName, parentMat, "", dataType);
      break;
    }
    case TexMath: {
      tex = new ReMath(texName, parentMat);
      break;
    }
    case TexColorMath: {
      tex = new ReColorMath(texName, parentMat);
      break;
    }
    case TexBand: {
      tex = new ReBand(texName, parentMat);
      break;
    }
    case TexBricks: {
      tex = new ReBricks(texName, parentMat, dataType);
      break;
    }
    case TexCheckers: {
      tex = new ReCheckers(texName, parentMat);
      break;
    }
    case TexClouds: {
      tex = new ReClouds(texName, parentMat);
      break;
    }
    case TexConstant: {
      tex = new ReConstant(texName, parentMat);
      tex->setTextureDataType(dataType);
      break;
    }
    case TexFBM: {
      tex = new ReFBM(texName, parentMat);
      break;
    }
    case TexFresnelColor: {
      tex = new ReFresnelColor(texName, parentMat);
      break;
    }
    case TexMarble: {
      tex = new ReMarble(texName, parentMat);
      break;
    }
    case TexWood: {
      tex = new ReWood(texName, parentMat);
      break;
    }
    case TexMix: {
      tex = new ReMixTexture(texName, parentMat, dataType);
      break;
    }
    case TexMultiMix: {
      tex = new MultiMix(texName, parentMat);
      break;
    }
    case TexDistortedNoise: {
      tex = new ReDistortedNoise(texName, parentMat);
      break;
    }
    case TexInvertMap: {
      tex = new InvertMap(texName, parentMat);
      break;
    }
    case TexGrayscale: {
      tex = new ReGrayscale(texName, parentMat);
      break;
    }
    case TexUndefined: {
      // Nothing yet, throw an exception?
    }
  }
  return tex;
}

ReTexture* ReTextureCreator::createTexture( const QString texName,
                                          const ReTexturePtr baseTex ) 
{
  ReTexture* tex = NULL;
  switch(baseTex->type) {
    case TexImageMap: {
      tex = new ReImageMap(baseTex);
      break;
    }
    case TexMath: {
      tex = new ReMath(baseTex);
      break;
    }
    case TexColorMath: {
      tex = new ReColorMath(baseTex);
      break;
    }
    case TexBand: {
      tex = new ReBand(baseTex);
      break;
    }
    case TexBricks: {
      tex = new ReBricks(baseTex);
      break;
    }
    case TexCheckers: {
      tex = new ReCheckers(baseTex);
      break;
    }
    case TexClouds: {
      tex = new ReClouds(baseTex);
      break;
    }
    case TexConstant: {
      tex = new ReConstant(baseTex);
      break;
    }
    case TexFBM: {
      tex = new ReFBM(baseTex);
      break;
    }
    case TexFresnelColor: {
      tex = new ReFresnelColor(baseTex);
      break;
    }
    case TexMarble: {
      tex = new ReMarble(baseTex);
      break;
    }
    case TexMix: {
      tex = new ReMixTexture(baseTex);
      break;
    }
    case TexMultiMix: {
      tex = new MultiMix(baseTex);
      break;
    }
    case TexDistortedNoise: {
      tex = new ReDistortedNoise(baseTex);
      break;
    }
    case TexWood: {
      tex = new ReWood(baseTex);
      break;
    }
    case TexInvertMap: {
      tex = new InvertMap(baseTex);
      break;
    }
    case TexGrayscale: {
      tex = new ReGrayscale(baseTex);
      break;
    }
    case TexUndefined: {
      // Nothing yet, throw an exception?
    }
  }
  tex->setName(texName);
  return tex;  
}

ReTexturePtr ReTextureCreator::createTexture( const QVariantMap& data,
                                            ReTextureContainer* parentMat ) 
{

  ReTexturePtr tex;
  QString texName = data.value("name").toString();
  ReTextureType texType = static_cast<ReTextureType>(data.value("type").toInt());
  tex = ReTexturePtr(
          createTexture( texName, 
                         texType, 
                         parentMat ) 
        );
  ReQtTextureImporterPtr texImporter = ReQtTextureImporterFactory::getImporter(texType);
  texImporter->restoreTexture(tex, data);
  return tex;
}

ReTexturePtr ReTextureCreator::deserialize( QDataStream& dataStream,
                                          ReTextureContainer* parentMat ) 
{

  // Save the position before looking ahead
  quint64 currentPos = dataStream.device()->pos();
  quint16 type, dataType;
  QString name;
  dataStream >> type >> name >> dataType;
  // Restore the original position
  dataStream.device()->seek(currentPos);

  ReTexture* tex = createTexture( name, 
                                  static_cast<ReTextureType>(type), 
                                  parentMat,
                                  static_cast<ReTexture::ReTextureDataType>(dataType) );
  if (tex) {
    tex->deserialize(dataStream);      
  }
  return ReTexturePtr(tex);
}
  

}