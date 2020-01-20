/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_LUX_CORE_RENDERER_OCL_H
#define RE_LUX_CORE_RENDERER_OCL_H

#include <QtCore>
#include "ReDefs.h"
#include "exporters/lux/ReLuxOCLRenderer.h"

/**
 * Class that defines the default parameters that we use when
 * rendering with the LuxCore engine, either PATH or BIDIR but unbiased
 */
class ReLuxCoreRenderer : public ReLuxOCLRenderer {
public:
  //! Constructor: ReLuxCoreRenderer
  ReLuxCoreRenderer()
  {
    // It does not matter...
    tracerType = PATH;
  };

  inline void setProperty( const QString& propName, QVariant propValue ) {

  }
  
  inline QString toString() {
    return QString(
                     "Renderer \"luxcore\"\n"
                     "  \"string config\" [\n"
                     "    \"renderengine.type = PATHOCL\"\n"
                     "    %1\n"
                     "    %2\n"
                     "  ]\n"
                   )
                   .arg(getEnabledDevicesString())
                   .arg(getWorkGroupSizeString());

  }
};

#endif
