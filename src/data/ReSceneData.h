/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 *
 * Reality scene data.
 *
 * This class stores the data from the scene, gathered from the host (Poser or DAZ Studio).
 */

#ifndef RESCENEDATA_H
#define RESCENEDATA_H

#include <QtCore>
#include <QHash>
#include <QSharedPointer>

#include "ReDefs.h"
#include "ReTexture.h"
#include "ReGeometry.h"
#include "ReGeometryObject.h"
#include "ReVolumes.h"
#include "ReCamera.h"
#include "ReLight.h"
#include "ReSurfaceIntegrator.h"
#include "exporters/ReSceneExporterFactory.h"
#include "ReIPC.h"
#include "reality_lib_export.h"

#define KEY_SCENE_WIDTH               "sceneWidth"
#define KEY_SCENE_HEIGHT              "sceneHeight"
#define KEY_SCENE_ALPHA_CHANNEL       "hasAlphaChannel"
#define KEY_SCENE_FRAME_MULTIPLIER    "frameMultiplier"
#define KEY_SCENE_IMAGE_NAME          "imageFileName"
#define KEY_SCENE_IMAGE_FILE_FORMAT   "imageFileFormat"
#define KEY_SCENE_SCENE_NAME          "sceneFileName"
#define KEY_SCENE_RENDERER            "selectedRenderer"
#define KEY_SCENE_GEOMETRY_FORMAT     "geometryFormat"
#define KEY_SCENE_OUTLIER_REJECTION   "outlierRejection" 
#define KEY_SCENE_MAX_SPX             "maxSPX"
#define KEY_SCENE_GAMMA               "gamma"
#define KEY_SCENE_PRESERVE_HUE        "preserveLightHue"
// #define KEY_SCENE_GPU_SUPPORT         "GPUEnabled"
#define KEY_SCENE_NUM_THREADS         "numThreads"
#define KEY_SCENE_ENABLE_DISPLACEMENT "enableDisplacement"
#define KEY_SCENE_OVERWRITE_WARNING   "overwriteWarning"
#define KEY_SCENE_SURFACE_INTEGRATOR  "surfaceIntegrator"
#define KEY_SCENE_OCL_GROUP_SIZE      "OCLGroupSize"
//! Wheter the scene uses OpenCL for rendering
#define KEY_SCENE_USE_OCL             "UseOCL"
#define KEY_SCENE_OCL_DEVICES_ENABLED "OCLDevicesEnabled"
#define KEY_SCENE_OCL_NUM_DEVICES     "OCLNumDevices"
#define KEY_SCENE_OCL_BIAS            "OCLBias"
#define KEY_SCENE_CPU_BIAS            "CPUBias"
#define KEY_SCENE_VOLUME_LINKAGE      "volumeLinkage"
#define KEY_SCENE_TEXTURE_COLLECTION  "textureCollection"
#define KEY_SCENE_TEXTURE_SIZE        "textureSize"
//! Deprecated
#define KEY_SCENE_TURBO_MODE          "turboMode"

#define KEY_SCENE_LOG_LEVEL           "logLevel"
#define KEY_SCENE_RENDER_AS_STATUE    "renderAsStatue"
#define KEY_SCENE_MAIN_VERSION        "mainVersion"
#define KEY_SCENE_SUB_VERSION         "subVersion"
#define KEY_SCENE_PATCH_VERSION       "patchVersion"
#define KEY_SCENE_SELECTED_CAMERA     "selectedCamera"
#define KEY_SCENE_RENDER_SAMPLER      "renderSampler"
#define KEY_SCENE_NOISE_AWARE_SAMPLER "noiseAwareSampler"
#define KEY_SCENE_CPU_ACCEL           "cpuAcceleration"

namespace Reality {

/**
 A class used to communicate with the host app-side plugin. The class is allocated by Reality's library 
 and it provides storage for the geometry and all the other data components used to push the geometry from
 the host app into Reality's exporter. For example, the Poser Python classes use this class to pass
 the geometry, UV maps, normal maps and material associations. 
 */
class ReGeometryBuffer {

  public:
    int numTriangles;
    int numVertices;

    QString name;

    // Variable: vertices
    //   The buffer that holds the list of vertices for a unit of geometry
    ReVectorF* vertices;

    // List of the polygons, triangulated. Each entry is a triangle, each 
    //  triangle element is an index into the <vertices> lits
    ReTriangle* triangles;

    // The list of UV point coordinates
    ReUVPoint* uvmap;

    // Normal  vectors
    ReVectorF* normals;


    ReGeometryBuffer() {
      init();
    }

    void init() {
      numTriangles = 0;
      numVertices  = 0;
      vertices     = NULL;
      uvmap        = NULL;
      normals      = NULL;
      triangles    = NULL;
    }

    void reset() {
      delete[] vertices;
      delete[] uvmap;
      delete[] normals;
      delete[] triangles;
      init();
    }

