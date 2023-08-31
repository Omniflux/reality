/**
  File: ReSceneData.cpp

  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReSceneData.h"

#include <QDir>
#include <QFileInfo>
#include <QSetIterator>
#include <QSettings>
#include <QVariantMap>
#include <QJson/Parser>
#include <QJson/Serializer>

#include "ReIPC.h"
#include "ReOpenCL.h"
#include "ReLuxGeometryExporter.h"
#include "ReLuxRunner.h"
#include "ReRenderContext.h"
#include "exporters/ReLuxSceneExporter.h"
#include "exporters/ReJSONSceneExporter.h"
#include "exporters/ReQtSceneExporter.h"
#include "exporters/ReSLGSceneExporter.h"
#include "exporters/lux/ReLuxMaterialExporterFactory.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "importers/qt/ReQtCameraImporter.h"
#include "importers/qt/ReQtGeometryObjectImporter.h"
#include "importers/qt/ReQtLightImporter.h"
#include "importers/qt/ReQtSceneImporter.h"
#include "importers/qt/ReVolumeImporter.h"


using namespace Reality;

extern "C" {
  /**
   * The global variable used to access the data
   */
  REALITY_LIB_EXPORT ReSceneData* RealitySceneData;
}

using namespace Reality;

ReBaseSceneExporter* ReSceneData::getLuxRenderSceneExporter(void) {
  return new ReLuxSceneExporter(RealitySceneData);
}

ReBaseSceneExporter* ReSceneData::getJSONSceneExporter(void) {
  return new ReJSONSceneExporter(RealitySceneData);
}

ReBaseSceneExporter* ReSceneData::getSLGSceneExporter(void) {
  return new ReSLGSceneExporter(RealitySceneData);
}

ReBaseSceneExporter* ReSceneData::getQtSceneExporter(void) {
  return new ReQtSceneExporter(RealitySceneData);
}

//! Constructor
ReSceneData::ReSceneData() {
  initScene();
  destroying = false;
  sceneExporterFactory.registerExporter("lux", ReSceneData::getLuxRenderSceneExporter);
  sceneExporterFactory.registerExporter("map", ReSceneData::getQtSceneExporter);
  sceneExporterFactory.registerExporter("slg", ReSceneData::getSLGSceneExporter);
  sceneExporterFactory.registerExporter("json", ReSceneData::getJSONSceneExporter);
}

//! Destructor
ReSceneData::~ReSceneData() {
  destroying = true;
  RE_LOG_INFO() << "ReSceneData closed";
}


void ReSceneData::initScene() {
  ReConfigurationPtr config = RealityBase::getConfiguration();

  needsSaving     = false;
  inGUIMode       = false;
  displayInterval = config->value(RE_CFG_LUX_DISPLAY_REFRESH).toInt();
  writeInterval   = config->value(RE_CFG_LUX_WRITE_INTERVAL).toInt();
  properties.sceneWidth      = 1280;
  properties.sceneHeight     = 720;
  properties.imageFileFormat = PNG;
  properties.renderer        = LuxRender;
  properties.alphaChannel    = false;
  properties.frameMultiplier = 1.0;
  properties.geometryFormat     = BinaryPLY;
  properties.gamma              = 2.2f;
  properties.outlierRejection   = 1;
  properties.maxSPX             = 0;
  properties.luxLogLevel        = LUX_WARNINGS;
  properties.preserveLightHue   = false;
  properties.enableDisplacement = true;
  properties.numThreads         = getNumCPUs();
  properties.cpuAcceleration    = true;
  properties.OCLGroupSize       = config->value(RE_CFG_OCL_GROUP_SIZE).toInt();
  properties.OCLDeviceFlags     = 0;
  properties.OCLBias            = 0;
  properties.CPUBias            = 0;
  properties.numOCLDevices      = 0;
  properties.overwriteWarning   = config->value(RE_CFG_OVERWRITE_WARNING).toBool();
  properties.surfaceIntegrator  = ReSurfaceIntegratorPtr(new ReSIPath());
  properties.textureCollection = false;
  properties.textureSize       = T_ORIGINAL;
  properties.useOpenCL         = false;
  properties.renderAsStatue    = false;
  properties.sampler           = sobol;
  properties.noiseAwareSampler = false;

  setDefaultSceneName();
  ReVolumePtr world = ReVolumePtr(new ReVolume(RE_AIR_VOLUME));
  world->setEditable(true);
  world->setIOR(1.0);
  world->setColor(QColor(255,255,255));
  saveVolume(world);

  ReLightPtr IBL = ReLightPtr(new ReLight(RE_IBL_LIGHT_ID, RE_IBL_LIGHT_ID, IBLLight));
  IBL->turnOnOff(false);
  saveLight(RE_IBL_LIGHT_ID, IBL);

  // Check if there is a GPU and if so if it's nVidia. In that case we need
  // to set the group size at 64 or higher.
  auto ocl = ReOpenCL::getInstance();
  if (ocl->isEnabled()){
    foreach( QString pl, ocl->getPlatformList() ) {
      if ((pl.indexOf("nVidia") != -1) && 
          (properties.OCLGroupSize < RE_CFG_NVIDIA_OCL_GROUP_SIZE)) 
      {
        properties.OCLGroupSize = RE_CFG_NVIDIA_OCL_GROUP_SIZE;
      }
    }
  }
}

void ReSceneData::setGUIMode( const bool onOff ) {
  inGUIMode = onOff;
}

bool ReSceneData::isInGUIMode() {
  return inGUIMode;
}


void ReSceneData::setDefaultSceneName() {
  QString sceneLocation = RealityBase::getConfiguration()->value(RE_CFG_DEFAULT_SCENE_LOCATION).toString();
  setSceneFileName(QString("%1/%2")
                     .arg(sceneLocation)
                     .arg(RE_CFG_DEFAULT_SCENE_NAME));

  setImageFileName(QString("%1/%2")
                     .arg(sceneLocation)
                     .arg(RE_CFG_DEFAULT_IMAGE_NAME));
}
  
void ReSceneData::setSceneFileName( const QString newName ) {
  properties.outputSceneFileName = newName;
};

QString ReSceneData::getSceneFileName() {
  return properties.outputSceneFileName;
};


void ReSceneData::setImageFileName( const QString imageName ) {  
  properties.renderedImageFileName = imageName;
};

QString ReSceneData::getImageFileName() {
  return properties.renderedImageFileName;
};


void ReSceneData::setFrameSize( const unsigned int width, const unsigned int height){
  properties.sceneWidth = width;
  properties.sceneHeight = height;

  if (!isInGUIMode()) {
    realityIPC->frameSizeChanged(width, height);
  }
};

quint16 ReSceneData::getWidth() const { 
  return properties.sceneWidth; 
};

quint16 ReSceneData::getHeight() const { 
  return properties.sceneHeight; 
};

bool ReSceneData::hasAlphaChannel() const { 
  return properties.alphaChannel; 
};

void ReSceneData::setAlphaChannel( bool onOff ) {
  properties.alphaChannel = onOff;
}

void ReSceneData::setNeedsSaving( const bool newVal ) {
  needsSaving = newVal;
};

bool ReSceneData::doesNeedsSaving() const {
  return needsSaving;
};

void ReSceneData::setRenderAsStatue( const bool onOff ) {
  properties.renderAsStatue = onOff;
};

bool ReSceneData::isRenderAsStatueEnabled() const {
  return properties.renderAsStatue;
};

void ReSceneData::setFrameMultiplier( const float val ) {
  properties.frameMultiplier = val;
};

