/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_WATER_LUX_EXPORTER_H
#define RE_WATER_LUX_EXPORTER_H

#include <boost/any.hpp>

#include "reality_lib_export.h"
#include "ReMaterial.h"
#include "ReWater.h"
#include "exporters/lux/ReLuxMaterialExporter.h"


/*
  Class: ReWaterLuxExporter
 */

namespace Reality {

class REALITY_LIB_EXPORT ReWaterLuxExporter : public ReLuxMaterialExporter {

public:
  
  void exportMaterial( const ReMaterial* basemat, boost::any& result ) {
    if (basemat->getType() != MatWater) {
      result = QString();
      return;
    }
    auto mat = static_cast<const ReWater*>(basemat);
    QString str;
    QString matName = mat->getUniqueName();
    str = QString("# Mat: %1\n").arg(matName);

    // Bump map.The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    ReTexturePtr bumpMap = mat->getBumpMap();
    QString bmTextures;
    QString bmClause;
    if (!bumpMap.isNull()) {
      QString bumpMapTexName = makeBumpMap( mat,
                                            bumpMap, 
                                            bmTextures, 
                                            mat->getBmNegative(), 
                                            mat->getBmPositive(), 
                                            mat->getBmStrength() );
      bmClause = QString(" \"texture bumpmap\" [\"%1\"] ").arg(bumpMapTexName);
    }

    // Displacement map. The method needs to write several textures into the scene 
    // before the material is exported. This must NOT fetch the texture
    // from the cache
    ReTexturePtr dispMap = mat->getDisplacementMap();
    QString dmTextures;
    if (!dispMap.isNull()) {
      QString dispMapTexName = makeDispMap( mat,
                                            dispMap, 
                                            dmTextures, 
                                            mat->getDmNegative(), 
                                            mat->getDmPositive(), 
                                            mat->getDmStrength() );
    }

    str += QString(
             "MakeNamedMaterial \"%1\" \"string type\" [\"glass2\"] "
             "\"bool architectural\" [\"false\"] \"bool dispersion\" [\"false\"] %2\n"
           )
           .arg(matName)
           .arg(bmClause);
           
    result =  QString(bmTextures % dmTextures % str);
  }
};

}

#endif