    void allocate( const QString& bufferName,
                   const int requestedVertices, 
                   const int requestedTriangles,
                   const bool hasUVs ) 
    {
      name = bufferName;
      numVertices  = requestedVertices;
      numTriangles = requestedTriangles;

      vertices    = new ReVectorF[numVertices];
      normals     = new ReVectorF[numVertices];
      if (hasUVs) {
        uvmap       = new ReUVPoint[numVertices];      
      }
      else {
        uvmap = NULL;
      }
      // memset(uvmap, 0 , numVertices * sizeof(ReUVPoint));
      triangles   = new ReTriangle[numTriangles];
    }
};

//! A dictionary keyed by string that holds the list of objects in the scene.
//! This is the catalog of objects managed by the <ReSceneData> class.
typedef QHash<QString,ReGeometryObjectPtr> ReGeometryObjectDictionary;
typedef QHashIterator<QString,ReGeometryObjectPtr> ReGeometryObjectIterator;

//! Camera dictionary
typedef QHash<QString, ReCameraPtr> ReCameraDictionary;
typedef QHashIterator<QString, ReCameraPtr> ReCameraIterator;

//! Light dictionary
typedef QHash<QString, ReLightPtr> ReLightDictionary;
typedef QHashIterator<QString, ReLightPtr> ReLightIterator;

//! Volume dictionary
typedef QHash<QString, ReVolumePtr> ReVolumeDictionary;
typedef QHashIterator<QString, ReVolumePtr> ReVolumeIterator;

/*
 * Volume/Material linkage
 */

//! We maintain a list of the connections between volumes and the
//! the objects that use them. A volume is a scene-wide entity but it is
//! associated with single materials, which, in turn, are part of objects.
//! For example, a skin material is usually
//! connected to one volume for the SSS. So, we keep track of that connection
//! in the ReSceneData class.
//! 
//! The list, actually a map, is keyed by volume name and each entry is a
//! another map keyed by object ID.
//! 
//! <volume name> => { 
//!    <objectID 1> { <material name 1> ... <material name n>},
//!    <objectID n> { <material name 1> ... <material name n>},
//  }
//!
//! First we define the type for the value.
//! The first typedef defines a map between an object and the set of 
//! materials that are connected to a volume.
//! The map is keyed by object ID and each value is a set of strings.
//! Each string in the set is a name of a material that has a connection
//! to a volume.
//! <objectID> [<matName[0]..<matName[n]>]
typedef QMap< QString, QSet<QString> > ReObjectMaterialMap;
//! Iterator for the ReObjectMaterialMap map
typedef QMapIterator< QString, QSet<QString> > ReObjectMaterialMapIterator;

//! Secondly, we define another map that links the volumes to the objects
//! that they are connected to. Each entry is a volume name. Each value
//! is a ReObjectMaterialMap.
typedef QMap<QString, ReObjectMaterialMap> ReVolumeMaterialLinks;

//! Iterator, to make the job of traversing the list easier
typedef QMapIterator<QString, ReObjectMaterialMap> ReVolumeLinkageIterator;

/**
 * This struct implements the storage for the data used by ReSceneData to
 * control how the scene is rendered. The fields in this struct are accessible
 * via the UI and controlled by the user.
 */  
struct ReSceneRenderOptions {

  //! Width of the scene
  unsigned short int sceneWidth;

  //! Height of the scene
  unsigned short int sceneHeight;

  //! Lux scene file name
  QString outputSceneFileName;

  //! Image file name
  QString renderedImageFileName;

  //! Image file format: PNG, Targa or OpenEXR
  ExportImageFileFormat imageFileFormat;

  //! The selected renderer.
  ReRenderers renderer;

  bool alphaChannel;
  float frameMultiplier;

  //! Lux native, Binary PLY, Text PLY
  GeometryFileFormat geometryFormat;

  quint16 outlierRejection;

  //! The sampler used to render the scene
  RenderSampler sampler;

  //! Flag that indicates if the Sampler for LuxRender uses the 
  //! noise-aware option
  bool noiseAwareSampler;

  //! Maximum number of samples to compute
  quint32 maxSPX;

  //! The gamma used for the scene
  float gamma;

  //! Preserve light hueflag.
  bool preserveLightHue;

  //! If this is true, the default, displacement maps are
  //! used, if defined. Otherwise  disp.maps are not exported
  //! even if they are defined for the materials.
  bool enableDisplacement;

  //! Number of rendering threads for Lux
  quint16 numThreads;

  //! If on it causes Reality to ask for confirmation to overwrite 
  //! the renderer's scene file, is it exists
  bool overwriteWarning;

  //! If enabled, it causes all materials to be rendered as white matte.
  bool renderAsStatue;

  //! The OpenCL Workgroupsize parameter
  quint16 OCLGroupSize;

  //! The OpenCL device enabled. This is that indicates if a device is
  //! to be used during rendering or not.
  //! Bit 0 is device number 0, bit 1 is device number 1 and so on.
  //! For example, if device 0 is disable, and device 1 and 2 are enabled,
  //! that will translate into a configurayion string of 
  //! "opencl.devices.select = 011" 
  quint16 OCLDeviceFlags;

  //! The number of OpenCL devices available. This includes all devices,
  //! regardless of type.
  quint16 numOCLDevices;

  //! Indicates if the user has enabled the OCL biased rendering. Stored
  //! as an int to allow future expansions beyond the simple true/false
  //! meaning.
  RE_UINT OCLBias;

  //! Indicates if the user has enabled the CPU biased rendering. Stored
  //! as an int to allow future expansions beyond the simple true/false
  //! meaning.
  RE_UINT CPUBias;

  //! Flag that indicates if we render with OpenCL
  bool useOpenCL;

  //! Texture collection
  bool textureCollection;
  ReTextureSize textureSize;

  //! Flag that indicates if we are using cpu acceleration
  bool cpuAcceleration;

  LUX_LOG_LEVEL luxLogLevel;

  //! The backlinks between volumes and the materials that use them
  ReVolumeMaterialLinks volumeMaterialLinks;

