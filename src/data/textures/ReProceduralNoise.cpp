/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReProceduralNoise.h"

#include "ReTools.h"

namespace Reality {

QString ReProceduralNoise::lutTypesToStrings[ReProceduralNoise::BLENDER_ORIGINAL+1] = {
  QString("Cell noise"),
  QString("Crackle"),
  QString("Voronoi 1"),
  QString("Voronoi 2"),
  QString("Voronoi 2-1"),
  QString("Voronoi 3"),
  QString("Voronoi 4"),
  QString("Perlin Improved"),
  QString("Perlin Original"),
  QString("Blender original")
};

QString ReProceduralNoise::lutLuxNoiseBasisNames[BLENDER_ORIGINAL+1] = {
  QString("cell_noise"),
  QString("voronoi_crackle"),
  QString("voronoi_f1"),
  QString("voronoi_f2"),
  QString("voronoi_f2_f1"),
  QString("voronoi_f3"),
  QString("voronoi_f4"),
  QString("improved_perlin"),
  QString("original_perlin"),
  QString("blender_original")
};

ReProceduralNoise::ReProceduralNoise(const QString name, ReTextureContainer* parentMat ) : 
  ReTexture3D(name, parentMat),
  noiseSize(0.25),
  brightness(1.0),
  contrast(1.0),
  noiseBasis(BLENDER_ORIGINAL) 
{
  textureDataType = numeric;
}

// Texture conversion ctor
ReProceduralNoise::ReProceduralNoise( const ReTexturePtr srcTex ) :
  ReTexture3D(srcTex),
  noiseSize(0.25),
  brightness(1.0),
  contrast(1.0),
  noiseBasis(BLENDER_ORIGINAL) 
{
  textureDataType = numeric;

  ReProceduralNoisePtr t2 = srcTex.dynamicCast<ReProceduralNoise>();
  if (!t2.isNull()) {
    noiseSize  = t2->noiseSize;
    brightness = t2->brightness;
    contrast   = t2->contrast;
    noiseBasis = t2->noiseBasis;
  }

}

  
void ReProceduralNoise::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "noiseBasis") {
    setNoiseBasis(static_cast<Reality::ReProceduralNoise::NoiseDistortionType>(value.toInt()));
  }
  else if (vname == "noiseSize") {
    setNoiseSize(value.toDouble());
  }
  else if (vname == "brightness") {
    setBrightness(value.toDouble());
  }
  else if (vname == "contrast") {
    setContrast(value.toDouble());
  }
  else {
    ReTexture3D::setNamedValue(vname, value);
  }
}

const QVariant ReProceduralNoise::getNamedValue( const QString& vname ) {
  if (vname == "noiseBasis") {
    return noiseBasis;
  }
  else if (vname == "noiseSize") {
    val = getNoiseSize();
  }
  else if (vname == "brightness") {
    val = getBrightness();
  }
  else if (vname == "contrast") {
    val = getContrast();
  }
  else {
    return ReTexture3D::getNamedValue(vname);
  }
  return val;
}

void ReProceduralNoise::copyProperties( const ReTexturePtr srcTex ) {
  setNamedValue("noiseBasis", srcTex->getNamedValue("noiseBasis"));
  setNamedValue("noiseSize", srcTex->getNamedValue("noiseSize"));
  setNamedValue("brightness", srcTex->getNamedValue("brightness"));
  setNamedValue("contrast", srcTex->getNamedValue("contrast"));
}


void ReProceduralNoise::serialize( QDataStream& dataStream ) {
  ReTexture3D::serialize(dataStream);
  dataStream << noiseSize << (quint16) noiseBasis << brightness << contrast;
};

void ReProceduralNoise::deserialize( QDataStream& dataStream ) {
  ReTexture3D::deserialize(dataStream);
  quint16 tmpBasis;
  dataStream >> noiseSize >> tmpBasis >> brightness >> contrast;
  noiseBasis = (NoiseDistortionType) tmpBasis;
};



}