float ReSceneData::getFrameMultiplier() const {
  return properties.frameMultiplier;
};

float ReSceneData::getGamma() const {
  return properties.gamma;
};
/*
 Method: setGamma
 */
void ReSceneData::setGamma( float newVal ) {
  properties.gamma = newVal;
};

/*
 Method: getMaxSPX
 */
quint32 ReSceneData::getMaxSPX() const {
  return properties.maxSPX;
};
/*
 Method: setMaxSPX
 */
void ReSceneData::setMaxSPX( quint32 newVal ) {
  properties.maxSPX = newVal;
};

bool ReSceneData::isOCLRenderingON() const {
  return properties.useOpenCL;
}

void ReSceneData::setOCLRendering( const bool isOn ) {
  properties.useOpenCL = isOn;
}

/**
 Method: hasTextureCollection
 */
bool ReSceneData::hasTextureCollection() const {
  return properties.textureCollection;
};
/**
 Method: setTextureCollection
 */
void ReSceneData::setTextureCollection( const bool isOn ) {
  properties.textureCollection = isOn;
};

ReTextureSize ReSceneData::getTextureSize() const {
  return properties.textureSize;
}

void ReSceneData::setTextureSize( const ReTextureSize newSize ) {
  properties.textureSize = newSize;
}

/*
 Method: getDisplayInterval
 */
quint16 ReSceneData::getDisplayInterval() const {
  return displayInterval;
};
/*
 Method: setDisplayInterval
 */
void ReSceneData::setDisplayInterval( quint16 newVal ) {
  displayInterval = newVal;
};
  
/*
 Method: getWriteInterval
 */
quint16 ReSceneData::getWriteInterval() const {
  return writeInterval;
};
/*
 Method: setWriteInterval
 */
void ReSceneData::setWriteInterval( quint16 newVal ) {
  writeInterval = newVal;
};

/*
 Method: getOutlierRejection
 */
quint16 ReSceneData::getOutlierRejection() const {
  return properties.outlierRejection;
};
/*
 Method: setOutlierRejection
 */
void ReSceneData::setOutlierRejection( quint16 newVal ) {
  properties.outlierRejection = newVal;
};


/*
 Method: getImageFileFormat
 */
ExportImageFileFormat ReSceneData::getImageFileFormat() const {
  return properties.imageFileFormat;
};
/*
 Method: setImageFileFormat
 */
void ReSceneData::setImageFileFormat( ExportImageFileFormat newVal ) {
  properties.imageFileFormat = newVal;
};

/*
 Method: getPreserveLightHue
 */
bool ReSceneData::getPreserveLightHue() const {
  return properties.preserveLightHue;
};
/*
 Method: setPreserveLightHue
 */
void ReSceneData::setPreserveLightHue( bool newVal ) {
  properties.preserveLightHue = newVal;
};

//! Returns the selected renderer
ReRenderers ReSceneData::getRenderer() const {
  return properties.renderer;
};
/**
 * Set the renderer used to render the scene.
 */
void ReSceneData::setRenderer( const ReRenderers newVal ) {
  properties.renderer = newVal;
};

void ReSceneData::setNumThreads( const int newVal ) {
  properties.numThreads = newVal;
};

int ReSceneData::getNumThreads() const {
  return properties.numThreads;
};

void ReSceneData::setLuxLogLevel( LUX_LOG_LEVEL logLevel ) {
  properties.luxLogLevel = logLevel;
};

LUX_LOG_LEVEL ReSceneData::getLuxLogLevel() {
  return properties.luxLogLevel;
};

bool ReSceneData::getOverwriteWarning() const {
  return properties.overwriteWarning;
};
void ReSceneData::setOverwriteWarning( bool newVal ) {
  properties.overwriteWarning = newVal;
};

const ReSurfaceIntegratorPtr ReSceneData::getSurfaceIntegrator() {
  return properties.surfaceIntegrator;
}

int ReSceneData::getOCLGroupSize() const {
  return properties.OCLGroupSize;
};

void ReSceneData::setOCLGroupSize( int newVal ) {
  properties.OCLGroupSize = newVal;
};

unsigned char ReSceneData::getOCLDeviceFlags() const {
  return properties.OCLDeviceFlags;
};

void ReSceneData::setOCLDeviceFlags( quint16 flags ) {
  properties.OCLDeviceFlags = flags;
};

void ReSceneData::setOCLDeviceFlag( quint16 deviceNo, const bool onOff ) {
  if (onOff) {
    properties.OCLDeviceFlags |= 1U << deviceNo;
  }
  else {
    properties.OCLDeviceFlags &= ~(1U << deviceNo);
  }
};

void ReSceneData::setSurfaceIntegrator( const ReSurfaceIntegratorType si ) {
  if (si != properties.surfaceIntegrator->getType()) {
    properties.surfaceIntegrator.clear();
    switch(si) {
      case SI_BIDIR:
        properties.surfaceIntegrator = ReSurfaceIntegratorPtr( new ReSIBidir() );
        break;
      case SI_PATH:
        properties.surfaceIntegrator = ReSurfaceIntegratorPtr( new ReSIPath() );
        break;
      case SI_SPPM:
        properties.surfaceIntegrator = ReSurfaceIntegratorPtr( new ReSISPPM() );
        break;
      case SI_EXPM:
        properties.surfaceIntegrator = ReSurfaceIntegratorPtr( new ReSIExPhotonMap() );
        break;
      case SI_DIRECT:
        properties.surfaceIntegrator = ReSurfaceIntegratorPtr( new ReSIDirect() );
        break;
      default:
        properties.surfaceIntegrator = ReSurfaceIntegratorPtr( new ReSIBidir() );
    }
  }
}


