/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_STATUE_LUX_EXPORTER_H
#define RE_STATUE_LUX_EXPORTER_H

#include <boost/any.hpp>

#include "reality_lib_export.h"
#include "ReAlphaChannelMaterial.h"
#include "ReMaterial.h"
#include "ReTexture.h"
#include "exporters/lux/ReLuxMaterialExporter.h"


/**
  Exports the Matte material to Lux
 */
namespace Reality {

class REALITY_LIB_EXPORT ReStatueLuxExporter : public ReLuxMaterialExporter {
private:

  inline QString exportBaseMat( const QString& matName ) {
    float Kd = 0.8;
    return QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"matte\"] "
             "\"color Kd\" [%2 %2 %2] \"float sigma\" [%3] \n"
           )
           .arg(matName)
           .arg(Kd)
           .arg(0.4);    
  }
public:
  
  void exportMaterial( const ReMaterial* baseMat, boost::any& result ) {
    QString str;

    QString matName = baseMat->getUniqueName();
    auto mat = static_cast<const ReAlphaChannelMaterial*>(baseMat);

    // Alpha channel/map handling...
    ReTexturePtr alphaMap = mat->getAlphaMap();
    float alphaStrength = mat->getAlphaStrength();

    if (!alphaMap.isNull() || alphaStrength < 1.0) {
      str = exportBaseMat( QString("%1_BaseMat").arg(matName) );
      str += makeAlphaChannel(matName, alphaMap, alphaStrength);
    }
    else {
      str += exportBaseMat( matName );
    }
    result = str;
  }
};

}

#endif
