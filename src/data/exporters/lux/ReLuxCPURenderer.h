/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_LUX_CPU_RENDERER_H
#define RE_LUX_CPU_RENDERER_H

#include <QVariant>

#include "exporters/ReLuxRendererConfig.h"


/**
 * Class that defines the default parameters that we use when
 * rendering with classic of accelerated (Embree) CPU rendere.
 */
class ReLuxCPURenderer : public ReLuxRendererConfig {

private:
  bool useAcceleration;

public:
  //! Constructor: ReLuxCPURenderer
  ReLuxCPURenderer() :
    useAcceleration(true)
  {
    tracerType = BIDIR;
  };

  inline void setProperty( const QString& propName, QVariant propValue ) {
    if (propName == "useAcceleration") {
      useAcceleration = propValue.toBool();
    }
  }
  
  inline QString toString() {
    QString str = QString("Renderer ");
    if (useAcceleration) {
      str += "\"luxcore\" ";
      QString tracer = tracerType == BIDIR ? "BIDIRCPU" : "PATHCPU";
      str += QString("\"string config\" [\n"
                     "  \"renderengine.type = %1\"\n"
                     "  \"accelerator.type = EMBREE\"\n"
                     "]").arg(tracer);
    }
    else {
      str += "\"sampler\"";
    }
    str += "\n";
    return str;
  }
};


#endif
