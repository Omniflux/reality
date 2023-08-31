/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_BIAS_PATH_H
#define RE_BIAS_PATH_H

#include <QSharedPointer>

#include "ReRenderContext.h" 
#include "exporters/lux/ReLuxOCLRenderer.h"


//! The amount of radiance clamping that we need when there is the sun in 
//! the scene
#define RE_BIASEDPATH_CLAMPING_FOR_SUN          21000.0
//! The amount of radiance clamping that we need when there is an IBL light in 
//! the scene
#define RE_BIASEDPATH_CLAMPING_FOR_IBL            400.0
//! The amount of radiance clamping that we need for each mesh light.
#define RE_BIASEDPATH_CLAMPING_PER_MESH_LIGHT      10.0
//! The amount of radiance clamping that we need for each spot light.
#define RE_BIASEDPATH_CLAMPING_PER_SPOT_LIGHT      50.0
//! The amount of minimum radiance clamping that we need for each infinite light.
#define RE_BIASEDPATH_CLAMPING_PER_INFINITE_LIGHT 100.0

//! The amount of radiance clamping that we start with, before adding the contribution
//! of each light
#define RE_BIASEDPATH_CLAMPING_DEFAULT 10

using namespace Reality;
/**
 * Class that defines the default parameters that we use when
 * rendering with BIASPATH_OCL
 */

class ReBiasPathRenderOptions : public ReLuxOCLRenderer {
public:
  enum DeviceType {
    CPU,
    OpenCL
  };

private:
  RE_UINT tile_size;
  // Sampling
  RE_UINT sampling_aa_size;
  RE_UINT sampling_diffuse_size;
  RE_UINT sampling_glossy_size;
  RE_UINT sampling_specular_size;
  RE_UINT sampling_direct_light_size;
  // Ray depth
  RE_UINT pathdepth_total;
  RE_UINT pathdepth_diffuse;
  RE_UINT pathdepth_glossy;
  RE_UINT pathdepth_specular;
  float clamping_radiance_maxvalue; 
  bool enable_multipass; 
  float convergence_threshold; 
  float convergence_threshold_reduction; 

  //! Flag that determines if we use BIASPATHCPU or BIASPATHOCL
  DeviceType device_type;
public:


  //! Constructor: ReBiasPathRenderOptions
  ReBiasPathRenderOptions() :
    tile_size(32),
    sampling_aa_size(5),
    sampling_diffuse_size(2),
    sampling_glossy_size(2),
    sampling_specular_size(2),
    sampling_direct_light_size(1),
    pathdepth_total(10),
    pathdepth_diffuse(4),
    pathdepth_glossy(5),
    pathdepth_specular(4),
    clamping_radiance_maxvalue(RE_BIASEDPATH_CLAMPING_DEFAULT),
    enable_multipass(true),
    convergence_threshold(0.04),
    convergence_threshold_reduction(0.9),
    device_type(CPU)
  {
    auto renderCtx = ReRenderContext::getInstance();
    if (renderCtx->hasSun()) {
      clamping_radiance_maxvalue = RE_BIASEDPATH_CLAMPING_FOR_SUN;
    }
    else if (renderCtx->hasIBL()) {
      clamping_radiance_maxvalue = RE_BIASEDPATH_CLAMPING_FOR_IBL;
    }
    clamping_radiance_maxvalue += renderCtx->getMeshLightCount() * 
                                  RE_BIASEDPATH_CLAMPING_PER_MESH_LIGHT;
    clamping_radiance_maxvalue += renderCtx->getSpotLightCount() * 
                                  RE_BIASEDPATH_CLAMPING_PER_SPOT_LIGHT;
    clamping_radiance_maxvalue += renderCtx->getInfiniteLightCount() * 
                                  RE_BIASEDPATH_CLAMPING_PER_INFINITE_LIGHT;
  };

  inline void setDeviceType( const DeviceType devType ) {
    device_type = devType;
  }

