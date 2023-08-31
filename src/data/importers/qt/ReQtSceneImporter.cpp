/**
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReQtSceneImporter.h"

#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"


namespace Reality {

void ReQtSceneImporter::restoreSceneData( const QVariantMap& data, 
                                          const int mainVersion, 
                                          const int subVersion ) 
{
  RealitySceneData->setFrameSize( data.value(KEY_SCENE_WIDTH).toInt(), 
                                  data.value(KEY_SCENE_HEIGHT).toInt() );
  RealitySceneData->setAlphaChannel( data.value(KEY_SCENE_ALPHA_CHANNEL).toBool() );
  RealitySceneData->setFrameMultiplier( data.value(KEY_SCENE_FRAME_MULTIPLIER).toFloat() );
  RealitySceneData->setImageFileName( data.value(KEY_SCENE_IMAGE_NAME).toString() );
  // 0 is the PNG format
  RealitySceneData->setImageFileFormat( 
    static_cast<ExportImageFileFormat>(data.value(KEY_SCENE_IMAGE_FILE_FORMAT,0).toInt())
  );
  RealitySceneData->setSceneFileName( data.value(KEY_SCENE_SCENE_NAME).toString() );
  RealitySceneData->setRenderer(
    static_cast<ReRenderers>(
      data.value(KEY_SCENE_RENDERER, ReRenderers::LuxRender).toInt()
    )
  );
  RealitySceneData->setCpuAcceleration(data.value(KEY_SCENE_CPU_ACCEL, true).toBool());
  RealitySceneData->setCPUBias(static_cast<RE_UINT>(data.value(KEY_SCENE_CPU_BIAS, 0).toInt()));
  RealitySceneData->setOCLBias(static_cast<RE_UINT>(data.value(KEY_SCENE_OCL_BIAS, 0).toInt()));
  // Compatibility with deprecated turbo mode
  RealitySceneData->setOCLRendering(data.value(KEY_SCENE_TURBO_MODE, false).toBool());
  // New way of setting OpenCL rendering
  RealitySceneData->setOCLRendering(data.value(KEY_SCENE_USE_OCL, false).toBool());
  RealitySceneData->setLuxLogLevel(
    static_cast<LUX_LOG_LEVEL>(data.value(KEY_SCENE_LOG_LEVEL, LUX_WARNINGS).toInt())
  );
  RealitySceneData->setSampler(static_cast<RenderSampler>(
                                 data.value(KEY_SCENE_RENDER_SAMPLER, sobol).toInt()
                               ));

  RealitySceneData->setTextureCollection(
    data.value(KEY_SCENE_TEXTURE_COLLECTION, false).toBool()
  );
  RealitySceneData->setTextureSize(
    static_cast<ReTextureSize>(
      data.value(KEY_SCENE_TEXTURE_SIZE, T_ORIGINAL).toInt()
    )
  );
  // If we are loading a scene saved with Reality 3.0 then the geometry
  // file format was always set to LuxNative. We convert it automatically
  // to the PLY binary.
  GeometryFileFormat geomFormat = static_cast<GeometryFileFormat>(data.value(KEY_SCENE_GEOMETRY_FORMAT).toInt());
  if ( mainVersion == 3 && subVersion == 0 && geomFormat == LuxNative ) {
    geomFormat = BinaryPLY;
  }
  RealitySceneData->setGeometryFormat(geomFormat);
  RealitySceneData->setOutlierRejection( data.value(KEY_SCENE_OUTLIER_REJECTION).toInt() );
  RealitySceneData->setMaxSPX( data.value(KEY_SCENE_MAX_SPX).toInt() );
  RealitySceneData->setGamma( data.value(KEY_SCENE_GAMMA).toFloat() );
  RealitySceneData->setPreserveLightHue( data.value(KEY_SCENE_PRESERVE_HUE).toBool() );
  RealitySceneData->setEnableDisplacement( data.value(KEY_SCENE_ENABLE_DISPLACEMENT).toBool() );
  RealitySceneData->setOverwriteWarning( data.value(KEY_SCENE_OVERWRITE_WARNING).toBool() );
  RealitySceneData->setOCLGroupSize( data.value(KEY_SCENE_OCL_GROUP_SIZE).toInt() );
  RealitySceneData->setNumOCLDevices( data.value(KEY_SCENE_OCL_NUM_DEVICES, 0).toInt());
  RealitySceneData->setOCLDeviceFlags( static_cast<quint16>(data.value(KEY_SCENE_OCL_DEVICES_ENABLED, 0).toInt()) );
  // RealitySceneData->setOCLDevicesEnabled( static_cast<unsigned char>(data.value(KEY_SCENE_OCL_DEVICES_ENABLED).toInt()) );
  RealitySceneData->setRenderAsStatue( data.value(KEY_SCENE_RENDER_AS_STATUE, false).toBool() );

  // Restoring the Surface Integrator
  QVariantMap si = data.value(KEY_SCENE_SURFACE_INTEGRATOR).toMap();
  ReSurfaceIntegratorType siType = static_cast<ReSurfaceIntegratorType>(si.value("type").toInt());
  RealitySceneData->setSurfaceIntegrator(siType);
  RealitySceneData->getSurfaceIntegrator()->setValues(si);
  RealitySceneData->setNoiseAware(data.value(KEY_SCENE_NOISE_AWARE_SAMPLER, false).toBool());
}

} // namespace
