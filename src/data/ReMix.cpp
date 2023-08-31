/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReMix.h"

#include "ReMaterials.h"


using namespace Reality;

ReMix::ReMix(const QString name, const ReGeometryObject* parent) :
  ReMaterial( name, parent ),
  mixAmount(0.5),
  ownsMaterials(true) 
{
  type = originalType = MatMix;
  channels["mix"] = ReTexturePtr();
}

void ReMix::setMaterial1( ReMaterialPtr mat) {
  mat1 = mat;
};

void ReMix::setMaterial2( ReMaterialPtr mat) {
  mat2 = mat;
};

void ReMix::setMaterials( ReMaterialPtr newMat1, ReMaterialPtr newMat2) {
  mat1 = newMat1;
  mat2 = newMat2;
};

void ReMix::setMixTexture( ReTexturePtr tex ) {
  channels["mix"] = tex;
  addTextureToCatalog(channels["mix"]);
};

void ReMix::setMixAmount( const float newMixAmount) {
  mixAmount = newMixAmount;
};

void ReMix::serialize( QDataStream& dataStream ) const {
  // Call the base class serializer
  ReMaterial::serialize(dataStream);

  dataStream << mixAmount;

  mat1->serialize(dataStream);
  mat2->serialize(dataStream);
};


void ReMix::deserialize( QDataStream& dataStream ) {
  ReMaterial::deserialize( dataStream );
  dataStream >> mixAmount;
  mat1 = ReMaterialPtr(ReMaterialCreator::deserialize(dataStream, parent));
  mat2 = ReMaterialPtr(ReMaterialCreator::deserialize(dataStream, parent));
};