void ReSceneData::getOutputData( QVariantMap& data ) {
  data.insert(KEY_SCENE_WIDTH, properties.sceneWidth);
  data.insert(KEY_SCENE_HEIGHT, properties.sceneHeight);
  data.insert(KEY_SCENE_SCENE_NAME, toStandardFilePath(getSceneFileName()));
  data.insert(KEY_SCENE_IMAGE_NAME, toStandardFilePath(getImageFileName()));
  data.insert(KEY_SCENE_IMAGE_FILE_FORMAT, getImageFileFormat());
  data.insert(KEY_SCENE_RENDERER, getRenderer());
  data.insert(KEY_SCENE_ALPHA_CHANNEL, hasAlphaChannel());
  data.insert(KEY_SCENE_FRAME_MULTIPLIER, getFrameMultiplier());
  data.insert(KEY_SCENE_GEOMETRY_FORMAT, getGeometryFormat());
  data.insert(KEY_SCENE_OUTLIER_REJECTION, getOutlierRejection());
  data.insert(KEY_SCENE_RENDER_SAMPLER, getSampler());
  data.insert(KEY_SCENE_NOISE_AWARE_SAMPLER, isNoiseAwareOn());
  data.insert(KEY_SCENE_MAX_SPX, (int)getMaxSPX());
  data.insert(KEY_SCENE_GAMMA, getGamma());
  data.insert(KEY_SCENE_PRESERVE_HUE, getPreserveLightHue());
  data.insert(KEY_SCENE_ENABLE_DISPLACEMENT, isDisplacementEnabled());
  // data.insert(KEY_SCENE_GPU_SUPPORT, gpuEnabled);
  data.insert(KEY_SCENE_NUM_THREADS, getNumThreads());
  data.insert(KEY_SCENE_OVERWRITE_WARNING, getOverwriteWarning());
  data.insert(KEY_SCENE_RENDER_AS_STATUE, isRenderAsStatueEnabled());
  data.insert(KEY_SCENE_OCL_GROUP_SIZE, getOCLGroupSize());
  data.insert(KEY_SCENE_OCL_DEVICES_ENABLED, getOCLDeviceFlags());
  data.insert(KEY_SCENE_OCL_NUM_DEVICES, getNumOCLDevices());
  data.insert(KEY_SCENE_OCL_BIAS, getOCLBias());
  data.insert(KEY_SCENE_CPU_BIAS, getCPUBias());
  data.insert(KEY_SCENE_USE_OCL, isOCLRenderingON());
  data.insert(KEY_SCENE_TEXTURE_COLLECTION, hasTextureCollection());
  data.insert(KEY_SCENE_TEXTURE_SIZE, getTextureSize());
  // data.insert(KEY_SCENE_SELECTED_CAMERA, selectedCamera);
  data.insert(KEY_SCENE_CPU_ACCEL, cpuAccelerationEnabled());
  data.insert(KEY_SCENE_LOG_LEVEL, getLuxLogLevel());

  QVariantMap volumes;
  // Volume linkage
  ReVolumeLinkageIterator vli(properties.volumeMaterialLinks);
  // Scan all the volumes
  while(vli.hasNext()) {
    vli.next();
    QVariantMap objects;
    // Scan all the objects linked to the volume
    ReObjectMaterialMapIterator omi(vli.value());    
    while(omi.hasNext()) {
      omi.next();
      // Scan all the materials of this object
      QSetIterator<QString> si(omi.value());
      QVariantList mats;
      while(si.hasNext()) {
        mats << si.next();        
      }
      objects[omi.key()] = mats;
    }
    volumes[vli.key()] = objects;
  }
  data[KEY_SCENE_VOLUME_LINKAGE] = volumes;
  // Add the Surface Integrator data
  QVariantMap si = properties.surfaceIntegrator->getValues();
  si["type"] = properties.surfaceIntegrator->getType();
  data[KEY_SCENE_SURFACE_INTEGRATOR] = si;
}

QString ReSceneData::getOutputData() {
  QVariantMap data;
  getOutputData(data);
  QJson::Serializer json;
  return json.serialize(data);
}

// Method: setOutputData
//   Sets the whole output data from a JSON object
void ReSceneData::setOutputData( QString jsonData ) {
  QJson::Parser parser;
  bool ok;

  QVariantMap data = parser.parse(jsonData.toUtf8(), &ok).toMap();
  if (!ok) {
    RE_LOG_DEBUG() << "Error in ReSceneData::getOutputData(), JSON data incorrect";
    RE_LOG_DEBUG() << QSS(jsonData);
    return;
  }
  // selectedCamera        = data.value(KEY_SCENE_SELECTED_CAMERA).toString();
  properties.sceneWidth            = data.value(KEY_SCENE_WIDTH).toInt();
  properties.sceneHeight           = data.value(KEY_SCENE_HEIGHT).toInt();
  properties.alphaChannel          = data.value(KEY_SCENE_ALPHA_CHANNEL).toBool();
  properties.frameMultiplier       = data.value(KEY_SCENE_FRAME_MULTIPLIER).toDouble();
  properties.renderedImageFileName = data.value(KEY_SCENE_IMAGE_NAME).toString();
  properties.imageFileFormat = static_cast<ExportImageFileFormat>(
                                 data.value(KEY_SCENE_IMAGE_FILE_FORMAT).toInt()
                               );
  properties.outputSceneFileName = data.value(KEY_SCENE_SCENE_NAME).toString();
  properties.renderer = static_cast<ReRenderers>(
                            data.value(KEY_SCENE_RENDERER, 
                                       ReRenderers::LuxRender).toInt()
                        );
  properties.geometryFormat   = static_cast<GeometryFileFormat>(data[KEY_SCENE_GEOMETRY_FORMAT].toInt());
  properties.outlierRejection = data.value(KEY_SCENE_OUTLIER_REJECTION).toInt();
  properties.maxSPX           = data.value(KEY_SCENE_MAX_SPX).toInt();
  properties.gamma            = data.value(KEY_SCENE_GAMMA).toDouble();
  properties.sampler = static_cast<RenderSampler>(
                            data.value(
                              KEY_SCENE_RENDER_SAMPLER, 
                              static_cast<int>(metropolis)
                            ).toInt()
                          );
  properties.noiseAwareSampler = data.value(KEY_SCENE_NOISE_AWARE_SAMPLER).toBool();

  bool ras = data.value(KEY_SCENE_RENDER_AS_STATUE, false).toBool();
  // When we flip-flop between normal render and render-as-statue we
  // need to clear the cache of material converters in case previous
  // renders filled the cache.
  if (ras != properties.renderAsStatue) {
    properties.renderAsStatue = ras;
    ReLuxMaterialExporterFactory::clearExporterCache();    
  }
  properties.preserveLightHue = data.value(KEY_SCENE_PRESERVE_HUE).toBool();
  // gpuEnabled            = data.value(KEY_SCENE_GPU_SUPPORT).toBool();
  properties.cpuAcceleration       = data.value(KEY_SCENE_CPU_ACCEL, false).toBool();
  // Compatibility with old, deprecated turbo mode
  if (data.contains(KEY_SCENE_TURBO_MODE)) {
    properties.useOpenCL = data.value(KEY_SCENE_TURBO_MODE).toBool();
  }
  properties.useOpenCL             = data.value(KEY_SCENE_USE_OCL, false).toBool();
  properties.luxLogLevel           = static_cast<LUX_LOG_LEVEL>(data.value(KEY_SCENE_LOG_LEVEL, LUX_WARNINGS).toInt());
  properties.numThreads            = data.value(KEY_SCENE_NUM_THREADS).toInt();
  properties.enableDisplacement    = data.value(KEY_SCENE_ENABLE_DISPLACEMENT).toBool();
  properties.overwriteWarning      = data.value(KEY_SCENE_OVERWRITE_WARNING).toBool();
  properties.OCLGroupSize          = data.value(KEY_SCENE_OCL_GROUP_SIZE).toInt();
  properties.OCLDeviceFlags        = static_cast<unsigned char>(
                                       data.value(KEY_SCENE_OCL_DEVICES_ENABLED, 
                                                  OCL_DEVICE_1_SELECTED).toInt()
                                     );
  properties.CPUBias               = data.value(KEY_SCENE_CPU_BIAS, 0).toInt();
  properties.OCLBias               = data.value(KEY_SCENE_OCL_BIAS, 0).toInt();
  properties.numOCLDevices         = data.value(KEY_SCENE_OCL_NUM_DEVICES, 0).toInt();
  properties.textureCollection     = data.value(KEY_SCENE_TEXTURE_COLLECTION, false).toBool();
  properties.textureSize           = static_cast<ReTextureSize>(
                                       data.value(KEY_SCENE_TEXTURE_SIZE, 
                                                  T_ORIGINAL).toInt()
                                     );
  QVariantMap si = data[KEY_SCENE_SURFACE_INTEGRATOR].toMap();

  auto siType = static_cast<ReSurfaceIntegratorType>(si.value("type").toInt());
  setSurfaceIntegrator(siType);
  properties.surfaceIntegrator->setValues(si);

  // Volume linkage
  properties.volumeMaterialLinks.clear();
  QVariantMap volumeLinkage = data[KEY_SCENE_VOLUME_LINKAGE].toMap();
  QMapIterator<QString, QVariant> vli(volumeLinkage);
  while( vli.hasNext() ) {
    vli.next();
    QMapIterator<QString, QVariant> omi(vli.value().toMap());
    while( omi.hasNext() ) {
      omi.next();
      QVariantList mats = omi.value().toList(); 
      int numMats = mats.count();
      for (int i = 0; i < numMats; i++) {
        linkVolumeToMaterial(vli.key(), omi.key(), mats[i].toString());
      }
    }
  }

}

