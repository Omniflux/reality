/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_LUXCORE_RENDERER_CONFIG_H
#define RE_LUXCORE_RENDERER_CONFIG_H

#include <QSharedPointer>


/**
 * Interface that defines how the renderer configuration is selected when
 * exporting a scene to a specific renderer. In particular, this is designed
 * to provide all the different parameteres for all the different renderers 
 * included in Lux. Classes implementing this interface are expected to be
 * defined for the standard CPU rendering of Lux, for the CPU-accelerated
 * renderer (Embree), for the OpenCL and BiasPathOCL renderers. 
 * 
 * Each class is expected to define its own defaults and to provide a
 * string that will be injected in the scene definition to activate the features
 * provided by the renderer. 
 * 
 * The setProperty() method is used to overwrite the default properties with
 * custom values.
 */
class ReLuxRendererConfig {

public:
  enum TracerType {
    BIDIR,
    PATH
  };

protected:
  TracerType tracerType;

public:
  //! Constructor: ReLuxCoreRendererConfig
  ReLuxRendererConfig() {
  };

  //! Destructor: ReLuxRendererConfig
  virtual ~ReLuxRendererConfig() {
  };

  inline void setTracerType( const TracerType t ) {
    tracerType = t;
  }

  virtual void setProperty( const QString& propName, QVariant propValue ) = 0;

  virtual QString toString() = 0;
};

typedef QSharedPointer<ReLuxRendererConfig> ReLuxRendererConfigPtr;

#endif