  //! The Surface Integrator used by the scene
  ReSurfaceIntegratorPtr surfaceIntegrator;

  friend QDataStream& operator <<( QDataStream& strm, ReSceneRenderOptions& opt );
  friend QDataStream& operator >>( QDataStream& strm, ReSceneRenderOptions& opt );

};

inline QDataStream& operator <<( QDataStream& strm, ReSceneRenderOptions& opt ) {
  strm.resetStatus();
  strm << opt.sceneWidth          << opt.sceneHeight
       << opt.outputSceneFileName << opt.renderedImageFileName
       << (quint16)opt.imageFileFormat
       << (quint16)opt.renderer
       << opt.alphaChannel        << opt.frameMultiplier
       << (quint16)opt.geometryFormat
       << opt.outlierRejection
       << (quint16)opt.sampler    << opt.noiseAwareSampler
       << opt.maxSPX              << opt.gamma
       << opt.preserveLightHue    << opt.enableDisplacement
       << opt.numThreads          << opt.overwriteWarning
       << opt.renderAsStatue      << opt.OCLGroupSize
       << opt.OCLDeviceFlags      << opt.numOCLDevices
       << opt.OCLBias             << opt.CPUBias
       << opt.useOpenCL           << opt.textureCollection
       << (quint16)opt.textureSize
       << opt.cpuAcceleration
       << (quint16)opt.luxLogLevel
       << opt.volumeMaterialLinks
       << opt.surfaceIntegrator;

  return strm;
}

inline QDataStream& operator >>( QDataStream& strm, ReSceneRenderOptions& opt ) {
  quint16 imageFileFormatInt,
          rendererInt,
          geometryFormatInt,
          samplerInt,
          textureSizeInt,
          luxLogLevelInt;

  strm >> opt.sceneWidth          >> opt.sceneHeight
       >> opt.outputSceneFileName >> opt.renderedImageFileName
       >> imageFileFormatInt
       >> rendererInt
       >> opt.alphaChannel        >> opt.frameMultiplier
       >> geometryFormatInt
       >> opt.outlierRejection
       >> samplerInt              >> opt.noiseAwareSampler
       >> opt.maxSPX              >> opt.gamma
       >> opt.preserveLightHue    >> opt.enableDisplacement
       >> opt.numThreads          >> opt.overwriteWarning
       >> opt.renderAsStatue      >> opt.OCLGroupSize
       >> opt.OCLDeviceFlags      >> opt.numOCLDevices
       >> opt.OCLBias             >> opt.CPUBias
       >> opt.useOpenCL           >> opt.textureCollection
       >> textureSizeInt          >> opt.cpuAcceleration
       >> luxLogLevelInt
       >> opt.volumeMaterialLinks       
       >> opt.surfaceIntegrator;

  opt.imageFileFormat   = static_cast<ExportImageFileFormat>(imageFileFormatInt);
  opt.renderer          = static_cast<ReRenderers>(rendererInt);
  opt.geometryFormat    = static_cast<GeometryFileFormat>(geometryFormatInt);
  opt.sampler           = static_cast<RenderSampler>(samplerInt);
  opt.textureSize       = static_cast<ReTextureSize>(textureSizeInt);
  opt.luxLogLevel       = static_cast<LUX_LOG_LEVEL>(luxLogLevelInt);

  return strm;
}

/**
 * This class provides a system for the storing scene data. "Scene Data" refers to
 * everything that Reality needs to know about a given scene in order to render it.
 * This includes:
 *     - Camera information
 *     - Frame information
 *     - Light list
 *     - List of all the materials for all the figures involed and their composition
 *
 * The structure of the data stored, for the geometry objects is:
 *
 * Objects: ListOfObjects
 *
 * Each object in the list contains a dictionary of all the materials for the objects.
 * The entries are keyed by material GUID.
 *
 * NOTE: some methods are declared virtual because this class is dynamically loaded. Without
 * the virtual declaration the linker will fail when linking clients that dynamically load this
 * class.
 *
 */
class REALITY_LIB_EXPORT ReSceneData {

private:

  ReSceneExporterFactory sceneExporterFactory;

  //! Creator function for the LuxRender scene type. This is used with the
  //! \ref ReSceneExporterFactory class.
  static ReBaseSceneExporter* getLuxRenderSceneExporter(void);

  //! Creator function for the Qt scene type. This is used with the
  //! \ref ReSceneExporterFactory class.
  static ReBaseSceneExporter* getQtSceneExporter(void);

  //! Creator function for the JSON scene type. This is used with the
  //! \ref ReSceneExporterFactory class.
  static ReBaseSceneExporter* getJSONSceneExporter(void);

  //! Creator function for the SLG scene type. This is used with the
  //! \ref ReSceneExporterFactory class.
  static ReBaseSceneExporter* getSLGSceneExporter(void);

protected:

  //! The data block
  ReSceneRenderOptions properties;

  //! The animation properties are not stored in the Reality data
  //! and therefore they are separate from the ReSceneRenderOptions object.
  //! These values are not used in the host side of RealitySceneData.
  //! Instead they are set by the UI-side instance of the class and set
  //! by the plugin. The UI then queries these properties whenever the user 
  //! wants to specify a range for the animation to be rendered.
  int animationStartFrame,
      animationEndFrame,
      animationFPS;

  //! This flag indicates if the Scene object is the one connected to the GUI 
  //! or the one running in the host-app side.
  bool inGUIMode; //!

