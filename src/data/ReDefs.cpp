/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReDefs.h"

#include <QString>


QString Reality::ReTextureTypesForUI[TexUndefined+1] = {
  QString(QObject::tr("Image Map")),
  QString(QObject::tr("Color")),
  QString(QObject::tr("Texture Combine")),
  QString(QObject::tr("Math")),
  QString(QObject::tr("Band")),
  QString(QObject::tr("Bricks")),
  QString(QObject::tr("Checkers")),
  QString(QObject::tr("Clouds")),
  QString(QObject::tr("Fractal noise")),
  QString(QObject::tr("Fresnel")),
  QString(QObject::tr("Marble")),
  QString(QObject::tr("Mix")),
  QString(QObject::tr("Multi-Mix")),
  QString(QObject::tr("Distorted Noise")),
  QString(QObject::tr("Wood")),
  QString(QObject::tr("Inverted Map")),
  QString(QObject::tr("Grayscale")),
  QString(QObject::tr("Undefined"))
}; 

// Initialization of static table
quint8 Reality::ReTextureDataTypes[TexUndefined+1] = {
  RE_TEX_DATATYPE_COLOR_NUMERIC,  //TexImageMap,
  RE_TEX_DATATYPE_COLOR_NUMERIC,  //TexConstant,
  RE_TEX_DATATYPE_COLOR,          //TexColorMath,
  RE_TEX_DATATYPE_NUMERIC,        //TexMath,
  RE_TEX_DATATYPE_ALL,            //TexBand,
  RE_TEX_DATATYPE_COLOR_NUMERIC,  //TexBricks,      
  RE_TEX_DATATYPE_NUMERIC,        //TexCheckers,
  RE_TEX_DATATYPE_NUMERIC,        //TexClouds,
  RE_TEX_DATATYPE_NUMERIC,        //TexFBM,
  RE_TEX_DATATYPE_FRESNEL,        //TexFresnelColor,
  RE_TEX_DATATYPE_NUMERIC,        //TexMarble,
  RE_TEX_DATATYPE_ALL,            //TexMix,
  RE_TEX_DATATYPE_ALL,            //TexMultiMix,
  RE_TEX_DATATYPE_NUMERIC,        //TexDistortedNoise,
  RE_TEX_DATATYPE_NUMERIC,        //TexWood,
  RE_TEX_DATATYPE_COLOR,          //TexInvertMap,
  RE_TEX_DATATYPE_COLOR_NUMERIC,  //TexGrayscale,
  0x00                            //TexUndefined
};