bool ReSceneData::addObject( const QString objName, 
                             const QString internalName, 
                             const QString geometryName  ) 
{
  if (!objects.contains(internalName)) {
    objects[internalName] = ReGeometryObjectPtr(
      new ReGeometryObject(objName,internalName, geometryName)
    );
    return true;
  }
  return false;
}

bool ReSceneData::addObject( ReGeometryObject* objPtr ) {
  QString internalName = objPtr->getInternalName();
  if (!objects.contains(internalName)) {
    objects[internalName] = ReGeometryObjectPtr(objPtr);
    return true;
  }
  return false;
}

void ReSceneData::renameObject( const QString objectID, const QString newName ) {
  if (objects.contains(objectID)) {
    objects[objectID]->rename(newName);
    if (!isInGUIMode()) {
      realityIPC->objectRenamed(objectID, newName);      
    }
  }
  else {
    RE_LOG_DEBUG() << "Error: could not find object " << QSS(objectID);
  }
}

void ReSceneData::renameObjectID( const QString objectID, const QString newID ) {
  if (objects.contains(objectID)) {
    ReGeometryObjectPtr obj = objects.value(objectID);
    objects[newID] = obj;
    obj->setInternalName(newID);
    objects.remove(objectID);
    if (!isInGUIMode()) {
      realityIPC->objectIDRenamed(objectID, newID);
    }
  }
  else {
    RE_LOG_DEBUG() << "Error: could not find object " << QSS(objectID);
  }
}

void ReSceneData::deleteObject( const QString objectID, const bool usePublicName ) {
  if (!usePublicName) {
    if (objects.contains(objectID)) {
      objects.remove(objectID);
      // Notify the UI
      if (!isInGUIMode()) {
        realityIPC->objectDeleted(objectID);
      }
    }
  }
  else {
    ReGeometryObjectIterator i(objects);
    while( i.hasNext() ) {
      i.next();
      auto obj = i.value();
      if (obj->getName() == objectID) {
        deleteObject(obj->getInternalName(), false);
        break;
      }
    }
  }
}

void ReSceneData::setObjectVisible( const QString objectID, 
                                    const bool isVisible ) 
{
  if (objects.contains(objectID)) {
    objects[objectID]->setVisible(isVisible);
  }
  // Notify the UI of the change
  if (!isInGUIMode()) {
    realityIPC->objectVisibilityChanged(objectID, isVisible);
  }
}

void ReSceneData::addMaterial( const QString objName,
                               const QString matGUID,
                               const QString materialData) {
  ReGeometryObjectPtr obj = objects.value(objName);
  if (!obj) {
    RE_LOG_WARN() << "Null object! " << objName;
    return;
  }

  obj->addMaterial(matGUID,materialData);
}

void ReSceneData::addMaterial( const QString objName,
                               const QString matGUID,
                               const QVariantMap& materialData) {
  ReGeometryObjectPtr obj = objects.value(objName);
  if (!obj) {
    RE_LOG_WARN() << "Null object! " << objName;
    return;
  }

  obj->addMaterial(matGUID, materialData);
}

void ReSceneData::updateMaterial( const QString objName,
                                  const QString matGUID,
                                  const QString materialData) {
  if (objects.contains(objName)) {
    if (objects[objName]->hasMaterial(matGUID)) {
      objects[objName]->updateMaterial(matGUID,materialData);
      //! Let the UI know about the change
      if (!isInGUIMode()) {
        realityIPC->materialUpdated(matGUID, objName);
      }    
    }
    else {
      // This case happens when the user createa a new material either
      // by using the Group Tool in Poser or the Geometry Editor in 
      // Studio.
      addMaterial(objName, matGUID, materialData);
      if (!isInGUIMode()) {
        // To force the refresh in the GUI we pretend to have
        // removed and re-added the object.
        realityIPC->objectDeleted(objName);
        realityIPC->objectAdded(objName);
      }    
    }
  }
}

void ReSceneData::updateMaterial( const QString objName,
                                  const QString matGUID,
                                  const QVariantMap& materialData) {
  if (objects.contains(objName)) {
    if (objects[objName]->hasMaterial(matGUID)) {
      objects[objName]->updateMaterial(matGUID, materialData);
      //! Let the UI know about the change
      if (!isInGUIMode()) {
        realityIPC->materialUpdated(matGUID, objName);
      }    
    }
    else {
      // If a material is not present, because it was added after the 
      // initial scan of an object, we add it at this point. This allows
      // for operations like editing of poly groups in Poser or Studio
      addMaterial(objName, matGUID, materialData);
      if (!isInGUIMode()) {
        // To force the refresh in the GUI we pretend to have
        // removed and re-added the object.
        realityIPC->objectDeleted(objName);
        realityIPC->objectAdded(objName);
      }    
    }
  }
}

int ReSceneData::getNumObjectMaterials( const QString objectID ) {
  if (objects.contains(objectID)) {
    return objects.value(objectID)->getNumMaterials();
  }
  return 0;
}

int ReSceneData::getNumMaterials() {
  int numMats = 0;
  ReGeometryObjectIterator i(objects);
  while(i.hasNext()) {
    i.next();
    numMats += i.value()->getNumMaterials();
  }
  return numMats;
}

ReMaterialPtr ReSceneData::getMaterial( const QString& objectName, const QString& materialName ) {
  if (objects.contains(objectName)) {
    return objects[objectName]->getMaterial(materialName);
  }
  return ReMaterialPtr();
}


void ReSceneData::restoreMaterial( const QString objName,
                                   const QString matID,
                                   const QString materialData) {
  if (objects.contains(objName)) {
    QJson::Parser parser;
    bool ok;

    QVariantMap data = parser.parse(materialData.toUtf8(), &ok).toMap();
    if (!ok) {
      RE_LOG_DEBUG() << "Error in ReSceneData::restoreMaterial(), JSON data incorrect";
      return;
    }    
    objects[objName]->restoreMaterial(matID,data);  
  }
}

void ReSceneData::markObjectsForDeletion() {
  objectsToDelete.clear();
  objectsToDelete << objects.keys();
};

bool ReSceneData::clearObjectDeletionFlag( const QString objectID ) {
  bool result = objectsToDelete.contains(objectID);
  objectsToDelete.removeOne(objectID);
  return result;
}


void ReSceneData::deleteOrphanedObjects() {
  QStringListIterator i(objectsToDelete);
  while( i.hasNext() ) {
    QString objectID = i.next();
    deleteObject(objectID);
    // Notifies the GUI of the event
    if (!isInGUIMode()) {
      realityIPC->objectDeleted(objectID);
    }
  }
};


void ReSceneData::restoreLight( const QString& dataStr ) {
  QJson::Parser parser;
  bool ok;

  QVariantMap data = parser.parse(dataStr.toUtf8(), &ok).toMap();
  if (!ok) {
    RE_LOG_DEBUG() << "Error in ReSceneData::restoreLight(), JSON data incorrect";
    return;
  }
  ReQtLightImporter importer;
  ReLightPtr light = importer.importLight( data );
  saveLight(light->getID(), light);
}