  inline void setProperty( const QString& propName, QVariant propValue ) {
    if (propName == "tile_size") {
      tile_size = propValue.toInt();
    }
    else if (propName == "sampling_aa_size") {
      sampling_aa_size = propValue.toInt();
    }
    else if (propName == "sampling_diffuse_size") {
      sampling_diffuse_size = propValue.toInt();
    }
    else if (propName == "sampling_glossy_size") {
      sampling_specular_size = propValue.toInt();
    }
    else if (propName == "sampling_specular_size") {
      sampling_glossy_size = propValue.toInt();
    }
    else if (propName == "sampling_direct_light_size") {
      sampling_direct_light_size = propValue.toInt();
    }
    else if (propName == "pathdepth_total") {
      pathdepth_total = propValue.toInt();
    }
    else if (propName == "pathdepth_diffuse") {
      pathdepth_diffuse = propValue.toInt();
    }
    else if (propName == "pathdepth_glossy") {
      pathdepth_glossy = propValue.toInt();
    }
    else if (propName == "pathdepth_specular") {
      pathdepth_specular = propValue.toInt();
    }
    else if (propName == "clamping_radiance_maxvalue") {
      clamping_radiance_maxvalue = propValue.toFloat();
    }
    else if (propName == "convergence_threshold") {
      convergence_threshold = propValue.toFloat();
    }
    else if (propName == "convergence_threshold_reduction") {
      convergence_threshold_reduction = propValue.toFloat();
    }
    else if (propName == "enable_multipass") {
      enable_multipass = propValue.toBool();
    }    
  }
  
  inline QString toString() {
    QString device = device_type == CPU ? "CPU" : "OCL";

    QString postFix;
    if (device_type == OpenCL) {
      postFix = QString("    %1\n"
                        "    %2\n")
                  .arg(getEnabledDevicesString())
                  .arg(getWorkGroupSizeString());
    }
    return QString(
                    "Renderer \"luxcore\"\n"
                    "  \"string config\" [\n"
                    "    \"renderengine.type = BIASPATH%1\"\n" 
                    "    \"tile.size = %2\"\n"
                    "    \"tile.multipass.enable = %3\"\n"
                    "    \"tile.multipass.convergencetest.threshold = %4\"\n"
                    "    \"tile.multipass.convergencetest.threshold.reduction = %5\"\n"

                    "    \"biaspath.sampling.aa.size = %6\"\n"
                    "    \"biaspath.sampling.diffuse.size = %7\"\n"
                    "    \"biaspath.sampling.glossy.size = %8\"\n"
                    "    \"biaspath.sampling.specular.size = %9\"\n"
                    "    \"biaspath.sampling.directlight.size = %10\"\n"

                    "    \"biaspath.pathdepth.total = %11\"\n"
                    "    \"biaspath.pathdepth.diffuse = %12\"\n"
                    "    \"biaspath.pathdepth.glossy = %13\"\n"
                    "    \"biaspath.pathdepth.specular = %14\"\n"

                    "    \"biaspath.clamping.radiance.maxvalue = %15\"\n"
                    "    %16\n"
                    "  ]\n"
                  )
                  .arg(device)
                  .arg(tile_size)
                  .arg(enable_multipass ? 1 : 0)
                  .arg(convergence_threshold)
                  .arg(convergence_threshold_reduction)
                  .arg(sampling_aa_size)
                  .arg(sampling_diffuse_size)
                  .arg(sampling_glossy_size)
                  .arg(sampling_specular_size)
                  .arg(sampling_direct_light_size)
                  .arg(pathdepth_total)
                  .arg(pathdepth_diffuse)
                  .arg(pathdepth_glossy)
                  .arg(pathdepth_specular)
                  .arg(clamping_radiance_maxvalue)
                  .arg(postFix);
  }
};

typedef QSharedPointer<ReBiasPathRenderOptions> ReBiasPathRenderOptionsPtr;

#endif
