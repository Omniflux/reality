/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_LUX_LIGHT_EXPORTER_H
#define RE_LUX_LIGHT_EXPORTER_H

#include "reality_lib_export.h"
#include "exporters/ReBaseLightExporter.h"

namespace Reality {
  class ReLight;
  typedef QSharedPointer<ReLight> ReLightPtr;
}


namespace Reality {

/**
  Exports a light using the Lux format.
  Implemented as a singleton. To call it use:

    ReLuxLightExporter::getInstance()
 */
class REALITY_LIB_EXPORT ReLuxLightExporter : public ReBaseLightExporter {
private:
  static ReLuxLightExporter* instance;

  //! Given a light, this method writes the texture declaration for the 
  //! light's color model. If the color mode is RGB a constant texture
  //! will be written. If it's set to color temperature then the blackbody
  //! texture will be written. Lastly, if it's set to a preset then the
  //! lampspectrum texture will be written
  //! 
  //! Returns: 
  //!   A String with the name of the texture that has been written to the
  //!   buffer.
  QString writeColorModelTexture( ReLightPtr light, QString& str );

  //! This light differs from the others. A mesh light is a geometric
  //! mesh that emits light. The export method is meant to be called
  //! by the geometry export routine to provide the material/light
  //! attributes that are then going to be attached to the geometry.
  QString exportMeshLight( const ReLightPtr light );
  
  QString exportSpotLight( const ReLightPtr light );
  QString exportPointLight( const ReLightPtr light );
  QString exportSunlight( const ReLightPtr light );
  QString exportInfiniteLight( const ReLightPtr light );
  QString exportIBLLight( const ReLightPtr light );
  
public:

  // Constructor: ReLuxLightExporter
  ReLuxLightExporter() {

  };
  // Destructor: ReLuxLightExporter
  ~ReLuxLightExporter() {
    if (instance) {
      delete instance;
    }
  };

  QString exportStr;
  QString exportLight( const ReLightPtr light );

  static ReLuxLightExporter* getInstance();
};

} // namespace


#endif
