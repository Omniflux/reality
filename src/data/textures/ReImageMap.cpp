/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "textures/ReImageMap.h"

#include <QCryptographicHash>

#include "ReTools.h"
#include "textures/ReColorMath.h"
#include "textures/ReGrayscale.h"


namespace Reality {

ReImageMap::ReImageMap( const QString name,
                    ReTextureContainer* parentMat, 
                    const QString fName,
                    ReTextureDataType dType ) : 
  Re2DTexture(name, parentMat)  
{  
  type            = TexImageMap;
  gain            = 1.0f;
  // Numeric textures use linear gamma
  gamma           = (dType == ReTexture::color ? 2.2f : 1.0);
  fileName        = fName;
  textureDataType = dType;
  rgbChannel      = RGB_Mean;

  checkIfNormalMap();
};

// Copy ctor
ReImageMap::ReImageMap( const ReImageMap& t2 ) :
  Re2DTexture(t2),
  gain(t2.gain),
  gamma(t2.gamma),
  fileName(t2.fileName),
  rgbChannel(t2.rgbChannel),
  normalMap(t2.normalMap)
{  
  type            = TexImageMap;
  textureDataType = t2.textureDataType;  
}

const QString ReImageMap::getGUID() {
  QString longForm = QString("%1|%2|%3|%4|%5|%6|%7|%8|%9")
                       .arg(getTypeAsString())
                       .arg(getDataTypeAsString())
                       .arg(normalizeRuntimePath(fileName))
                       .arg(gamma)
                       .arg(gain)
                       .arg(uTile)
                       .arg(vTile)
                       .arg(uOffset)
                       .arg(vOffset);
  return QString(
           QCryptographicHash::hash(
             longForm.toAscii(), 
             QCryptographicHash::Sha1
           ).toHex()
         );  
}

// Conversion ctor
ReImageMap::ReImageMap( const ReTexturePtr srcTex ) :
  Re2DTexture(srcTex)
{
  type = TexImageMap;
  gain            = 1.0f;
  gamma           = 2.2f;
  fileName        = "";
  rgbChannel      = RGB_Mean;    
  textureDataType = srcTex->getDataType();
  // Pseudo copy ctor
  switch( srcTex->getType() ) {
    case TexImageMap: {
      ReImageMapPtr t2  = srcTex.staticCast<ReImageMap>();
      gain            = t2->gain;
      gamma           = t2->gamma;
      fileName        = t2->fileName;
      textureDataType = t2->getDataType();
      rgbChannel      = t2->rgbChannel;
      normalMap       = t2->normalMap;
      break;
    }
    case TexColorMath: {
      ReColorMathPtr cmTex = srcTex.staticCast<ReColorMath>();
      ReTexturePtr cmTex1 = cmTex->getTexture1();
      normalMap = false;
      if ( !cmTex1.isNull() && cmTex1->getType() == TexImageMap ) {
        fileName = cmTex1->getNamedValue("fileName").toString();
        gain     = cmTex1->getNamedValue("gain").toFloat();
        gamma    = cmTex1->getNamedValue("gamma").toFloat();
        checkIfNormalMap();
      }
      break;
    }
    case TexMath: {
      auto t1 = srcTex->getNamedValue("texture1").value<ReTexturePtr>();
      if (t1->getType() == TexImageMap) {
        copyProperties(t1);
      }
      break;
    }
    case TexMix: {
      auto t1 = srcTex->getNamedValue("tex1").value<ReTexturePtr>();
      if (t1 && t1->getType() == TexImageMap) {
        copyProperties(t1);
      }
      else {
        auto imgTex = findImageMapTexture(srcTex);
        if (!imgTex.isNull()) {
          copyProperties(imgTex);
        }
      }
      break;
    }
    case TexGrayscale: {
      auto t = srcTex.staticCast<ReGrayscale>()->getTexture();
      if (t->getType() == TexImageMap) {
        fileName = t->getNamedValue("fileName").toString();
        textureDataType = color;
        gain = t->getNamedValue("gain").toFloat();
        gamma = t->getNamedValue("gamma").toFloat();
        rgbChannel = static_cast<RGBChannel>(
                       t->getNamedValue("rgbChannel").toInt()
                     );

      }
      break;
    }
    default: {
      checkIfNormalMap();
    }
  }
}

bool ReImageMap::checkIfNormalMap( const QString fName ) {
  QString lcFileName = fName.toLower();
  if (lcFileName.contains("nrm")    || 
      lcFileName.contains("norm")   || 
      lcFileName.contains("normal") || 
      lcFileName.contains("nmap")) 
  {
    return true;
  }
  return false;
}


void ReImageMap::checkIfNormalMap() {
  QString lcFileName = fileName.toLower();
  if (lcFileName.contains("nrm")    || 
      lcFileName.contains("norm")   || 
      lcFileName.contains("normal") || 
      lcFileName.contains("nmap")) 
  {
    normalMap = true;
  }
  else {
    normalMap = false;
  }  
}

// Texture comparison, useful for the texture cache.
bool ReImageMap::operator ==( const ReImageMap& t2) const {
  return( 
          fileName        == t2.fileName  && 
          uTile           == t2.uTile     &&
          vTile           == t2.vTile     &&
          uOffset         == t2.uOffset   &&
          vOffset         == t2.vOffset   &&
          textureDataType == t2.textureDataType &&
          rgbChannel      == t2.rgbChannel &&
          normalMap       == t2.normalMap
  );
};

RGBChannel ReImageMap::getRgbChannel() const {
  return rgbChannel;
};

void ReImageMap::setRgbChannel( RGBChannel newVal ) {
  rgbChannel = newVal;
};

QString ReImageMap::toString() {
  return QString("Reality::ReImageMap(%1) => %2").arg(name).arg(fileName);
}

void ReImageMap::serialize( QDataStream& dataStream ) {
  Re2DTexture::serialize(dataStream);
  dataStream << gain << gamma << (quint16) rgbChannel
             << fileName << normalMap;
};

void ReImageMap::deserialize( QDataStream& dataStream ) {
  Re2DTexture::deserialize(dataStream);
  quint16 rgbChannelInt; 
  dataStream >> gain >> gamma >> rgbChannelInt >> fileName >> normalMap;
  rgbChannel = static_cast<RGBChannel>(rgbChannelInt);
};

void ReImageMap::copyProperties( const ReTexturePtr srcTex ) {
  textureDataType = srcTex->getDataType();
  setNamedValue("fileName", srcTex->getNamedValue("fileName"));
  setNamedValue("gain", srcTex->getNamedValue("gain"));
  setNamedValue("gamma", srcTex->getNamedValue("gamma"));
  setNamedValue("rgbChannel", srcTex->getNamedValue("rgbChannel"));
  setNamedValue("normalMap", srcTex->getNamedValue("normalMap"));
  Re2DTexture::copyProperties(srcTex);
}

void ReImageMap::setNamedValue( const QString& vname, const QVariant& value ) {
  if (vname == "fileName") {
    setFileName(value.toString());
  }
  else if (vname == "gain") {
    gain = value.toDouble();
  }
  else if (vname == "gamma") {
    gamma = value.toDouble();
  }
  else if (vname == "rgbChannel") {
    rgbChannel = static_cast<RGBChannel>(value.toInt());
  }
  else if (vname == "normalMap") {
    setNormalMap(value.toBool());
  }
  else {
    Re2DTexture::setNamedValue(vname, value);
  }
}

const QVariant ReImageMap::getNamedValue( const QString& vname ) {
  if (vname == "fileName") {
    return QVariant(fileName);
  }
  else if (vname == "gain") {
    return QVariant(gain);
  }
  else if (vname == "gamma") {
    return QVariant(gamma);
  }
  else if (vname == "rgbChannel") {
    return QVariant(rgbChannel);
  }
  else if (vname == "normalMap") {
    return normalMap;
  }
  return Re2DTexture::getNamedValue(vname);
}

}