void ReSceneData::restoreCamera( const QString& dataStr ) {
  QJson::Parser parser;
  bool ok;

  QVariantMap data = parser.parse(dataStr.toUtf8(), &ok).toMap();
  if (!ok) {
    RE_LOG_DEBUG() << "Error in ReSceneData::restoreLight(), JSON data incorrect";
    return;
  }
  ReQtCameraImporter importer;
  addCamera(importer.importCamera( data ));
}


QStringList ReSceneData::getMaterialNames( const QString& objectID ) {
  if (objects.contains(objectID)) {
    return objects[objectID]->getMaterialNames();
  }
  return QStringList();
} 


void ReSceneData::changeMaterialType( const QString objectID, 
                                      const QString materialID, 
                                      const ReMaterialType newType,
                                      const QString jsonData ) 
{
  objects[objectID]->changeMaterialType(materialID, jsonData, newType);
  // If we are not in GUI mode it means that this function is running
  // in the host-app side of Reality. So we need to alert the GUI that
  // the request conversion has been done.
  if (!isInGUIMode()) {
    realityIPC->materialTypeChanged(objectID, materialID, newType);
  }
}

void ReSceneData::changeMaterialType( const QString objectID, 
                                      const QString materialID, 
                                      const ReMaterialType newType,
                                      const QVariantMap& matData ) 
{
  objects[objectID]->changeMaterialType(materialID, matData, newType);
  // If we are not in GUI mode it means that this function is running
  // in the host-app side of Reality. So we need to alert the GUI that
  // the request conversion has been done.
  if (!isInGUIMode()) {
    realityIPC->materialTypeChanged(objectID, materialID, newType);
  }
}


void ReSceneData::addCamera( const QVariantMap camData ) {
  QString camID = camData["ID"].toString();
  cameras[camID] = ReCameraPtr(
    new ReCamera(camData["name"].toString(), 
                 camID, 
                 camData)
  );
  if (selectedCamera == "") {
    selectedCamera = camID;
  }
  // Notify the UI of the change
  if (!isInGUIMode()) {
    realityIPC->cameraAdded(camID);
  }
}

void ReSceneData::addCamera( const ReCameraPtr cam ) {
  cameras[cam->getID()] = cam;
  if (selectedCamera == "") {
    selectedCamera = cam->getID();
  }
  // Notify the UI of the change
  if (!isInGUIMode()) {
    realityIPC->cameraAdded(cam->getID());
  }
}


void ReSceneData::renameCamera( const QString cameraID, const QString newName ) {
  if (cameras.contains(cameraID)) {
    cameras[cameraID]->setName(newName);
    if (!isInGUIMode()) {
      realityIPC->cameraRenamed(cameraID, newName);
    }
  }
  else {
    RE_LOG_DEBUG() << "Error: could not find camera " << QSS(cameraID);
  }
}


void ReSceneData::updateCamera( const QString& cameraID, 
                                const QVariantMap& data ) 
{
  if (cameras.contains(cameraID)) {
    cameras[cameraID]->setValues(data);
    selectedCamera = cameraID;
    // Notify the UI
    if (!isInGUIMode()) {
      realityIPC->cameraDataChanged(cameraID);      
    }
  }
}

void ReSceneData::removeCamera( const QString camID ) {
  if (cameras.contains(camID)) {
    if (selectedCamera == camID) {
      selectedCamera = "";
    }
    cameras.remove(camID);
  }
  if (!isInGUIMode()) {
    realityIPC->cameraRemoved(camID);      
  }
}

/*
 * Light management
 */

/**
 * Remove all the lights. The IBL light is constant and cannot be removed.
 * The first parameter determines if meshlights should be removed as well.
 * It defaults to false
 */
void ReSceneData::removeAllLights( const bool removeMeshLights ) {
  QStringList lightIDs = lights.keys();
  foreach(QString lightID, lightIDs) {
    auto light = lights.value(lightID);
    bool isRealityMeshLight = (light->getType() == MeshLight && !light->isHostAreaLight());
    if (lightID != RE_IBL_LIGHT_ID && !isRealityMeshLight) {
      deleteLight(lightID);
    }
  }
  if (!isInGUIMode()) {
    realityIPC->allLightsRemoved();
  }  
}

void ReSceneData::backupLights( const bool removeMeshLights ) {
  foreach(QString lightID, lights.keys()) {
    auto light = lights.value(lightID);
    bool isRealityMeshLight = (light->getType() == MeshLight && !light->isHostAreaLight());
    if (lightID != RE_IBL_LIGHT_ID && !isRealityMeshLight) {
      lightBackup[lightID] = lights.value(lightID);
    }
  }
}

//! Restore the lights from the backup. We don't actually assign the
//! pointers to the light list, we only restore the lights that are already
//! in the list of lights. The restore copies the properties of the saved
//! light to the light in the lights list.
void ReSceneData::restoreLightsFromBackup() {
  foreach(QString lightID, lightBackup.keys()) {
    if (lights.contains(lightID)) {
      *lights.value(lightID) = *lightBackup.value(lightID);
      lightBackup.remove(lightID);
    }
  }
  lightBackup.clear();
}

int ReSceneData::getNumLights() {
  return lights.count();
}

void ReSceneData::saveLight( const QString lightID, ReLightPtr light ) {
  lights[lightID] = light;
}

void ReSceneData::addLight( const QString lightID, 
                            const QVariantMap& data ) 
{
  // Avoid adding a light that is already present
  if (lightID != RE_IBL_LIGHT_ID && lights.contains(lightID)) {
    return;
  }

  ReLightType lightType = MeshLight;
  if (data.contains("type")) {
    lightType = static_cast<ReLightType>(data["type"].toInt());
  }
  bool isIBL = (lightType == IBLLight);

  ReConfigurationPtr config = RealityBase::getConfiguration();

  QString lightName;
  if (data.contains("name")) {
    lightName = data["name"].toString();    
  }
  else {
    lightName = "Reality light";
  }

  ReLightPtr light;
  if (isIBL) {
    light = lights.value(RE_IBL_LIGHT_ID);
  }
  else {
    light = ReLightPtr(new ReLight(lightName, lightID, lightType));
  }

  // If "ignore infinite lights" is on then we keep the light
  // off no matter what's in the host app
  if ( config->value(RE_CFG_IGNORE_INFINITE).toBool() && 
       (lightType == InfiniteLight) && 
       (lightID.toLower() != "sun") ) {
    light->turnOnOff(false);
  }
  else {
    light->turnOnOff(data["on"].toBool());
  }
  light->setIntensity(data["intensity"].toFloat());

  if (lightType == SpotLight) {
    light->setAngle(data["angle"].toFloat());
  }

  light->setColor(convertFloatColor(data["color"].toMap()["color"].toList()));

  if ( !data.contains("colorModel") ) {
    QColor c = light->getColor();
    if ( !(isGrayscale(c) || isPureWhite(c)) ) {
      light->setColorModel(ReLight::RGB);
    }
  }
  else {
    light->setColorModel(static_cast<ReLight::LightColorModel>(data["colorModel"].toInt()));
  }

  QString lightTextureFileName = data["color"].toMap().value("map").toString();
  if (lightTextureFileName != "") {
    if (isIBL) {
      light->setIBLMapFileName(lightTextureFileName);
      light->setIBLMapFormat(data.value("IBLMapFormat").toString());
    }
    else {
      light->setTexture(lightTextureFileName);
    }
  }

  if (data.contains("isHostAreaLight")) {
    light->setIsHostAreaLight(data["isHostAreaLight"].toBool());
  }

  if (!isIBL) {
    saveLight(lightID, light);
  }
  // Notify the UI
  if (!isInGUIMode()) {
    if (isIBL) {
      realityIPC->lightUpdated(lightID);      
    }
    else {
      realityIPC->lightAdded(lightID);      
    }
  }
}

