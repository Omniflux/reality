/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReTextureChannelDataModel.h"

#include "ReLogger.h"
#include "ReMaterial.h"


using namespace Reality;

bool ReTextureChannelDataModel::changeTextureType( const ReTextureType newType ) {
  if (texture.isNull()) {
    return false;
  }
  ReMaterial* mat = static_cast<ReMaterial*>(texture->getParent());
  if (!mat) {
    RE_LOG_INFO() << "Material pointer is NULL for texture " 
                  << texture->getName().toStdString();
    return false;
  }
  ReTexturePtr newTex = mat->changeTextureType(texture->getName(), newType);
  if ( !newTex.isNull() ) {
    texture = newTex;
    return true;
  }
  return false;
}
