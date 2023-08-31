/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_QT_MATERIAL_IMPORTER_H
#define RE_QT_MATERIAL_IMPORTER_H

#include <QSharedPointer>

#include "reality_lib_export.h"
#include "ReAcsel.h"
#include "ReAlphaChannelMaterial.h"
#include "ReDisplaceableMaterial.h"
#include "ReMaterial.h"
#include "ReTextureCreator.h"


namespace Reality {

/**
 * Base class for all material importers.
 */
class REALITY_LIB_EXPORT ReQtMaterialImporter {

public:
  //! Constructor: ReQtMaterialImporter
  ReQtMaterialImporter() {
  };

  //! Destructor: ReQtMaterialImporter
  virtual ~ReQtMaterialImporter() {
  };

  enum ReplaceTextureMode {
    //! Forces the replacement of all textures. If a channel doesn't
    //! have a texture then the channel will be emptied.
    Replace,
    //! Keeps the existing textures.
    Keep,
    //! Replaces only the textures that are present, like when using a generic
    //! preset. If a channel is not set then it will not be emptied.
    //! This is used when applying a generic shader. In thst case some
    //! textures might be present, like the specular color, if the 
    //! channel uses a solid color. When a generic shader is saved only
    //! Constant textures are saved and only if they are linked directly
    //! to a channel at top level. 
    //! A channel value could be saved as empty and so
    //! it's important, when applying the generic shader that that channel
    //! is not emptied, otherwise it might replace the values coming from
    //! the converted host shader. For example, a generic shader will never
    //! have a bump map, because bump maps are not solid colors. If the
    //! converted shader from the host has a bump map and we use the standard
    //! texture replacement then the bump map channel will be emptied. That
    //! is not what needs to be done. In that case we need to apply the
    //! properties of the generic shader but leave the bump map, or other
    //! textures alone.
    ShallowReplace
  };

  /**
   * \param mat Pointer to the material to update
   * \param data The shader data that contains the texture definition used
   *             to update the textures of the material mat
   * \param texMode The mode of replacing the texture. It can be one of 
   *                Replace, Keep, ShallowReplace
   */
  void importTextures( ReMaterialPtr mat, 
                       const QVariantMap& data, 
                       const ReplaceTextureMode texMode ) 
  {
    // Textures
    auto textures = data.value("textures").toList();
    int numTextures = textures.count();

    // If there are no textures available then we must exit now.
    // Textures are removed from default/generic shaders, with the
    // exception of ReConstant textures and only if they are linked
    // directly by a channel. For that reason we might end up with a
    // a shader that has no textures but that is used to create a
    // material using the "replace textures" option. When this method
    // is called the caller has no idea if there are textures to
    // be replaced or nor so it's the responsibilith of this method
    // to check what needs to be done.
    if ( !numTextures ) {
      return;
    }

    if (texMode == Replace) {
      // First remove all the previous textures, if any.
      mat->removeAllTextures();

      for (int i = 0; i < numTextures; i++) {
        mat->addTextureToCatalog(
          ReTexturePtr(
            TextureCreator::createTexture(textures[i].toMap(), mat.data())
          )
        );
      }

      // Channels
      QMapIterator<QString, QVariant> ci(data.value("channels").toMap());
      while( ci.hasNext() ) {
        ci.next();
        QString texName = ci.value().toString();
        mat->setChannel(ci.key(), texName);
      }
    }
    else  if ( texMode == ShallowReplace ) {
      // Create a map of the texture array keyed by texture name for
      // quick lookup
      QHash<QString, int> textureIndex;
      for (int i = 0; i < numTextures; i++) {
        QVariantMap tex = textures[i].toMap();
        textureIndex[tex.value("name").toString()] = i;
      }

      // Channels
      QMapIterator<QString, QVariant> ci(data.value("channels").toMap());
      // Replace the channels if, and only if, the channe from the shader
      // data is not empty
      while( ci.hasNext() ) {
        ci.next();
        QString texName = ci.value().toString();
        if (texName.isEmpty()) {
          continue;
        }
        // Now replace the texture. 
        // First we check if the texture pointed by the channel is in the
        // catalog of textures that was saved with the shader.
        if (!textureIndex.contains(texName)) {
          continue;
        }
        // Then we test if the index in in range, just to make sure
        int index = textureIndex.value(texName);
        if (textures.count() < index) {
          continue;
        }
        // Fetch the texture data
        auto tex = textures[index].toMap();
        // Does this texture already exist in the material?
        auto channelName = ci.key();
        auto channelTex = mat->getChannel(channelName);
        if ( 
             channelTex.isNull() ||
             (!channelTex.isNull() && channelTex->getType() == TexConstant) ||
              data["type"] == "Glass"
            )
        {
          mat->deleteTexture(texName);
          mat->addTextureToCatalog(
            ReTexturePtr(
              TextureCreator::createTexture(tex, mat.data())
            )
          );

          mat->setChannel(channelName, texName);
        }
      }
    }
    else if (texMode == Keep) {
      // Just to be extra sure
      return;
    }

  }