QString ReSceneData::unlinkLightFromObject( const QString& lightID ) {
  // Simple brute-force scan of the object list. We don't have any fancier
  // method at this point.
  QString objID = "";

  ReGeometryObjectIterator i(objects);
  while( i.hasNext() ) {
    i.next();
    auto obj = i.value();
    if (obj->isLight()) {
      auto lights = obj->getLights();
      ReMaterialIterator li(*lights);
      while( li.hasNext() ) {
        li.next();
        ReLightMaterialPtr mat = li.value().staticCast<ReLightMaterial>();
        if (mat->getLight()->getID() == lightID) {
          objID = obj->getInternalName();
        }
      }
    }
  }
  if (objID != "") {
    auto obj = getObject(objID);
    obj->unlinkLight(lightID);
  }
  return objID;
}

void ReSceneData::deleteLight( const QString lightID, const bool deleteObjects ) {  
  if (destroying) {
    return;
  }
  if (lightID == RE_IBL_LIGHT_ID) {
    return;
  }
  if (lights.contains(lightID)) {
    auto light = lights.value(lightID);
    if (light->isHostAreaLight() && deleteObjects) {
      QString objID = unlinkLightFromObject(lightID);
      if (objID != "") {
        deleteObject(objID);
      }
    }
    lights.remove(lightID);
    // Notify the UI of the change
    if (!isInGUIMode()) {
      realityIPC->lightDeleted(lightID);
    }    
  }
}

void ReSceneData::renameLight( const QString lightID, const QString& newName ) {
  if (lights.contains(lightID) && lightID != RE_IBL_LIGHT_ID) {
    lights.value(lightID)->setName(newName);
  }
  if (!isInGUIMode()) {
    realityIPC->lightRenamed(lightID, newName);    
  }
}

void ReSceneData::updateLight( const QString& lightID, const QVariantMap lightData ) {
  if (!lights.contains(lightID)) {
    return;
  }
  ReLightPtr light = lights.value(lightID);

  if (light->getType() != IBLLight) {
    light->setName(lightData.value("name").toString());    
    ReLightType lightType = static_cast<ReLightType>(lightData["type"].toInt());
    light->setType(lightType);
    if (lightType == SpotLight) {
      light->setAngle(lightData["angle"].toFloat());
    }
    // Protect the projector spotlight from having its texture erased
    QString lightTexture = lightData.value("color").toMap().value( "map" ).toString();
    if (!lightTexture.isEmpty()) {
      light->setTexture(lightTexture);
    }
  }
  else {
    light->setIBLMapFormat( static_cast<ReLight::IBLType>
      ( lightData.value( "IBLMapFormat" ).toInt() ) 
    );    
    light->setIBLMapFileName( lightData.value("color").toMap().value( "map" ).toString() );
    light->setIBLRotationAngle(lightData.value("angle",0).toInt());
  }

  // If "ignore infinite lights" is on then we keep the light
  // off no matter what's in the host app
  ReConfigurationPtr config = RealityBase::getConfiguration();
  if ( config->value(RE_CFG_IGNORE_INFINITE).toBool() && 
       (light->getType() == InfiniteLight) && 
       (lightID.toLower() != "sun") ) {
    light->turnOnOff(false);
  }
  else {
    light->turnOnOff(lightData.value("on").toBool());
  }
  light->setIntensity(lightData["intensity"].toFloat());
  light->setColor(convertFloatColor(lightData["color"].toMap()["color"].toList()));
  if (lightData.contains("colorModel")) {
    light->setColorModel(static_cast<ReLight::LightColorModel>(
      lightData["colorModel"].toInt())
    );
  }

  // Notifies the GUI
  if (!isInGUIMode()) {
    realityIPC->lightUpdated(lightID);
  }

}

void ReSceneData::setLightType( const QString& lightID, const ReLightType newLightType ) {
  ReLightPtr light = getLight(lightID);
  if (!light.isNull()) {
    ReLightType oldLightType = light->getType();
    if (oldLightType == newLightType) {
      return;
    }
    if (oldLightType != IBLLight && newLightType == IBLLight) {
      deleteLight(lightID);
      getLight(RE_IBL_LIGHT_ID)->turnOnOff(true);        
    }
    else {
      light->setType(newLightType);
    }
  }
  else if (newLightType != IBLLight) {
    // If a light doesn't exist in our catalog it means that it was previously
    // converted to IBL. So we then add that light to the catalog again and
    // turn off the IBL
    ReLightPtr newLight = ReLightPtr( new ReLight(lightID, lightID, newLightType) );
    saveLight(lightID, newLight);
    getLight(RE_IBL_LIGHT_ID)->turnOnOff(false);
  }
  // Notify the UI of the change
  if (!isInGUIMode()) {
    realityIPC->lightTypeChanged(lightID);
  }    
}

void ReSceneData::setLightIntensity( const QString& lightID, const float gain ) {
  if (lights.contains(lightID)) {
    auto light = lights[lightID];
    light->setIntensity(gain);
  }
}

void ReSceneData::setIBLImage( const QString sIBLFileName ) {
  ReLightPtr iblLight = lights.value(RE_IBL_LIGHT_ID);
  if (iblLight) {
    iblLight->setIBLMapFileName(sIBLFileName);
  }
  // Notifies the GUI
  if (!isInGUIMode()) {
    realityIPC->lightUpdated(RE_IBL_LIGHT_ID);
  }
}

void ReSceneData::setIBLRotation( const float angle ) {
  ReLightPtr ibl = lights.value(RE_IBL_LIGHT_ID);
  if (ibl) {
    ibl->setIBLRotationAngle(angle);
  }
  // Notifies the GUI
  if (!isInGUIMode()) {
    realityIPC->lightUpdated(RE_IBL_LIGHT_ID);
  }
}

QString ReSceneData::getIBLPreviewMap() {
  ReLightPtr iblLight = lights.value(RE_IBL_LIGHT_ID);
  if ( !iblLight ) {
    return "";
  }
  return iblLight->getTexture();
}

ReLightDictionary ReSceneData::getLights() {
  return lights;
}

ReLightPtr ReSceneData::getLight( const QString lightID ) {
  return lights.value(lightID);
}


void ReSceneData::setLightMatrix( const QString lightName, const QVariantList& tm ) {
  ReLightPtr light = lights.value(lightName);
  if (!light.isNull()) {
    light->setMatrix(tm);
  }
}

void ReSceneData::setLightMatrix( const QString lightName, const ReMatrix& tm ) {
  ReLightPtr light = lights.value(lightName);
  if (!light.isNull()) {
    light->setMatrix(tm);
  }
}


void ReSceneData::linkVolumeToMaterial( const QString volumeName, 
                                        const QString& objectID, 
                                        const QString& materialName ) 
{
  properties.volumeMaterialLinks[volumeName][objectID].insert(materialName);
}

