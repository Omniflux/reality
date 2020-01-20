#ifndef RE_QT_WATER_IMPORTER_H
#define RE_QT_WATER_IMPORTER_H

#include "ReWater.h"
#include "ReQtMaterialImporter.h"
#include <QVariant>

/*
  Class: ReWaterImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class ReQtWaterImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReWaterPtr mat = baseMat.staticCast<ReWater>();
    importBasicInformation(mat, data);
    mat->setRipples( data.value("ripples").toFloat() );
    mat->setPresetNumber( data.value("ripplePreset").toFloat() );
    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }

  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {
    
    ReWaterPtr mat = baseMat.staticCast<ReWater>();

    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setRipples( data.value("ripples").toFloat() );
    mat->setPresetNumber( data.value("ripplePreset").toFloat() );
    mat->setIOR( data.value("IOR", 1.33).toFloat() );
    mat->setKt( QColor(data.value("Kt", "#ffffff").toString()) );
    mat->setClarityAtDepth( data.value("clarityAtDepth", 0.1).toFloat() );

    importModifiers( mat, data );
    importAlphaChannel( mat, data );
    // We import the texture uncoditionally because the tint of the water is
    // not strictly considered as a texture
    importTextures(mat, data, Replace);

    mat->setEdited();
  };

};

}

#endif
