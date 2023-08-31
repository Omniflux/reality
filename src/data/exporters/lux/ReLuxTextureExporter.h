/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_BASE_TEXTURE_EXPORTER_H
#define LUX_BASE_TEXTURE_EXPORTER_H

#include <QHash>
#include <QSharedPointer>

#include "reality_lib_export.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "ReTexture.h"
#include "ReTools.h"
#include "textures/ReConstant.h"


namespace Reality {

//! The texture cache
typedef QHash<QString, ReTexturePtr> ReTextureCache;

/**
  Base class for all texture exporters for LuxRender. It provides 
  services shared by all specialized classes. The export of a specific
  type of texture is provided by a specialized subclass.
 */
class REALITY_LIB_EXPORT ReLuxTextureExporter {

private:
  // Just a temp string to make returning values a bit faster
  static QString tmpStr;
  static ReTextureCache textureCache;
  static bool cachingEnabled;

public:

  virtual ~ReLuxTextureExporter() {

  }

  const QString getMappingAsString( const ReTexture3D* tex ) const {
    switch( tex->getMapping() ) {
      case ReTexture3D::UV:
        return "uv";
      case ReTexture3D::GLOBAL:
        return "global";
      case ReTexture3D::GLOBAL_NORMAL:
        return "global normal";
      case ReTexture3D::LOCAL:
        return "local";
      case ReTexture3D::LOCAL_NORMAL:
        return "localnormal";
    }
    return "uv";
  }

  const QString getMappingAsString( const ReTexture3DPtr tex ) const {
    return getMappingAsString(tex.data());
  }

  //! Returns a string that identifies the channel the texture belongs to
  inline QString findChannel( const ReTexturePtr tex ) {
    if (!tex.isNull()) {
      auto parent = tex->getParent();
      if (parent) {
        return parent->identifyChannel(tex);
      }
    }
    return "<none>";
  }

  //! When writing a float version of a color texture we need to generate 
  //! a unique name for the texture. The following method returns the
  //! *base name* of that unique name.
  static inline QString& getFloatTextureName( const ReTexturePtr tex ) {
    if (tex->getDataType() == ReTexture::numeric) {
      tmpStr = tex->getName();
    }
    else {
      tmpStr = QString("%1_float").arg(tex->getName());    
    }
    return tmpStr;
  }

  //! When writing a float version of a color texture we need to generate 
  //! a unique name for the texture. The following method returns the
  //! *full name* of that unique name. The geometry exporter needs to 
  //! figure out what is the full name of such texture after the texture itself
  //! has been written by the texture exporter. This is because in Lux the 
  //! displacement map, for example, is written in the geometry section and not
  //! in the material section. The following method is used to figure out
  //! the name without having to store and pass data between sections of the 
  //! program.
  static inline QString& getFloatTextureUniqueName( const ReTexturePtr tex ) {
    if (tex->getDataType() == ReTexture::numeric) {
      tmpStr = tex->getUniqueName();
    }
    else {
      tmpStr = QString("%1_float").arg(tex->getUniqueName());    
    }
    return tmpStr;
  }

  /**
   * Method: exportColor
   * Takes a color texture and returns the Lux parameter to export it as
   * a simple color. This is used by some textures to optimize their parameters
   * into simple colors instead of using a Constant texture
   */
  QString exportColor( const ReConstantPtr colorTex, const QString& label ) {
    QColor clr = colorTex->getColor();
    gammaCorrect(clr, RealitySceneData->getGamma());
    qreal r,g,b;
    clr.getRgbF(&r, &g, &b);
    return QString(
             "\"color %1\" [%2 %3 %4]"
           )
           .arg(label)
           .arg(r)
           .arg(g)
           .arg(b);
  }

  //! Creates a float version of a texture. The texture is assumed to
  //! be a color texture. This method only makes sense for image map 
  //! textures. If the texture is of another kind the method returns
  //! just the name of the original texture. 
  //! If the texture is of the correct kind the method writes the float
  //! version in the buffer passed and then returns the name of the texture
  //! so that the called can refer to it.  
  //! Parameters:
  //!   name - Input. The name used to label the exported texture. 
  //!   assignedName - Overrides the name used for the exported texture
  //!   isForPreview - A flag to communicate that this export is for the previewer
  //! \return The pointer to the newly created texture
  static ReTexturePtr writeFloatVersion( ReTexturePtr tex, QString& scene );
  
  static void initializeTextureCache();
  static void enableTextureCache( bool yesNo );
  static ReTexturePtr getTextureFromCache( ReTexturePtr tex );
  static ReTexturePtr getTextureFromCache( const QString& texGUID );
  static bool isTextureInCache( ReTexturePtr tex );
  //! Overloaded version to look up by texture name
  static bool isTextureInCache( const QString& key );

  //! Add one texture to the cache
  static void addTextureToCache( ReTexturePtr tex );

  virtual const QString exportTexture( ReTexturePtr tex, 
                                       const QString& assignedName = "",
                                       bool isForPreview = false ) = 0;

};

typedef QSharedPointer<ReLuxTextureExporter> ReLuxTextureExporterPtr;

} // namespace

#endif