  //! The scene is saved inside the host-app scene only if the GUI has been called at
  //! least once or if the scene had been saved befoe. Otherwise the Reality data will 
  //! not be passed at the host-app.
  //! This flag is false by defaul and it gets set to true when the GUI starts or when the 
  //! scene data is loaded.
  bool needsSaving; //!

  //! Determines how often Lux will show the updated render on screen
  quint16 displayInterval; //!

  //! Determines how often Lux will write the image file to disk
  quint16 writeInterval; //!

  // LUX_LOG_LEVEL luxLogLevel;
  /*********************************************
   * Internal geometry objects
   *********************************************/  

  //! The list of objects in the scene, not counting lights and cameras
  ReGeometryObjectDictionary objects;

  //! List of lights
  ReLightDictionary lights;

  //! Backup list of lights. See backupLights method
  ReLightDictionary lightBackup;

  //! List of cameras
  ReCameraDictionary cameras;
  QString selectedCamera; //!! Shouldn't this be passed around?

  //! List of volumes
  ReVolumeDictionary volumes;

  //! The geometry buffer used during the export
  static ReGeometryBuffer geometryBuffer;

  /**
   * objects need to delete associated lights. This is done via the call to
   * ReSceneData::deleteLight(). If the call happens when the scene object is
   * getting destroyed then it will cause Reality to crash on exit. This is because
   * the lights object is destroyed before the object's object. By setting the 
   * following variable to true we can decide to ignore calls that can crash Reality
   * during the destruction of the ReSceneData object. This is safe because the whole
   * set of data is getting destroyed anyway. 
   */
  bool destroying;

  /**
   * List used to support the function of methods markObjectsForDeletion() and
   * deleteOrphanesObjects()
   */
  QStringList objectsToDelete;

  //! File object used to write the Lux scene
  QFile sceneFile;

  //! The include file used in the main Lux scene
  QFile sceneIncludeFile;

public:

  //! Constructor
  ReSceneData();

  //! Destructor
  ~ReSceneData();

  inline bool isDestroying() {
    return destroying;
  }

  void initScene();
  void setDefaultSceneName();

  void setGUIMode( const bool onOff = true );
  bool isInGUIMode();
  // const bool isGPUEnabled() const;
  // void setGPUFlag( bool onOff );

  //! Set/Get the flag about CPU acceleration
  inline void setCpuAcceleration( const bool val ) {
    properties.cpuAcceleration = val;
  }
  
  inline bool cpuAccelerationEnabled() {
    return properties.cpuAcceleration;
  }

  void setNumThreads( const int newVal );
  int getNumThreads() const;
  //! Sets the verbosity of the Lux log
  void setLuxLogLevel( LUX_LOG_LEVEL logLevel );
  LUX_LOG_LEVEL getLuxLogLevel();

  void setNeedsSaving( const bool newVal );
  bool doesNeedsSaving() const;

  //! Returns wheter or not the displacement is enabled for the whole 
  //! scene
  bool isDisplacementEnabled() const;
  
  //! Enable or disable the displacement for the whole scene
  void setEnableDisplacement( bool newVal );
  
  const ReSurfaceIntegratorPtr getSurfaceIntegrator();
  void setSurfaceIntegrator( const ReSurfaceIntegratorType si );

  void linkVolumeToMaterial( const QString volumeName, 
                                     const QString& objectID,
                                     const QString& materialName );
  void unlinkVolumeFromMaterial( const QString volumeName,
                                         const QString& objectID,
                                         const QString& materialName );

  void renameLinkedVolume( const QString oldName, const QString newName );

  //! Returns a list of material names that use a given volume
  //! \param objectID The name of the object that contains the materials
  //!                 linked to a specific volume
  //! \param volumeID The nameof the volume for which we want to know the
  //!                 names of the linked materials
  //! \return A QStringList with the name of the linked materials
  QStringList getMaterialsLinkedToVolume( const QString& objectID, 
                                          const QString& volumeID );

  void setSceneFileName( const QString newName );
  QString getSceneFileName();

  void setImageFileName( const QString imageName );
  QString getImageFileName();

  void setFrameSize( const unsigned int width, const unsigned int height);
  quint16 getWidth() const;
  quint16 getHeight() const;

  bool hasAlphaChannel() const;
  void setAlphaChannel( bool onOff = true );

  /*
   Method: getGamma
   */
  float getGamma() const;

  /*
   Method: setGamma
   */
  void setGamma( float newVal );

  /*
   Method: getMaxSPX
   */
  quint32 getMaxSPX() const;

  /*
   Method: setMaxSPX
   */
  void setMaxSPX( quint32 newVal );
  
  /*
   Method: getDisplayInterval
   */
  quint16 getDisplayInterval() const;
  /*
   Method: setDisplayInterval
   */
  void setDisplayInterval( quint16 newVal );
    
  /*
   Method: getWriteInterval
   */
  quint16 getWriteInterval() const;
  /**
   Sets how often, in seconds, Lux will write the image to disk
   */
  void setWriteInterval( quint16 newVal );

  quint16 getOutlierRejection() const;

  /**
   Sets the outlier rejection parameter of Lux
   */
  void setOutlierRejection( quint16 newVal );

  //! Sets the noise-aware option
  inline void setNoiseAware( const bool newVal ) {
    properties.noiseAwareSampler = newVal;
  }

  inline bool isNoiseAwareOn() const {
    return properties.noiseAwareSampler;
  }

  /**
   * Returns the value of property sampler
   */
  inline RenderSampler getSampler() const {
    return properties.sampler;
  };
  /**
   Sets the value of property sampler
  */
  inline void setSampler( RenderSampler newVal ) {
    properties.sampler = newVal;
  };

