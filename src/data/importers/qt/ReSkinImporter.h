#ifndef RE_QT_SKIN_IMPORTER_H
#define RE_QT_SKIN_IMPORTER_H

#include "ReSkin.h"
#include "ReQtMaterialImporter.h"
#include <QVariant>

/*
  Class: ReSkinImporter

  Imports the material definition from a QVariantMap
 */
namespace Reality {

class ReQtSkinImporter : public ReQtMaterialImporter {

public:
  
  void importMaterial( ReMaterialPtr baseMat, const QVariantMap& data ) {    
    ReSkinPtr mat = baseMat.staticCast<ReSkin>();
    importBasicInformation(mat, data);
    mat->setUGlossiness(data.value("uGlossiness").toFloat());
    mat->setVGlossiness(data.value("vGlossiness").toFloat());
    mat->setSurfaceFuzz(data.value("surfaceFuzz").toBool());
    mat->setFresnelAmount(data.value("fresnelAmount",5).toFloat());

    //! This import is here because of a bug in Reality 3.0 and 3.1
    //! The Absorption scale is loaded from the Volumes settings.
    //! In 3.0 and 3.1 the volume data was saved with each material, which was
    //! a mistake. The same data was saved also with the volume settings.
    //! At the same time, the absorption scale parameter was not saved with 
    //! the volumes settings, which was another bug. So, for scene that 
    //! were saved with Reality 3.0 and 3.1 we need to read the absorption scale
    //! from the material data.
    if (data.contains("absorptionScale")) {
      mat->setNamedValue(
          "absorptionScale", 
          data.value("absorptionScale").toFloat()
      );
    }

    mat->setSSSEnabled(data.value("sssEnabled").toBool());
    mat->setHairMask(data.value("hairMask").toBool());
    mat->setHmGain(data.value("hmGain").toFloat());
    mat->setCoatThickness(data.value("coatThickness", RE_GLOSSY_COAT_DEFAULT_THICKNESS).toFloat());
    mat->setTopCoat(data.value("topCoat", false).toBool());

    importAlphaChannel(mat, data);
    importModifiers(mat, data);
  }

  void importFromClipboard( ReMaterialPtr baseMat, 
                            const QVariantMap& data, 
                            const ReplaceTextureMode texMode = Keep ) {
    
    ReSkinPtr mat = baseMat.staticCast<ReSkin>();

    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setUGlossiness(data.value("uGlossiness").toFloat());
    mat->setVGlossiness(data.value("vGlossiness").toFloat());
    mat->setSurfaceFuzz(data.value("surfaceFuzz").toBool());
    mat->setSSSEnabled(data.value("sssEnabled").toBool());
    mat->setHairMask(data.value("hairMask").toBool());
    mat->setHmGain(data.value("hmGain").toFloat());
    mat->setFresnelAmount(data.value("fresnelAmount").toFloat());
    importModifiers( mat, data );
    importAlphaChannel( mat, data );
    if ( texMode != Keep ) {
      importTextures(mat, data, texMode);
    }

    mat->setEdited();
  };

};

}

#endif
