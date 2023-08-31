/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_COLOR_MATH_TEXTURE_EXPORTER_H
#define LUX_COLOR_MATH_TEXTURE_EXPORTER_H

#include <QColor>

#include "reality_lib_export.h"
#include "ReLogger.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReTexture.h"
#include "ReTools.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReLuxTextureExporterFactory.h"
#include "textures/ReColorMath.h"


namespace Reality {

/*
  Class: ReLuxColorMathExporter
 */

class REALITY_LIB_EXPORT ReLuxColorMathExporter : public ReLuxTextureExporter {

private:
  QString multiplyColorAndTexture( const QColor& col, 
                                   const ReTexturePtr tex, 
                                   const QString texName ) const {

    QColor gcColor = col;
    gammaCorrect(gcColor, RealitySceneData->getGamma());
    return QString("Texture \"%1\" \"color\" \"scale\" \"color tex1\" [%2 %3 %4] " 
                  "\"texture tex2\" [\"%5\"] \n")
             .arg(texName) 
             .arg(gcColor.redF())
             .arg(gcColor.greenF())
             .arg(gcColor.blueF())
             .arg(tex->getUniqueName());
  }

public:
  const QString exportTexture( ReTexturePtr texbase, const QString& assignedName = "", bool isForPreview = false ) {

    ReColorMathPtr tex = texbase.staticCast<ReColorMath>();
    QString str = QString("# From channel %1\n").arg(findChannel(texbase));

    QString functionStr;
    ReColorMath::functions function = tex->getFunction();
    switch( function ) {
      case ReColorMath::add:
        functionStr = "add";
        break;
      case ReColorMath::subtract:
        functionStr = "subtract";
        break;
      case ReColorMath::multiply:
      case ReColorMath::none:
        functionStr = "scale";      
        break;
    }
    QColor col1 = tex->getColor1();
    QColor col2 = tex->getColor2();
    ReTexturePtr tex1 = ReLuxTextureExporter::getTextureFromCache(
                          tex->getTexture1()
                        );
    ReTexturePtr tex2 = ReLuxTextureExporter::getTextureFromCache(
                          tex->getTexture2()
                        );
    bool tex2Present = !tex2.isNull();

    if (tex1.isNull()) {
      RE_LOG_DEBUG() << "Warning: texture1 of " << QSS(tex->getName()) << " is NULL!";
      return "";
    }

    // This is correct.
    QString tex1Name = tex->getUniqueName();

    bool tex1Exported = false;
    // If we have a color to mix with the texture then let's export
    // that scale texture.
    if (!isPureWhite(col1)) {
      // If we have both textures then we will need to mix this
      // texture with the second one. This will require the output
      // of the Color Math texture. Otherwise the mix of color and
      // first texture is our only output so it gets named using
      // the name of the main ColorMath texture.
      if (function != ReColorMath::none) {
        tex1Name = QString("%1_colormix1").arg(tex1Name);        
      }      
      str += multiplyColorAndTexture(col1, tex1, tex1Name);
      tex1Exported = true;
    }
    else {
      // Color1 is pure white so we just output texture1 straight
      tex1Name = tex1->getUniqueName();
    }

    // Do we have a second slot valid? Either just the color or
    // combination of color2 and texture2
    if (function != ReColorMath::none) {
      if (tex2Present) {
        QString tex2Name = tex2->getUniqueName();
        // If we have a color to mix with the texture then let's export
        // that scale texture.
        if (!isPureWhite(col2)) {
          tex2Name = QString("%1_colormix2").arg(tex2Name);
          str += multiplyColorAndTexture(col2, tex2, tex2Name);
        }

        str += QString("Texture \"%1\" \"%2\" \"%3\" \"texture tex1\" [\"%4\"] " 
                      "\"texture tex2\" [\"%5\"] ")
                .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  // #1
                .arg(tex->getDataTypeAsString())
                .arg(functionStr)
                .arg(tex1Name)
                .arg(tex2Name);
      }
      else {
        gammaCorrect(col2,RealitySceneData->getGamma());
        str += QString("Texture \"%1\" \"%2\" \"%3\" \"texture tex1\" [\"%4\"] " 
                      "\"color tex2\" [%5 %6 %7] ")
                .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  // #1
                .arg(tex->getDataTypeAsString())
                .arg(functionStr)
                .arg(tex1Name)
                .arg(col2.redF())
                .arg(col2.greenF())
                .arg(col2.blueF());

      }
    }
    else {
      if (!tex1Exported) {
        // We only have the first texture, so we export that one only
        if (isPureWhite(col1)) {
          ReLuxTextureExporterPtr texExp = ReLuxTextureExporterFactory::getExporter(tex1);
          str += texExp->exportTexture(tex1, tex->getUniqueName());
        }
        else {
          str += multiplyColorAndTexture(col1, tex1, tex1Name);
        }
      }
    }

    return str;
  };
};

}
#endif