  /**
   */
  int getOCLGroupSize() const;

  //! Returns the bitmask that indiactes the enabled OpenCL devices
  unsigned char getOCLDeviceFlags() const;

  void setOCLDeviceFlags( quint16 flags );

  //! Set what OpenCL device should be used when there are multiple GPUs 
  void setOCLDeviceFlag( quint16 deviceNo, const bool onOff );

  /**
   */
  void setOCLGroupSize( int newVal );

  inline void setNumOCLDevices( const unsigned int numDev ) {
    properties.numOCLDevices = numDev;
  }

  inline unsigned int getNumOCLDevices() {
    return properties.numOCLDevices;
  }

  //! Get the value of the OCL bias
  inline RE_UINT getOCLBias() {
    return properties.OCLBias;
  }

  //! Set the value for the OCL bias
  inline void setOCLBias( const RE_UINT val ) {
    properties.OCLBias = val;
  }

  //! Get the value of the OCL bias
  inline RE_UINT getCPUBias() {
    return properties.CPUBias;
  }

  //! Set the value for the OCL bias
  inline void setCPUBias( const RE_UINT val ) {
    properties.CPUBias = val;
  }

  bool hasTextureCollection() const;

  void setTextureCollection( const bool isOn );

  ReTextureSize getTextureSize() const;
  void setTextureSize( const ReTextureSize newSize );

  /*
   Method: getImageFileFormat
   */
  ExportImageFileFormat getImageFileFormat() const;
  /*
   Method: setImageFileFormat
   */
  void setImageFileFormat( ExportImageFileFormat newVal );

  /**
   * Returns wheter the 
   */
  bool getPreserveLightHue() const;
  /**
   * Enable/disable the preservation of the light hue
   */
  void setPreserveLightHue( bool newVal );

  void setFrameMultiplier( const float val );
  float getFrameMultiplier() const;

  //! Returns whether OpenCL rendering is on
  bool isOCLRenderingON() const;

  //! Enable/disable the use of SLGRender
  void setOCLRendering( const bool isOn );

  /**
   Return wheter the overwrite warning flag is enabled
   */
  bool getOverwriteWarning() const;

  /**
   * Enable/disable the warning when overwriting a scene.
   */
  void setOverwriteWarning( bool newVal );

  void setRenderAsStatue( const bool onOff );
  bool isRenderAsStatueEnabled() const;

  //! Restore the data from a saved scene
  //! \param sceneData A map that describes all the scene information grouped
  //!                  by objetcs, lights, cameras etc.
  //! \param mergeScene If true the scene information about file names, scene 
  //!                   parameters in the Render tab and other general parameters
  //!                   will not be restored, only the objects, lights, cameras
  //!                   and volumes will be added to the scene.
  void restoreScene( const QVariantMap& sceneData, const bool mergeScene = false );

  /**
   Returns the geometry format used by tthe scene
   */
  GeometryFileFormat getGeometryFormat() const {
    return properties.geometryFormat;
  };
  /**
   Set the geometry format used to export the scene
   */
  void setGeometryFormat( GeometryFileFormat newVal ) {
    properties.geometryFormat = newVal;
  };

  //! Returns which renderer has been selected
  ReRenderers getRenderer() const;

  //! Sets the renderer to be used to render the scene
  void setRenderer( const ReRenderers newVal );
  
  //! Returns the output data as a JSON string
  QString getOutputData();

  void getOutputData( QVariantMap& data );

  //! Sets the whole output data from a JSON object
  void setOutputData( QString data );

  //! Serializes all the render options, the properties member,
  //! to a QByteArray, ready to be streamed.
  inline void getRenderOptions( QByteArray& renderData ) {
    QDataStream stream(&renderData, QIODevice::WriteOnly);
    stream << properties;
  }

  //! Deserializes the render options from a QByteArray in a way that is
  //! simmetrical to the getRenderOptions() method.
  inline void setRenderOptions( QByteArray& renderData ) {
    QDataStream stream(&renderData, QIODevice::ReadOnly);
    stream >> properties;
  }

  /**
   * Add one scene object to the database. 
   * 
   * \param  objName - The name of the object in the hosting application.
   * \param  internalName - See <ReGeometryObject> for details.
   * \param  geometryFile - Name of the geometry file that holds the mesh. See <ReGeometryObject>.
   * \return True if the object is added, false otherwise.
   */
  bool addObject( const QString objName, const QString internalName, const QString geometryFile );

  //! Overloaded function used when de-serializing the objects at the GUI level
  bool addObject( ReGeometryObject* objPtr );

  void renameObject( const QString objectID, const QString newName );

  //! Changes the ID of an object.
  void renameObjectID( const QString objectID, const QString newID );

  //! Delete an object based on the object's ID. If the usePublicName 
  //! parameter is set to true the object is located using the public name
  //! instead.
  //! \param objectName The unique identifier of the object. This is the key
  //!                   used for the objects map. If the usePublicName 
  //!                   parameter is set to true then this parameter is 
  //!                   interpreted to be the publci name instead.
  //! \param usePublicName If this parameter is false, the default, then the 
  //!                      deletion is done interpreting the objectName as
  //!                      the internal name of the object. If this parameter
  //!                      is true the name is interpreted to be the public one.
  void deleteObject( const QString objectName, const bool usePublicName = false );

  void setObjectVisible( const QString objectID, const bool isVisible );