void ReSceneData::unlinkVolumeFromMaterial( const QString volumeName, 
                                            const QString& objectID, 
                                            const QString& materialName ) 
{
  // If we are in closing Reality then this can cause a lockup. 
  // We are destroying everything anyway...
  if (destroying) {
    return;
  }
  if ( !properties.volumeMaterialLinks.contains(volumeName) || 
       !properties.volumeMaterialLinks[volumeName].contains(objectID) ) 
  {
    return;
  }
  properties.volumeMaterialLinks[volumeName][objectID].remove(materialName);
  // Remove the object reference if it's empty
  if (properties.volumeMaterialLinks[volumeName][objectID].count() == 0) {
    properties.volumeMaterialLinks[volumeName].remove(objectID);
    // Remove unused SSS volumes. We identify such volumes by:
    //  a) The have been created automatically. Those are volumes
    //     that are not editable
    //  b) They have no material reference anymore
    bool volumeIsAutomatic = !volumes[volumeName]->isEditable();
    if (properties.volumeMaterialLinks[volumeName].count() == 0 && volumeIsAutomatic) {
      properties.volumeMaterialLinks.remove(volumeName);
      volumes.remove(volumeName);
    }
  }
  ReMaterialPtr mat = getMaterial(objectID, materialName);
  if (!mat.isNull()) {
    mat->unlinkVolume(volumeName);
  }
}

void ReSceneData::renameLinkedVolume( const QString oldName, const QString newName ) {
  if (properties.volumeMaterialLinks.contains(oldName)) {
    properties.volumeMaterialLinks[newName] = properties.volumeMaterialLinks.value(oldName);
    properties.volumeMaterialLinks.remove(oldName);
  }
}

void ReSceneData::saveVolume( ReVolumePtr vol ) {
  volumes[vol->getName()] = vol;
};

ReVolumePtr ReSceneData::getVolume( const QString name ) {
  return volumes[name];
};


ReVolumeDictionary ReSceneData::getVolumes() {
  return volumes;
};

bool ReSceneData::renameVolume( const QString oldName, const QString newName ) {
  if (oldName == newName) {
    return false;
  }
  if (volumes.contains(oldName)) {
    // We need to make a copy of the pointer or otherwise the container
    // we'll used an implicitly shared copy and the removal below will
    // erase the renamed volume as well.
    volumes[newName] = ReVolumePtr(volumes[oldName]);
    volumes[newName]->setName(newName);
    volumes.remove(oldName);
    // Update the linked materials
    ReObjectMaterialMap mappedMats = properties.volumeMaterialLinks.value(oldName);
    // volume map iterator
    QMapIterator< QString, QSet<QString> > vmi(mappedMats);
    while( vmi.hasNext() ) {
      vmi.next();
      // mat. name iterator
      QSetIterator<QString> mni(vmi.value());
      while( mni.hasNext() ) {
        ReMaterialPtr mat = getMaterial(vmi.key(), mni.next());
        if (!mat.isNull()) {
          mat->volumeRenamed(oldName, newName);
        }
      }
    }
    renameLinkedVolume(oldName, newName);
    return true;
  }
  return false;
}

bool ReSceneData::removeVolume( const QString name ) {
  if (volumes.contains(name)) {
    volumes.remove(name);
    return true;
  }
  return false;
};

bool ReSceneData::volumeExists( const QString name ) const {
  return volumes.contains(name);
}

QStringList ReSceneData::getMaterialsLinkedToVolume( const QString& objectID, 
                                                     const QString& volumeID )
{
  auto volInfo = properties.volumeMaterialLinks[volumeID];
  if (volInfo.count() == 0) {
    return QStringList();
  }
  QSetIterator<QString> i(volInfo[objectID]);
  QStringList mats;
  while( i.hasNext() ) {
    mats << i.next();
  }
  return mats;
}


/*
 This is where we export the scene   
 */
const QString ReSceneData::exportScene( const QString& format, 
                                        const int frameNo ) 
{
  ReBaseSceneExporter* exporter = sceneExporterFactory.getExporter(format);
  boost::any result;
  exporter->exportScene(frameNo, result);
  QString s;
  try {
    s = boost::any_cast<QString>(result);
  }
  catch( const boost::bad_any_cast& e ) {
    RE_LOG_INFO() << "Exception: " << e.what();
    s = "";
  }
  return s;
}

ReBaseSceneExporter* ReSceneData::getSceneExporter( const QString& format ) 
{
  return sceneExporterFactory.getExporter(format);
}

void ReSceneData::newScene() {
  volumes.clear();
  properties.volumeMaterialLinks.clear();
  objects.clear();
  cameras.clear();
  lights.clear();
  
  initScene();
}

// Static member
ReGeometryBuffer ReSceneData::geometryBuffer;

void ReSceneData::newGeometryBuffer( const QString& bufferName,
                                     const int numVertices, 
                                     const int numTriangles, 
                                     const bool hasUVs ) 
{
  if (geometryBuffer.numTriangles) {
    geometryBuffer.reset();
  }
  geometryBuffer.allocate(bufferName, numVertices, numTriangles, hasUVs);
}

float* ReSceneData::getGeometryVertexBuffer() {
  return geometryBuffer.vertices[0];
}

float* ReSceneData::getGeometryNormalBuffer() {
  return geometryBuffer.normals[0];
}

ReUVPoint* ReSceneData::getGeometryUVPointBuffer(){
  return geometryBuffer.uvmap;
}


int* ReSceneData::getGeometryFaceBuffer() {
  return geometryBuffer.triangles->a;
}

bool ReSceneData::isDisplacementEnabled() const {
  return properties.enableDisplacement;
};

void ReSceneData::setEnableDisplacement( bool newVal ) {
  properties.enableDisplacement = newVal;
};

void ReSceneData::restoreScene( const QVariantMap& sceneData, const bool mergeScene ) 
{
  ReQtSceneImporter sceneImporter;
  ReQtVolumeImporter volumeImporter;

  if (!mergeScene) {
    sceneImporter.restoreSceneData(
      sceneData["scene"].toMap(),
      sceneData.value(KEY_SCENE_MAIN_VERSION).toInt(),
      sceneData.value(KEY_SCENE_SUB_VERSION).toInt()
    );
  }
  // Check if the image and Lux scene files point to valid directories, and if 
  // not, reset them to the default
  auto outputSceneFileName = getSceneFileName();
  auto renderedImageFileName = getImageFileName();
  QFileInfo imageFileInfo(renderedImageFileName);
  QFileInfo sceneFileInfo(outputSceneFileName);
  bool imgFileIsValid, scnFileIsValid;
  imgFileIsValid = (!renderedImageFileName.isEmpty() 
                    && imageFileInfo.absoluteDir().exists());
  scnFileIsValid = (!outputSceneFileName.isEmpty() 
                    && sceneFileInfo.absoluteDir().exists());
  if (!imgFileIsValid || !scnFileIsValid) {
    setDefaultSceneName();
  }

  // Restoring the volumes
  QMapIterator<QString, QVariant> i(sceneData["volumes"].toMap());
  while ( i.hasNext() ) {
    i.next();
    saveVolume(volumeImporter.importVolume( i.value().toMap() ));
  }
  // Restore the lights
  ReQtLightImporter lightImporter;
  QMapIterator<QString, QVariant> li(sceneData["lights"].toMap());
  bool usesLuxCore = isOCLRenderingON() || cpuAccelerationEnabled();

  while ( li.hasNext() ) {
    li.next();
    ReLightPtr light = lightImporter.importLight(li.value().toMap());
    // If this is a scene that was saved with a previous version of Reality,
    // where mesh lights had the alpha channel enabled by default, then the
    // channel is disabled, if the scene uses LuxCore, because that feature
    // is not supported by LuxCore and exporting a light with alpha channel
    // will abort the render.
    if (usesLuxCore) {
      light->setAlphaChannel(false);
    }
    saveLight(li.key(), light);
  }

  // Restoring the objects
  QVariantMap objs = sceneData.value("objects").toMap();
  ReQtGeometryObjectImporter objImporter;
  QMapIterator<QString, QVariant> oi(objs);
  while( oi.hasNext() ) {
    oi.next();
    objects[oi.key()] = objImporter.importGeometryObject(
      oi.value().toMap()
    );
  }

  // Restore the cameras
  ReQtCameraImporter cameraImporter;
  QMapIterator<QString, QVariant> ci(sceneData["cameras"].toMap());
  while ( ci.hasNext() ) {
    ci.next();
    ReCameraPtr camera = cameraImporter.importCamera(ci.value().toMap());
    cameras[ci.key()] = camera;
  }
  // If the scene had data then it will need to be saved from this point 
  // on or we risk to have the host's data out of sync with ours
  setNeedsSaving(true);

  //! Notifies the GUI that a scene has been loaded in the host-app.
  //! Time for the GUI to request the data
  if (!isInGUIMode()) {
    realityIPC->sceneLoaded();
  }
}

