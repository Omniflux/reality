/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_MIX_TEXTURE_EXPORTER_H
#define LUX_MIX_TEXTURE_EXPORTER_H

//#include "ReTexture.h"
#include "textures/ReMix.h"
#include "ReTools.h"
#include "ReSceneData.h"
#include "ReLuxTextureExporter.h"
#include "reality_lib_export.h"

namespace Reality {

/**
 * This class exports a ReMix texture to the file format for LuxRender
 */
class REALITY_LIB_EXPORT ReLuxMixExporter : public ReLuxTextureExporter {

private:
  const QString exportColorTexture( ReMixTexturePtr tex,
                                    const QString& assignedName ) {
    ReTexturePtr mixTexture = ReLuxTextureExporter::getTextureFromCache(
                                tex->getMixTexture()
                              );
                  
    ReTexturePtr tex1 = ReLuxTextureExporter::getTextureFromCache(
                          tex->getTexture1()
                        );
    ReTexturePtr tex2 = ReLuxTextureExporter::getTextureFromCache( 
                          tex->getTexture2()
                        );

    // Optimize the output of color textures as color parameters
    QString tex1Str, tex2Str;
    if (tex1->getType() == TexConstant && 
        tex1->getDataType() == ReTexture::color) {
      tex1Str = exportColor(tex1.staticCast<ReConstant>(), "tex1");
    }
    else {
      tex1Str = QString("\"texture tex1\" [\"%1\"]").arg(tex1->getUniqueName());
    }
    if (tex2->getType() == TexConstant && 
        tex1->getDataType() == ReTexture::color) {
      tex2Str = exportColor(tex2.staticCast<ReConstant>(), "tex2");
    }
    else {
      tex2Str = QString("\"texture tex2\" [\"%1\"]").arg(tex2->getUniqueName());
    }

    QString str;

    str += QString("Texture \"%1\" \"color\" \"mix\" %2 %3 ")
             .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
             .arg(tex1Str)
             .arg(tex2Str);

    if (mixTexture.isNull()) {
      str.append(QString("\"float amount\" [%1] ").arg(tex->getMixAmount()));
    }
    else {
      ReTexturePtr mixer;
      if ( mixTexture->getDataType() != ReTexture::numeric ) {
        mixer = ReLuxTextureExporter::writeFloatVersion( mixTexture, str );
      }
      else {
        mixer = mixTexture;
      }
      str.append( QString("\"texture amount\" [\"%1\"] ")
                    .arg(mixer->getUniqueName()) );
    }

    str += "\n";
    return str;
  }

  const QString exportFloatTexture( ReMixTexturePtr tex, 
                                    const QString& assignedName ) {
    
    ReTexturePtr mixTexture = ReLuxTextureExporter::getTextureFromCache(
                                tex->getMixTexture()
                              );
    ReTexturePtr tex1 = ReLuxTextureExporter::getTextureFromCache(
                          tex->getTexture1()
                        );
    ReTexturePtr tex2 = ReLuxTextureExporter::getTextureFromCache(
                          tex->getTexture2()
                        );

    // If the texture uses constant float numbers then simply
    // output them
    QString tex1Str, tex2Str;
    if (tex->useFloatValues()) {
      tex1Str = QString("\"float tex1\" [%1]").arg(tex->getFloat1());
      tex2Str = QString("\"float tex2\" [%1]").arg(tex->getFloat2());
    }
    else {
      tex1Str = QString("\"texture tex1\" [\"%1\"]").arg(tex1->getUniqueName());
      tex2Str = QString("\"texture tex2\" [\"%1\"]").arg(tex2->getUniqueName());
    }

    QString str = QString(
                    "Texture \"%1\" \"float\" \"mix\" %2 %3 "
                  )
                  .arg(assignedName == "" ? tex->getUniqueName() : assignedName)
                  .arg(tex1Str)
                  .arg(tex2Str);

    if (mixTexture.isNull()) {
      str.append(QString("\"float amount\" [%1] ").arg(tex->getMixAmount()));
    }
    else {
      ReTexturePtr mixer;
      if ( mixTexture->getDataType() != ReTexture::numeric ) {
        mixer = ReLuxTextureExporter::writeFloatVersion( mixTexture, str );
      }
      else {
        mixer = mixTexture;
      }
      str.append(
                 QString("\"texture amount\" [\"%1\"] ")
                  .arg(mixer->getUniqueName())
                );
    }
    str += "\n";
    return str;  
  }

public:

  const QString exportTexture( ReTexturePtr texbase, 
                               const QString& assignedName = "", 
                               bool isForPreview = false ) 
  {
    ReMixTexturePtr tex = texbase.staticCast<ReMixTexture>();
    QString str;
    QString chStr = findChannel(texbase);
    if (!chStr.isEmpty()) {
      str = QString("# From channel %1\n").arg(chStr);
    }

    if (tex->getDataType() == ReTexture::color) {
      str += exportColorTexture( tex, assignedName );
    }
    else {
      str += exportFloatTexture( tex, assignedName );
    }
    return str;
  };
};

}
#endif