  //! Mark an object as a light. This is typically used to integrated the
  //! hosts's area light into Reality, as a mesh light. The procedure is meant 
  //! to work in this sequence:
  //!     - Create an object
  //!     - Mark the object as light
  //!     - Adde the material of the object
  //! While not a constraint, it is expected that an area/mesh light has only
  //! one material, which is assigned to the light-emission plane.
  inline void markObjectAsLight( const QString objName, const bool onOff ) {
    auto obj = objects[objName];
    if (obj) {
      obj->markAsLight(onOff);
    }
  }

  /**
   Add one material to the database. This method receives the data in JSON format and
   runs a conversion algorithm base on heuristicts that try to guess the best type of
   material based on the data passed. 
  
   Parameters:
     objName - The name of the object that owns the material
     matGUID - The material Global Unique ID
     materialData - A JSON string that holds the data description gathered from the 
                    hosting application (Poser or Studio).
   */
  void addMaterial( const QString objName,
                    const QString matGUID,
                    const QString materialData);

  //! Overloaded version
  void addMaterial( const QString objName,
                    const QString matGUID,
                    const QVariantMap& materialData);

  //! Updates a material's definition with new data from the host app.
  //! The material data is provided in JSON format in the materialData 
  //! parameter.
  void updateMaterial( const QString objName,
                       const QString matGUID,
                       const QString materialData);

  //! Updates a material's definition with new data from the host app.
  //! Overloaded version to use QVariantMap instead of a JSON string.
  void updateMaterial( const QString objName,
                       const QString matGUID,
                       const QVariantMap& materialData );

  //! Given an object ID this method returns a list of string containing
  //! the names of the materials associated with the object.
  QStringList getMaterialNames( const QString& objectID );
  
  /**
   * Reloads the text-serialized data and restores the material to its original 
   * state.
   * This method is used to restore a material that has been saved int he host 
   * app's scene. The data comes from a previous session of Reality and was 
   * obtained with the Rematerial::serialize(QDataStream textStream) method.
   */
  void restoreMaterial( const QString objName,
                        const QString matID,
                        const QString materialData);
  /**
   * Restores the light state from the JSON data saved in the host-app scene 
   * file.
   */
  void restoreLight( const QString& dataStr );

  /**
   * Changes the type of a material.
   *
   * \param objectID The unique identifier of the object that owns the material
   * \param materialID The name of the material to change
   * \param newType The new type that will be assigned to the material
   * \param jsonData JSON strign that defines the attributes of the material
   */ 
  void changeMaterialType( const QString objectID, 
                           const QString materialID, 
                           const ReMaterialType newType,
                           const QString jsonData );

  //! Overloaded version to use the QVarianMap data directly
  void changeMaterialType( const QString objectID, 
                           const QString materialID, 
                           const ReMaterialType newType,
                           const QVariantMap& matData );

  inline const QString getMaterialShortType(const QString& objID,
                                            const QString& matName) 
  {
    if (objects.contains(objID)) {
      auto mat = objects[objID]->getMaterial(matName);
      if (!mat) {
        return RE_MAT_TYPE_CODE_UNDEFINED;
      }
      auto matType = mat->getType();
      switch(matType) {
        case MatGlass:
          return RE_MAT_TYPE_CODE_GLASS;
        case MatGlossy:
          return RE_MAT_TYPE_CODE_GLOSSY;
        case MatHair:
          return RE_MAT_TYPE_CODE_HAIR;
        case MatMatte:
          return RE_MAT_TYPE_CODE_MATTE;
        case MatMetal:
          return RE_MAT_TYPE_CODE_METAL;
        case MatMirror:
          return RE_MAT_TYPE_CODE_MIRROR;
        case MatMix:
          return RE_MAT_TYPE_CODE_MIX;
        case MatNull:
          return RE_MAT_TYPE_CODE_NULL;
        case MatSkin:
          return RE_MAT_TYPE_CODE_SKIN;
        case MatVelvet:
          return RE_MAT_TYPE_CODE_VELVET;
        case MatWater:
          return RE_MAT_TYPE_CODE_WATER;
        case MatLight:
          return RE_MAT_TYPE_CODE_LIGHT;
        case MatCloth:
          return RE_MAT_TYPE_CODE_CLOTH;
        case MatUndefined:
          return RE_MAT_TYPE_CODE_UNDEFINED;
      }
    }
    return RE_MAT_TYPE_CODE_UNDEFINED;    
  }
  //! Returns an integer representing the number of objects in the database.  
  inline int getNumObjects() {
    return objects.count();
  }

  inline ReGeometryObjectDictionary getObjects() const {
    return objects;
  }

  //! Deprecated
  inline QStringList getObjectNames() {
    return objects.keys();
  }

  inline QStringList getObjectIDs() {
    return objects.keys();
  }

  inline const char* getObjectName( const QString& objectID ) const {
    if (objects.contains(objectID)) {
      return objects.value(objectID)->getName().toUtf8();
    }
    return "";
  }

  inline bool hasObject( const QString objectID ) const {
    return objects.contains(objectID);
  }

  inline ReGeometryObjectPtr getObject(const QString objectID) {
   return objects.value(objectID);
  }

  QString blankValueString;
  inline const QString& findObjectByName( const QString& objName ) {
    ReGeometryObjectIterator i(objects);
    while( i.hasNext() ) {
      i.next();
      auto obj = i.value();
      if (obj->getName() == objName) {
        return obj->getInternalName();
      }
    }
    return blankValueString;
  }