QString& ReSceneData::exportMaterial( const QString& materialName, 
                                      const QString& objectName,
                                      const QString& shapeName,
                                      HostAppID scale ) 
{
  auto geometryExporter = ReLuxGeometryExporter::getInstance();
  return geometryExporter->exportMaterial(materialName, 
                                          objectName,
                                          shapeName,
                                          &geometryBuffer, 
                                          scale);
}

void ReSceneData::renderSceneExportInstance( const QString& objectName, 
                                             const QVariantMap& transform,
                                             const HostAppID scale ) 
{
  auto geometryExporter = ReLuxGeometryExporter::getInstance();
  sceneIncludeFile.write(
    geometryExporter->exportInstance( objectName, transform, scale ).toUtf8()
  );
}

void ReSceneData::renderSceneExportInstance( const QString& objectName, 
                                             const ReMatrix& transform,
                                             const HostAppID scale ) 
{
  ReBaseGeometryExporter* geometryExporter;
  geometryExporter = ReLuxGeometryExporter::getInstance();
  sceneIncludeFile.write(
    geometryExporter->exportInstance( objectName, transform, scale ).toUtf8()
  );
}


void ReSceneData::renderSceneStart( const QString& fName, 
                                    const unsigned int frameNo ) 
{
  QString sceneFileName = fName;
  if (sceneFileName.isEmpty()) {
    sceneFileName = getSceneFileName();
  }
  QFileInfo sceneInfo(sceneFileName);
  QFileInfo sceneIncludeInfo(
              QString("%1/%2.lxi")
                .arg(sceneInfo.absolutePath())
                .arg(sceneInfo.baseName())
  );
  sceneFile.setFileName(sceneFileName);
  sceneIncludeFile.setFileName(sceneIncludeInfo.absoluteFilePath());

  if (sceneInfo.exists()) {
    if (!sceneFile.remove()) {
      RE_LOG_WARN() << "Warning: cannot delete file " 
                    << sceneFileName.toStdString() 
                    << ". Rendering aborted";
      return;
    }
  }
  if (sceneIncludeInfo.exists()) {
    if (!sceneIncludeFile.remove()) {
      RE_LOG_WARN() << "Warning: cannot delete file " 
                    << sceneIncludeInfo.absoluteFilePath().toStdString()
                    << ". Rendering aborted";
      return;
    }
  }
  QFile flmFile(QString("%1/%2.flm")
                  .arg(sceneInfo.absolutePath())
                  .arg(sceneInfo.baseName())
  );
  if (flmFile.exists()) {
    if ( !flmFile.remove() ) {
      RE_LOG_WARN() << "Warning: could not delete file "
                    << flmFile.fileName().toStdString()
                    << ". Rendering aborted";
      return;
    };
  }

  if ( !sceneFile.open(QIODevice::WriteOnly) ) {
    RE_LOG_WARN() << "Error: could not open the file "
                  << sceneInfo.absoluteFilePath().toStdString()
                  << " for writing. Rendering aborted";
    return;
  }
  if ( !sceneIncludeFile.open(QIODevice::WriteOnly) ) {
    RE_LOG_WARN() << "Error: could not open the file "
                  << sceneIncludeInfo.absoluteFilePath().toStdString()
                  << " for writing. Rendering aborted";
    return;
  }
  // Initialize the texture cache
  ReLuxTextureExporter::initializeTextureCache();
  ReLuxTextureExporter::enableTextureCache(true);

  QString sceneText = exportScene("lux", frameNo);
  sceneFile.write(sceneText.toUtf8());
  sceneIncludeFile.write(
    "#\n"
    "# LuxRender include file. Generated by Reality plugin\n"
    "#\n"
  );
};

void ReSceneData::renderSceneExportMaterial( const QString& matName,  
                                             const QString& objName,
                                             const QString& shapeName,
                                             const HostAppID scale ) 
{
  sceneIncludeFile.write(
    QString(
      exportMaterial( matName, objName, shapeName, scale )
    ).toUtf8()
  );
};

void ReSceneData::renderSceneFinish( const bool runRenderer ) {
  sceneFile.write("# End of scene\n");
  sceneIncludeFile.write("# End of include file\n");
  sceneFile.close();
  sceneIncludeFile.close();
  // Clear the cache
  ReLuxTextureExporter::initializeTextureCache();
  
  if (runRenderer) {
    switch(getRenderer()) {
      case LuxRender: {
        ReLuxRunner rr;
        rr.runGuiLux(
          getSceneFileName(), 
          getNumThreads(),
          getLuxLogLevel()
        );
        break;
      }
      case SLG: {
        ReLuxRunner luxRunner;
        if ( luxRunner.runSLG(getSceneFileName()) == ReLuxRunner::LR_COULD_NOT_START ) {
          RE_LOG_WARN() << "Could not start SLG!";
        }
        break;
      }
    }
  }
};

void ReSceneData::renderSceneObjectBegin( const QString objName ) {
  // Lux does not support instancing for all objects when using 
  // the GPU. In particular, it does not support instancing for 
  // emitters. So we find out if the object emits light. If so, then
  // we avoid using the instancing system.

  // The GPU-acceleration does not support instancing
  if ( isOCLRenderingON() )  {
    return;
  }

  bool isInstanceSource = ReRenderContext::getInstance()->isInstantiator(objName);
  if (isInstanceSource) {        
    auto geometryExporter = ReLuxGeometryExporter::getInstance();
    sceneIncludeFile.write( geometryExporter->exportObjectBegin(objName).toUtf8() );
  }
}

void ReSceneData::renderSceneObjectEnd( const QString objName ) {
  // Lux does not support instancing for all objects when using 
  // the GPU. In particular, it does not support instancing for 
  // emitters.

  // The GPU-acceleration does not support instancing
  if ( isOCLRenderingON() ) {
    return;
  }
  bool isInstanceSource = ReRenderContext::getInstance()->isInstantiator(objName);

  if (isInstanceSource) {
    auto geometryExporter = ReLuxGeometryExporter::getInstance();
    sceneIncludeFile.write( geometryExporter->exportObjectEnd(objName).toUtf8() );
  }
}

void ReSceneData::renderSceneIncludeFileCustomData( const QString str ) {
  sceneIncludeFile.write(str.toUtf8());
}

void ReSceneData::notifyGUI( const QString msg, const QString id ) {
  // Notify the UI
  if (!isInGUIMode()) {
    if (msg == "lightAdded") { 
      realityIPC->lightAdded(id);
    }
  }
}