  void importBasicInformation( ReMaterialPtr mat, const QVariantMap& data ) {
    mat->setName( data.value("name").toString() );
    bool isEdited = data.value("edited").toBool();
    if (isEdited) {
      mat->setEdited();
    }
    mat->setVisibleInRender(data.value("visibleInRender").toBool());
    mat->setInnerVolume(data.value("innerVolume").toString());
    mat->setOuterVolume(data.value("outerVolume").toString());
    QString acselSetID = data.value("acselSetID","").toString();
    mat->setAcselSetID(acselSetID);
    QString acselID = data.value("acselID","").toString();
    mat->setAcselID(acselID);
    QVariantMap shaderSetInfo;
    auto acsel = ReAcsel::getInstance();
    // It's possible that the ACSEL database has changed since a scene
    // was saved. In this section we verify that the data saved and the 
    // ACSEL database are synchronized.
    // If a shader does not exist in the database then we check if a 
    // generic shader is preset. If it is then we assign the ID of that
    // shader to the material. Otherwise we blank the the shader set ID.
    if (acsel->findShader(acselID, shaderSetInfo)) {
      if ( acsel->findShaderSet(acselSetID, shaderSetInfo) ) {
        if (shaderSetInfo[RE_ACSEL_BUNDLE_IS_DEFAULT].toBool()) {
          mat->setAcselSetName(RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL);
        }
        else {
          mat->setAcselSetName(shaderSetInfo[RE_ACSEL_BUNDLE_NAME].toString());
        }
      }
    }
    else {
      QString defaultAcselID = mat->computeAcselID(true);
      shaderSetInfo.clear();
      if (acsel->findShader( defaultAcselID, shaderSetInfo ) ) {
        mat->setAcselSetID(shaderSetInfo["SetID"].toString());
        mat->setAcselSetName(RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL);
      }
      else {
        mat->setAcselSetID("");
      }
    }
    importTextures(mat, data, Replace);
  }
  
  void importAlphaChannel( ReAlphaChannelMaterialPtr mat, const QVariantMap& data ) {
    mat->setAlphaStrength( data.value("alphaStrength").toFloat() );
    QString alphaMapName = data.value("alphaMap").toString();
    if (alphaMapName != "") {
      auto am = mat->getTexture( alphaMapName );
      if (!am.isNull()) {
        mat->setAlphaMap( am );
      }
    }
  }

  void importModifiers( ReDisplaceableMaterialPtr mat, const QVariantMap& data ) {
    mat->setNormalMapFlag( data.value("isNormalMap").toBool() );
    mat->setBmNegative( data.value("bmNegative").toFloat() );
    mat->setBmPositive( data.value("bmPositive").toFloat() );
    mat->setBmStrength( data.value("bmStrength").toFloat() );
    mat->setDmNegative( data.value("dmNegative").toFloat() );
    mat->setDmPositive( data.value("dmPositive").toFloat() );
    mat->setDmStrength( data.value("dmStrength").toFloat() );
    mat->setSubdivisions( data.value("subdivision").toFloat() );
    mat->setUseMicrofacets( data.value("useMicrofacets").toBool() );
    mat->setIsSmooth( data.value("smoothness").toBool() );
    mat->setKeepSharpEdges( data.value("keepSharpEdges").toBool() );
    mat->setEmitsLight( data.value("emitsLight", false).toBool() );
    mat->setLightGain( data.value("lightGain", 0.0).toFloat() );
    mat->setEmitsLight( data.value("emitsLight").toBool() );
  }

  virtual void importMaterial( ReMaterialPtr mat, const QVariantMap& data ) = 0;
  virtual void importFromClipboard( ReMaterialPtr mat, 
                                    const QVariantMap& data, 
                                    const ReplaceTextureMode texMode = Keep ) = 0;
};


} // namespace

#endif