  inline ReGeometryObjectPtr getObjectAt( const quint16 position ) {
    QStringList keys = objects.keys();
    if ((keys.count()-1) < position) {
      return ReGeometryObjectPtr(NULL);
    }
    return objects[keys[position]];
  }

  //! Returns the number of materials associated with an object
  int getNumObjectMaterials( const QString objectID );

  inline void setObjects( ReGeometryObjectDictionary newDict ) {
    ReGeometryObjectIterator i(newDict);
    while(i.hasNext()) {
      i.next();
      objects[i.key()] = i.value();
    }
  }

  /**
   * Flags all objects as being ready to be deleted. This is used when the host-app
   * needs to refresh the catalog of objects discovered. It can happen that the host app
   * is not reporting all the events that affect the object catalog. For example, Poser 9
   * does not report renaming of parented objects. Because of this the catalog of objects
   * kept by Reality can become out of sync with the host app. To resync it we mark all the
   * objects to be deleted and then scan the host app catalog again. When an object is 
   * found it is checked against the Reality catalog. If it already exist the deletion 
   * flag is reset. If it's not found then the flag will remain and then next operation,
   * deleteOrphanedObjects(), will erase all the flagged objects.
   */
  void markObjectsForDeletion();

  /**
   * Removes the deletion glaf applied by markObjectsForDeletion() to an object 
   * Returns true if the object swas marked for deletion, false otherwise.
   */
  bool clearObjectDeletionFlag( const QString objectID );

  /**
   * Delete all the objects that have been marked for deletion. See markObjectsForDeletion()
   */
  void deleteOrphanedObjects();

  /**
    \return The number of materials in the database.  
   */
  int getNumMaterials();

  ReMaterialPtr getMaterial( const QString& objectName, 
                             const QString& materialName );

  /**
   Export the scene into one of the several formats supported. This method is 
   called to both export the scene to a renderer and to save the data 
   into the host app scene. For Poser we need to call this method with the Python
   exporter. For Studio we need to use the JSON exporter.
   Return:
     A QString containing the full list of materials exported in the target format
  */
  const QString exportScene( const QString& format, const int frameNo = 0 );

  /**
   * Returns a the exporter corresponding to a given format
   */
  ReBaseSceneExporter* getSceneExporter( const QString& format );

  inline QString objectNameAt( const quint16 index ) {
    return objects.keys()[index];
  }

  //! Add a camera to the scene
  void addCamera( const QVariantMap camData );

  void addCamera( const ReCameraPtr cam );

  /**
   * Restores a camera from the saved scene data. The dataStr parameter is a 
   * JSON-formatted objects saved with the <ReJSONCameraExporter> class.
   */
  void restoreCamera( const QString& dataStr );

  //! Update the camera data with a string of JSON-formatted
  //! information. 
  void updateCamera( const QString& cameraName, const QVariantMap& data );

  void renameCamera( const QString cameraID, const QString newName );

  inline ReCameraPtr getCamera( const QString camID ) {
    if (cameras.contains(camID)) {
      return cameras[camID];       
    }
    return ReCameraPtr();
  }

  inline ReCameraDictionary* getCameras() {
    return &cameras;
  }

  inline bool hasCamera( const QString& camID ) {
    return cameras.contains(camID);
  }

  inline int getNumCameras() {
    return cameras.count();
  }

  //! Removes a camera from the list of cameras hadnled by Reality
  void removeCamera( const QString camID );

  inline void removeAllCameras() {
    cameras.clear();
  }

  inline void selectCamera( const QString& cameraID) {
    if (cameras.contains(cameraID)) {
      selectedCamera = cameraID;
    }
  }

  inline const QString& getSelectedCameraID() const {
    return selectedCamera;
  }

  inline ReCameraPtr getSelectedCamera() const {
    return cameras[selectedCamera];
  }

  /*
  * Lights
  */
   
  //! Clear the light list. Usually invoked when lights are 
  //! deleted in Poser. There is bug in Poser's notification of the
  //! events that causes the delete events to be sent *after* the add events
  //! See the notes in __init__.py
  //! The parameter determines if meshlights should be removed as well.

  void removeAllLights( const bool removeMeshLights = false );

  //! Make a copy of the lights so that they can be restored after the call
  //! to removeAllLights. This is to work around the bug in Poser 9 that 
  //! causes delet events being sent after the add events.
  //! The logic is to backup the lights, remove them and then wait for
  //! Poser's add events for the lights. Once those are processed we can 
  //! restore the light data
  void backupLights( const bool removeMeshLights = false );

  void restoreLightsFromBackup();

  // Method: getNumLights
  //   Returns the number of lights stored in the scene.
  int getNumLights();
  
  // Method: saveLight
  //   Adds a new light to the catalog, or updates the light if
  //   it was already present.
  void saveLight( const QString lightID, ReLightPtr light );

  // // Method: addLight
  // //   Adds a new light based on the JSON data passed in the <lightData> 
  // //   parameter. This method is the main mean of adding a light from Poser.
  // void addLight( const QString lightID, const char* lightData );

  // Method: addLight
  //   Adds a new light based on the data passed in the <data> 
  //   parameter. This method is the main mean of adding a light from a C++ client.
  // Values recognized in data map:
  //   - type
  //   - name
  //   - ID
  //   - on
  //   - intensity
  //   - angle
  //   - color,color
  //   - color,map
  //   - colorModel
  //   - IBLMapFormat
  void addLight( const QString lightID, const QVariantMap& data ); 

  inline const bool hasLight( const QString lightID ) const {
    return lights.contains(lightID);
  }

