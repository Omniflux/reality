/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReLuxSceneExporter.h"

#include <QSettings>

#include "RealityBase.h"
#include "ReCamera.h"
#include "ReMatrix.h"
#include "ReSceneResources.h"
#include "ReTools.h"
#include "exporters/lux/ReBiasPathRenderOptions.h"
#include "exporters/lux/ReLightExporter.h"
#include "exporters/lux/ReLuxCoreRenderer.h"
#include "exporters/lux/ReLuxCPURenderer.h"
#include "exporters/lux/ReLuxMaterialExporterFactory.h"
#include "exporters/lux/ReLuxSIExporterFactory.h"
#include "exporters/lux/ReVolumeExporter.h"
#include "exporters/luxcore/ReLuxcoreMaterialExporterFactory.h"


#define MAT_DEBUG 0

namespace Reality {

QString ReLuxSceneExporter::getCamera() {
  ReCameraPtr cam = RealitySceneData->getSelectedCamera();
  if (cam.isNull()) {
    // Extreme case. Just failing gracefully.
    return "# There is no selected camera!\n"
           "LookAt 1.0 1.0 1.0 0 0 0 0 0 0 \n";
  }

  HostAppID hostApp = RealityBase::getRealityBase()->getHostAppID();
  ReVector position; // = cam->getPosition();
  ReVector target;   // = cam->getTarget();
  ReVector up;       // = cam->getUpVector();
  const ReMatrix* matrix = cam->getMatrix();

  matrix->getPosition(position);
  matrix->getTarget(target);
  matrix->getUpVector(up);
  vectorToLux(position);
  vectorToLux(target);
  vectorToLux(up);

  QString str = QString("# Camera: %1\n").arg(cam->getName());
  str += QString("LookAt %1 %2 %3 %4 %5 %6 %7 %8 %9\n")
                // Position
                .arg(convertUnit(position.X, hostApp))
                .arg(convertUnit(position.Y, hostApp))
                .arg(convertUnit(position.Z, hostApp))
                // Point At
                .arg(convertUnit(target.X, hostApp))
                .arg(convertUnit(target.Y, hostApp))
                .arg(convertUnit(target.Z, hostApp))
                // Tilt/pan
                .arg(convertUnit(up.X, hostApp))
                .arg(convertUnit(up.Y, hostApp))
                .arg(convertUnit(up.Z, hostApp));

  uint width = RealitySceneData->getWidth();
  uint height = RealitySceneData->getHeight();
  float fov = cam->getFOV(width, height);

  // From the Lux documentation:
  // If aspect_ratio > 1: screenwindow = -aspect_ratio aspect_ratio -1 1
  // Otherwise: screenwindow = -1 1 -1/aspect_ratio 1/aspect_ratio
  // The FOV is calculated by the ReCamera class to take in account the
  // aspect ratio.
  QString screenWindow;
  float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
  str += QString(
           "Camera \"perspective\" \"float fov\" [%1]"
           " \"float screenwindow\" [-%2 %2 -1 1]"
          )
         .arg(fov)
         // .arg(screenWindow)
         .arg(aspectRatio);

  // Studio doesn't use hither and yon
  if (hostApp == Poser) {
    str += QString(" \"float hither\" [%1] \"float yon\" [%2]")
             .arg(cam->getHither())
             .arg(cam->getYon());
  }
  
  float userFStop = cam->getDOFStop();
  if (cam->isDOFEnabled()) {
    float lensRadius = (cam->getFocalLength()/1000)/(2.0*userFStop);
    str += QString(" \"float lensradius\" [%1] \"float focaldistance\" [%2]")
           .arg(lensRadius)
           .arg(cam->getFocalDistance());
    
  }
  str += "\n";
  return str;
}

QString ReLuxSceneExporter::getFilm( uint frameNo ) {
  QString retVal = "Film \"fleximage\"\n";
  auto config = RealityBase::getConfiguration();
  // Obtain the selected camera
  ReCameraPtr cam = RealitySceneData->getSelectedCamera();

  retVal += QString("  \"integer outlierrejection_k\" [%1] \n")
            .arg(RealitySceneData->getOutlierRejection());

  float frameMultiplier = RealitySceneData->getFrameMultiplier();
  retVal += QString("  \"integer xresolution\" [%1]\n"
                    "  \"integer yresolution\" [%2]\n")
                    .arg(RealitySceneData->getWidth()*frameMultiplier)
                    .arg(RealitySceneData->getHeight()*frameMultiplier);

  quint32 maxSPX = RealitySceneData->getMaxSPX();
  if (maxSPX > 0) {
    retVal += QString("  \"integer haltspp\" [%1]\n")
              .arg(maxSPX);    
  }
  bool hasAlpha = RealitySceneData->hasAlphaChannel();
  retVal += QString("  \"bool premultiplyalpha\" [\"%1\"]\n")
            .arg( hasAlpha ? "true" : "false" );

  QFileInfo imageFileInfo(RealitySceneData->getImageFileName());
  QFileInfo sceneFileInfo(RealitySceneData->getSceneFileName());
  QString imageFileName;
  // If the scene and the image file are in the same path then refer to the image file
  // simply by name
  if (imageFileInfo.absolutePath() == sceneFileInfo.absolutePath()) {
    imageFileName = imageFileInfo.baseName();
  }
  else {
    // Otherwise build an absolute path from the scene file name to the image file name
    // We must write the file name without the extension
    imageFileName = QString("%1/%2")
                      .arg(imageFileInfo.absolutePath())
                      .arg(imageFileInfo.baseName());
  }

  retVal += QString("  \"string filename\" [\"%1\"]\n")
            // .arg( (frameNo > -1 ? expandFrameNumber(imageFileName,frameNo) : imageFileName) );
            .arg( expandFrameNumber(imageFileName,frameNo) );
  retVal += QString("  \"float gamma\" [%1]\n").arg(RealitySceneData->getGamma());
  retVal += QString("  \"integer displayinterval\" [%1]\n"  
                    "  \"integer writeinterval\" [%2]\n")
                    .arg(config->value(RE_CFG_LUX_DISPLAY_REFRESH).toInt())
                    .arg(config->value(RE_CFG_LUX_WRITE_INTERVAL).toInt());

  LuxToneMapper toneMapper;
  if (cam->isExposureEnabled()) {
    toneMapper = Linear;
  }
  else {
    toneMapper = Autolinear;
  }
  if ( toneMapper == Autolinear) {
    retVal += QString("  \"string tonemapkernel\" [\"autolinear\"]\n");
  }
  else if ( toneMapper == Linear) {
    retVal += QString(
      " \"string tonemapkernel\" [\"linear\"]\n"
      "   \"float linear_sensitivity\" [%1]\n"
      "   \"float linear_exposure\" [%2]\n"
      "   \"float linear_fstop\" [%3]\n"
      "   \"float linear_gamma\" [%4]\n"
    )
      .arg(cam->getISO())
      .arg(cam->getShutter())
      .arg(cam->getFStop())
      .arg(RealitySceneData->getGamma());
  };
  
  QString filmResponse = cam->getFilmResponse();
  if (filmResponse != "" && filmResponse != "NONE") {
    retVal += QString("  \"string cameraresponse\" [\"%1\"]\n").arg(filmResponse);
  };

  QString rgbChannels = hasAlpha ? "RGBA" : "RGB";
  bool usePNG, useTGA, useEXR;
  usePNG = useTGA = useEXR = false;
  switch (RealitySceneData->getImageFileFormat()) {
    case PNG:
      usePNG = true;
      break;
    case EXR:
      useEXR = true;
      break;
    case TGA:
      useTGA = true;
      break;      
  }
  retVal += QString(
              "  \"bool write_png\" [\"%1\"]\n"
              "  \"string write_png_channels\" [\"%2\"]\n"
              "  \"bool write_png_16bit\" [\"false\"]\n"
              "  \"bool write_png_gamutclamp\" [\"true\"]\n"
            )
            .arg(usePNG ? "true" : "false")
            .arg(rgbChannels);      

  retVal += QString(
              "  \"bool write_tga\" [\"%1\"]\n"
              "  \"string write_tga_channels\" [\"%2\"]\n"
              "  \"bool write_tga_gamutclamp\" [\"true\"]\n"
            )
            .arg(useTGA ? "true" : "false")
            .arg(rgbChannels);

  retVal += QString(
              " \"bool write_exr\" [\"%1\"]\n"
              "  \"string write_exr_channels\" [\"%2\"]\n"
              "  \"string write_exr_compressiontype\" [\"ZIP (lossless)\"]\n"
              "  \"bool write_exr_gamutclamp\" [\"true\"]\n"
            )
            .arg(useEXR ? "true" : "false")
            .arg(rgbChannels);

  retVal += QString(
                    "  \"bool debug\" [\"false\"]\n" 
                    "  \"bool write_resume_flm\" [\"true\"]\n"
                    "  \"bool restart_resume_flm\" [\"false\"]\n"   
                    "  \"bool write_flm_direct\" [\"true\"]\n"   
                    "  \"float colorspace_white\" [0.314275 0.329411]\n"
                    "  \"float colorspace_red\" [0.630 0.340]\n"
                    "  \"float colorspace_green\" [0.310 0.5950]\n"
                    "  \"float colorspace_blue\" [0.1550 0.0700]\n"
                    "  \"string ldr_clamp_method\" [\"%1\"]\n"                    
                   ).arg(RealitySceneData->getPreserveLightHue() ? "hue" : "lum");
  return(retVal);}

QString ReLuxSceneExporter::getPixelFilter() {
  if ( !RealitySceneData->isOCLRenderingON() ) {
    return "PixelFilter \"blackmanharris\" \"float xwidth\" [1.5] \"float ywidth\" [1.5]\n";
  }
  return "PixelFilter \"mitchell\" \"float xwidth\" [1.5] \"float ywidth\" [1.5]"
         "\"float B\" [0.3333] \"float C\" [0.3333] \"bool supersample\" [\"true\"]\n";
}

QString ReLuxSceneExporter::getAccelerator( const LuxAccelerator accelerator ) {
  QString str;
  switch( accelerator ) {
    case tabreckdtree:
      str =  "Accelerator \"tabreckdtree\" \"integer intersectcost\" [80] "
             "\"integer traversalcost\" [1] \"float emptybonus\" [0.2] "
             "\"integer maxprims\" [1] \"integer maxdepth\" [-1]\n";
      break;
    case qbvh:
      str = "Accelerator \"qbvh\" \"integer maxprimsperleaf\" [4] "
            "\"integer fullsweepthreshold\" [16] \"integer skipfactor\" [1]\n";
      break;
    case sqbvh:
      str = "Accelerator \"sqbvh\" \"integer maxprimsperleaf\" [4] "
            "\"integer fullsweepthreshold\" [16] \"integer skipfactor\" [1]\n";
      break;
  };

  return str;
}

QString ReLuxSceneExporter::getSampler() {
  QString samplerStr;
  switch( RealitySceneData->getSampler() ) {
    case sobol:
      samplerStr = "Sampler \"sobol\"";
      break;
    default:
      samplerStr = "Sampler \"metropolis\" \"float largemutationprob\" [0.40]"
                   " \"bool usecooldown\" [\"false\"]";
      ;
  }
  samplerStr += QString(" \"bool noiseaware\" [\"%1\"]\n")
                  .arg(RealitySceneData->isNoiseAwareOn() ? "true" : "false");
  return samplerStr;
}

QString ReLuxSceneExporter::getSurfaceIntegrator() {
  ReLuxSIExporterPtr exporter = ReLuxSIExporterFactory::getExporter(
    RealitySceneData->getSurfaceIntegrator()
  );
  return exporter->exportSI();
}

QString ReLuxSceneExporter::getVolumeIntegrator() {
  return "VolumeIntegrator \"multi\" \"float stepsize\" [1.0]\n";
}

// Renderers supported by Lux/Luxcore:
// - PATHCPU (Embree)
// - BIDIRCPU (Embree)
// - PATHOCL
// - BIASPATHCPU (seems to not work, being remapped to PATHOCL)
// - BIASPATHOCL 
// - LIGHTCPU (?)
// - BIDIRVMCPU (Embree?)
// - FILESAVER (?)
// - RPATHOCL (?)
// - RTBIASPATHOCL 
QString ReLuxSceneExporter::getRenderer() {
  ReSurfaceIntegratorType siType = RealitySceneData->getSurfaceIntegrator()->getType();  
  // CPU acceleration, our default
  ReLuxRendererConfigPtr renderCfg;
  // OpenCL
  if (RealitySceneData->isOCLRenderingON()) {
    if (RealitySceneData->getOCLBias() == 1) {
      auto bpCfg = ReBiasPathRenderOptionsPtr( new ReBiasPathRenderOptions() );
      bpCfg->setDeviceType(ReBiasPathRenderOptions::OpenCL);
      renderCfg = bpCfg;
    }
    else{
      renderCfg = QSharedPointer<ReLuxCoreRenderer>( new ReLuxCoreRenderer() ); 
    }
    renderCfg.dynamicCast<ReLuxOCLRenderer>()->setEnabledDevices(
      RealitySceneData->getOCLDeviceFlags(),RealitySceneData->getNumOCLDevices()
    );
  }
  else {
    if ( RealitySceneData->cpuAccelerationEnabled() ) {
      if (RealitySceneData->getCPUBias()) {
        auto bpCfg = ReBiasPathRenderOptionsPtr( new ReBiasPathRenderOptions() );
        bpCfg->setDeviceType(ReBiasPathRenderOptions::CPU);
        renderCfg = bpCfg;
      }
      else {
        renderCfg = QSharedPointer<ReLuxCPURenderer>( new ReLuxCPURenderer() );
        renderCfg->setProperty("useAcceleration", true);
      }
    }
    else {
      renderCfg = QSharedPointer<ReLuxCPURenderer>( new ReLuxCPURenderer() );
      renderCfg->setProperty("useAcceleration", false );
    }
  }
  if (siType == SI_BIDIR) {
    renderCfg->setTracerType(ReLuxRendererConfig::BIDIR);
  }
  else if (siType == SI_PATH) {
    renderCfg->setTracerType(ReLuxRendererConfig::PATH);
  }

  return renderCfg->toString() + "\n";
}

QString ReLuxSceneExporter::getLights() {
  ReLuxLightExporter* lightExporter = ReLuxLightExporter::getInstance();
  ReLightIterator i(RealitySceneData->getLights());

  QString str;
  while(i.hasNext()) {
    i.next();    
    // Meshlights are handled by the geometry exporter, we need to
    // skip them here.
    ReLightPtr light = i.value();
    if (light->getType() == MeshLight) {
      continue;
    }
    if (light->isLightOn()) {
      str += lightExporter->exportLight(light);      
    }
  }
  return str;
}

QString ReLuxSceneExporter::getMatrixString( const ReMatrix& matrix, 
                                             const HostAppID appID,
                                             const bool forInstance ) 
{
  ReMatrix tx1(
    1, 0,  0, 0,
    0, 0, -1, 0,
    0, 1,  0, 0,
    0, 0,  0, 1
  );

  ReMatrix tx2(
    1,  0,  0, 0,
    0,  0,  1, 0,
    0, -1,  0, 0,
    0,  0,  0, 1
  );

  ReMatrix m2;
  if (forInstance) {
    m2 = tx1 * matrix * tx2;
  }
  else {
    m2 = matrix * tx2;
  }

  return QString("Transform [\n" 
                 "%01  %02  %03  %04\n" 
                 "%05  %06  %07  %08\n"  
                 "%09  %10  %11  %12\n"  
                 "%13  %14  %15  %16\n]\n")
          // X
          .arg(m2.m[0][0],6,'f')
          .arg(m2.m[0][1],6,'f')
          .arg(m2.m[0][2],6,'f')
          .arg(0.0,6,'f')
          // Y
          .arg(m2.m[1][0],6,'f')
          .arg(m2.m[1][1],6,'f')
          .arg(m2.m[1][2],6,'f')
          .arg(0.0,6,'f')
          // Z
          .arg(m2.m[2][0],6,'f')
          .arg(m2.m[2][1],6,'f')
          .arg(m2.m[2][2],6,'f')
          .arg(0.0,6,'f')  
          // Position
          .arg(convertUnit(m2.m[3][0], appID),6,'f')
          .arg(convertUnit(m2.m[3][1], appID),6,'f')
          .arg(convertUnit(m2.m[3][2], appID),6,'f')
          .arg(1.0,6,'f');
}

/*
 * Export Scene
 */
void ReLuxSceneExporter::exportScene( const int frameNo, boost::any& sceneData ) {
  QString sceneFileName = expandFrameNumber(RealitySceneData->getSceneFileName(), frameNo);
  ReSceneResources* sceneResources = ReSceneResources::getInstance();
  sceneResources->setSceneName(sceneFileName);
  sceneResources->initialize();

  QString sceneStr = QString(
    "#\n# LuxRender Scene created by Reality by Pret-a-3D\n#\n"
  );

  sceneStr += getCamera()            %
              "\n"                   %
              getFilm(frameNo)       %
              "\n"                   %
              getRenderer()          %
              "\n"                   %
              getPixelFilter()       %
              getAccelerator(qbvh)   %
              getSampler()           %
              getSurfaceIntegrator() %
              getVolumeIntegrator();

  // Get the list of all the objects in the scene and iterate
  // through them exporting all the materials
  QString materials;
  QString textures;
  ReGeometryObjectDictionary objs = scene->getObjects();
  ReGeometryObjectIterator i(objs);
  while( i.hasNext() ) {
    i.next();
    ReGeometryObjectPtr obj = i.value();

    if (!obj->isVisible()) {
      continue;
    }

    ReMaterialIterator mi(obj->getMaterials());

    while( mi.hasNext() ) {
      mi.next();

      ReMaterialPtr mat = mi.value();
      if (mat.isNull()) {
        RE_LOG_DEBUG() << "Error: one material for " << QSS(obj->getName()) << " is null";
        continue;        
      }
      
      if (!mat->isVisibleInRender()) {
        continue;
      }
      
      ReMaterialExporterPtr matExporter;
      switch(RealitySceneData->getRenderer()) {
        case LuxRender: {
          matExporter = ReLuxMaterialExporterFactory::getExporter(mat.data());
          break;
        }
        case SLG: {
          matExporter = ReLuxcoreMaterialExporterFactory::getExporter(mat.data());
          break;
        }
        default: {
          // Unknown format. Just in case
          sceneStr = QString("-> \"%1\" is an unknown renderer")
                      .arg(RealitySceneData->getRenderer());
          return;
        }
      }
      boost::any textureData;
      matExporter->exportTextures(mat.data(), textureData);
      textures  += boost::any_cast<QString>(textureData);
      boost::any exportedMats;
      matExporter->exportMaterial(mat.data(), exportedMats);
      QString mats;
      try {
        mats = boost::any_cast<QString>(exportedMats);
      }
      catch(...) {
        RE_LOG_WARN() << "Error: cast operation invalid for material " 
                      << mat->getName().toStdString();
      }
      materials += mats + "\n";
    }

    // Export all the material lights
    ReMaterialIterator li(*obj->getLights());
    while( li.hasNext() ) {
      li.next();

      ReLightMaterialPtr mat = li.value().staticCast<ReLightMaterial>();
      ReLuxMaterialExporterPtr matExporter = ReLuxMaterialExporterFactory::getExporter(mat.data());
      boost::any textureData;
      matExporter->exportTextures(mat.data(), textureData);
      textures  += boost::any_cast<QString>(textureData);

      boost::any exportedMats;
      matExporter->exportMaterial(mat.data(), exportedMats);
      materials += boost::any_cast<QString>(exportedMats) + "\n";
    }
  }
  // Export the volumes
  QString volumes;
  ReLuxVolumeExporter volExporter;

  ReVolumeIterator vi(RealitySceneData->getVolumes());
  while( vi.hasNext() ) {
    vi.next();
    volumes += volExporter.exportVolume(vi.value());
  }

  sceneStr += "WorldBegin\n";
  sceneStr += getLights();  
  sceneStr += QString("#\n"
                      "# Textures\n"
                      "#\n"
                      "%1\n\n"
                      "#\n"
                      "# Volumes\n"
                      "#\n"
                      "%2\n"
                      "#\n"
                      "# Materials\n"
                      "MakeNamedMaterial \"RealityNull\" \"string type\" [\"null\"]\n"
                      "#\n"
                      "%3\n")
              .arg(textures)
              .arg(volumes)
              .arg(materials);

  // Generate the name for the geometry file to be included
  QFileInfo sceneFile(sceneFileName);

  sceneStr += QString("\nInclude \"%1.%2\"\n\n")
             .arg(sceneFile.baseName()).arg(LUX_INCLUDE_EXTENSION);
  sceneStr += "WorldEnd\n";
  sceneResources->reset();

  sceneData = sceneStr;
}



}