  /**
   Update the data information about a light. It's meant to be called when 
   a light changes in the hots-app.
   */
  void updateLight( const QString& lightID, const QVariantMap lightData );

  //! Deletes a light and removes it from the catalog.
  //! \param lightID The unique ID of the light
  //! \param dontDeleteObjects If set to true it will not delete the object 
  //!                          that is connected to a mesh light. This is
  //!                          necessary when it's the object itself that is
  //!                          deleting the light, as part of its destructor
  void deleteLight( const QString lightID, const bool deleteObjects = true );

  //! Unlink a mesh/area light from the object that links to it.
  //! With host-native area lights we need to delete the object that holds
  //! the ReLightMaterial. This is because hots area lights are deleted 
  //! as lights, instead of being deletes as objects, like in the case of
  //! the Reality mesh light. With a Mesh Light we delete the object, which in
  //! turn will delete the associated light.
  //! To do the deletion we need to unlink the light first. This is because 
  //! lights that are linked to an object will be deleted automatically when
  //! the object is deleted. This can cause a dead-lock where deleting an 
  //! area light will cause the linked object to be deleted which in turn will 
  //! cause the linked light to be deleted and therefore getting into a vicious
  //! circle. 
  //! Now, because of the way C++ handles destructors, the vicious circle is
  //! actually not going to happen, always, but that is an implementation-specific
  //! side effect which is unsafe and obscure to rely on.
  //!
  //! The clean, correct way of doing the deletion is to unlink the light from
  //! the object and idenfify the object at the same time. 
  //! Then we delete the light and finally we delete the object.
  //!
  //! This method does the unlinking and returns the ID of the object, if found.
  QString unlinkLightFromObject( const QString& lightID );

  //! Rename a light
  void renameLight( const QString lightID, const QString& newName );

  //! Returns a pointer to the selected light
  ReLightPtr getLight( const QString lightID );

  //! Returns the dictionary of lights in the scene
  ReLightDictionary getLights();

  //! Changes the type of an existing light
  void setLightType( const QString& lightID, const ReLightType newType );

  //! Convenience method to simply set the gain of a light
  void setLightIntensity( const QString& lightID, const float gain );

  //! Sets the transfor matrix of a light. Used to store the light
  //! coordinates in the 3D scene when they change in the host app.
  void setLightMatrix( const QString lightName, const QVariantList& tm );
  //!  Overloaded version
  void setLightMatrix( const QString lightName, const ReMatrix& tm );

  //! Sets the vertical axis rotation angle for the IBL background
  void setIBLRotation( float angle );

  //! Set the IBL information from a sIBL file. See hdrlabs.com
  void setIBLImage( const QString sIBLFile );
  
  //! Method that returns the name of the file that should be used as a
  //! map for the IBL Preview Sphere. This file is either set manually
  //! or configured automatically from a sIBL file
  QString getIBLPreviewMap();
  
  //! Sends a notification to the GUI via message passing.
  //! Currently sending only the "lightAdded" message but open for exytensions
  void notifyGUI( const QString msg, const QString id );
   
  /*
   * Volumes
   */
  void saveVolume( ReVolumePtr vol );

  ReVolumePtr getVolume( const QString name );
  ReVolumeDictionary getVolumes();

  bool renameVolume( const QString oldName, const QString newName );
  bool removeVolume( const QString name );
  bool volumeExists( const QString name ) const;

  /*
   * Geometry export
   */
  void newGeometryBuffer( const QString& bufferName,
                          const int numVertices, 
                          const int numTriangles, 
                          const bool hasUVs );

  float* getGeometryVertexBuffer();
  float* getGeometryNormalBuffer();
  ReUVPoint* getGeometryUVPointBuffer();
  int* getGeometryFaceBuffer();

  QString& exportMaterial( const QString& materialName, 
                           const QString& objectName, 
                           const QString& shapeName,
                           HostAppID scale );

  //! Add an instance of an object to the scene.
  //! \param objectName The ID of the instance
  //! \param transform A transform matrix to apply to the instance
  void renderSceneExportInstance( const QString& objectName, 
                                  const QVariantMap& transform,
                                  const HostAppID scale );

  void renderSceneExportInstance( const QString& objectName, 
                                  const ReMatrix& transform,
                                  const HostAppID scale );

  //! Erases everything and returns the scene to its initial state
  void newScene();

  //! Starts the process of rendering the scene
  //! \param sceneFileName  The filename and to use to store the
  //!                       scene. If left blank Reality will use the file 
  //!                       name specified in the scene properties.
  void renderSceneStart( const QString& sceneFileName = "", 
                         const unsigned int frameNo = 0 );
  void renderSceneExportMaterial( const QString& matName,  
                                  const QString& objName,
                                  const QString& shapeName,
                                  const HostAppID scale );
  void renderSceneFinish( bool runRenderer );

  //! Injects a string into the current scene buffer. This is used for
  //! writing special data when exporting strand hair
  void renderSceneIncludeFileCustomData( const QString str );

  void renderSceneObjectBegin( const QString objName );
  void renderSceneObjectEnd( const QString objName );

  inline void setAnimationLimits( const int startFrame, const int endFrame, const int fps ) {
    animationStartFrame = startFrame;
    animationEndFrame = endFrame;
    animationFPS = fps;
  }

  inline void getAnimationLimits( int& startFrame, int& endFrame, int& fps ) const {
    startFrame = animationStartFrame;
    endFrame = animationEndFrame;
    fps = animationFPS;
  }
};

}

#endif
