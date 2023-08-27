/*
 * Copyright © 2012 Pret-a-3D/Paolo Ciccone. All rights reserved.
 */
 
#ifndef REDEFS_H
#define REDEFS_H

#include "reality_lib_export.h"

 //! This macro makes it easy to mark function parameters that are not used
//! so that the compiler doesn't issue a warning
#define UNUSED(param)

//! Our version of unsigned int
#define RE_UINT  unsigned int

//! Our version of unsigned short int
#define RE_USINT unsigned short int

#define REALITY_LIGHT_PREFIX          "RealityLight"
//! The name of the material used for the back of the Mesh light
#define REALITY_LIGHT_BACK_MATERIAL   "ReL_Back"
//! The name of the material used for the handle of the Mesh light
#define REALITY_LIGHT_HANDLE_MATERIAL "ReL_Handle"
#define REALITY_WATER_MAT             "RealityWater"

//! The subdirectory where the embedded LuxRender is installed 
#define LUX_SUBDIR "lux"

//! The name of the object for the IBL preview sphere
#define REALITY_IBL_SPHERE            "RealityIBLSphere" 
//! The name of the material used for the IBL Preview sphere
#define REALITY_IBL_SPHERE_MAT        "RealityIBLSphere" 

// The Poser version IPC port
#define IPC_PORT 9449
// The DS version IPC port
#define IPC_DS_PORT 9455
// The Reality Pro IPC port
#define IPC_PRO_PORT 9465

#define IPC_SERVER_ADDRESS "tcp://*:%1"
#define IPC_CLIENT_ADDRESS "tcp://%1:%2"

// The Poser PUBlish/SUBscribe port
#define IPC_PUBSUB_PORT 9450
// The DS PUBlish/SUBscribe port
#define IPC_DS_PUBSUB_PORT 9456
// The Reality Pro PUBlish/SUBscribe port
#define IPC_PRO_PUBSUB_PORT 9466
#define IPC_PUBLISHER  "tcp://*:%1"

//! CLIPBOARD identifier that signs that the data in the paste buffer is ours

// CPD => Clipboard Data
#define RE_CLIPBOARD_ID "Reality3CD" 

// Configuration keys
//! Deprecated
#define RE_CFG_LUX_PATH                 "LuxPath"
//! Deprecated
#define RE_CFG_LUX_VERSION              "LuxVersion"
//! Deprecated
#define RE_CFG_LUX_TIMESTAMP            "LuxTimestamp"

#define RE_CFG_VERSION                  "Version"
#define RE_CFG_MATERIAL_SPLITTER        "MaterialSplitter"
#define RE_CFG_CONVERT_INFINITE         "ConvertInfiniteLights"
#define RE_CFG_IGNORE_INFINITE          "IgnoreInfiniteLights"
#define RE_CFG_USE_REAL_SPOTS           "UseRealSpots"
#define RE_CFG_LUX_DISPLAY_REFRESH      "LuxDisplayRefresh"
#define RE_CFG_LUX_WRITE_INTERVAL       "LuxWriteInterval"
#define RE_CFG_OVERWRITE_WARNING        "OverwriteWarning"
#define RE_CFG_MAT_PREVIEW_CACHE_SIZE   "MatPreviewCacheSize"
// #define RE_CFG_USE_GPU                  "UseGPU"
#define RE_CFG_OCL_GROUP_SIZE           "OpenCLWorkgroupSize"
#define RE_CFG_DEFAULT_SCENE_LOCATION   "DefaultSceneLocation"
#define RE_CFG_LIBRARY_PATHS            "libraryPaths"
#define RE_CFG_ORGANIZATION_NAME        "Pret-a-3D"
#define RE_CFG_ORGANIZATION_NAME_SIMPLE "preta3d"
#define RE_CFG_DOMAIN_NAME              "preta3d.com"
#define RE_CFG_APPLICATION_NAME         "Reality"
#define RE_CFG_REALITY_DS_LOCATION      "Reality_DS_Location"
// #define RE_CFG_NUM_GPUS                 "numGPUs"
#define RE_CFG_LAST_BROWSED_DIR         "lastBrowsedDir"
#define RE_CFG_LAST_UPDATE_CHECK        "lastUpdateCheck"
// #define RE_CFG_USE_NATIVE_UI            "UseNativeLook"
#define RE_CFG_KEEP_UI_RESPONSIVE       "KeepUiResponsive"

#define RE_CFG_DEFAULT_SCENE_NAME        "reality_scene.lxs"
#define RE_CFG_DEFAULT_IMAGE_NAME        "reality_scene.png"
#define RE_CFG_SCENE_FILE_SUFFIX         "lxs"
#define RE_CFG_IMAGE_FILE_DEFAULT_SUFFIX "png"

#define RE_HOST_RENDERER_CODE_IRAY       "ir"
#define RE_HOST_RENDERER_CODE_3DELIGHT   "3dl"
#define RE_HOST_RENDERER_CODE_FIREFLY    "ff"
#define RE_HOST_RENDERER_CODE_SUPERFLY   "sf"
 
//!Abbreaviated names of material types 
#define RE_MAT_TYPE_CODE_CLOTH     "CLO"
#define RE_MAT_TYPE_CODE_GLASS     "GLS"
#define RE_MAT_TYPE_CODE_GLOSSY    "GLO"
#define RE_MAT_TYPE_CODE_HAIR      "HAR"
#define RE_MAT_TYPE_CODE_LIGHT     "LGT"
#define RE_MAT_TYPE_CODE_MATTE     "MAT"
#define RE_MAT_TYPE_CODE_METAL     "MTL"
#define RE_MAT_TYPE_CODE_MIRROR    "MIR"
#define RE_MAT_TYPE_CODE_MIX       "MIX"
#define RE_MAT_TYPE_CODE_NULL      "NUL"
#define RE_MAT_TYPE_CODE_SKIN      "SKN"
#define RE_MAT_TYPE_CODE_UNDEFINED "N/A"
#define RE_MAT_TYPE_CODE_VELVET    "VLV"
#define RE_MAT_TYPE_CODE_WATER     "WTR"

//! Defines the amount of cores left to the UI. We leave 10% out of control
//! of Lux
#define RE_CFG_PERCENT_CORES_FOR_RENDERING 0.9

//! The location of the Poser content
#define RE_CFG_POSER_CONTENT_DIR        "PS_ContentDir"
//! The location of the Studio content
#define RE_CFG_STUDIO_CONTENT_DIR       "DS_ContentDir"

//! The name to use for ACSEL saved shaders
#define RE_CFG_AUTHOR_NAME              "AuthorName"
//! Used to issue a warning the first time the user changes "Max Samples"
//! It has been noticed that some Poser user change that parameter 
//! thiniking that it can improve rendering. They don't know what it does
//! and then complain that the image is grainy.
#define RE_CFG_MAX_SAMPLES_FIRST_TIME   "MaxSamplesFirstTime"

#define RE_CFG_DAYS_BETWEEN_UPDATE_CHECKS 6
#define RE_CFG_DATE_FORMAT              "yyyy-M-dd"

//! Wheter of not we need to remind the user about the effect of flagging a 
//! shader set as "generic"
#define RE_CFG_GENERIC_FLAG_REMINDER   "GenericFlagReminder"

//! nVidia GPUs need this group size to work correctly
#define RE_CFG_NVIDIA_OCL_GROUP_SIZE  64

//! ID of the Reality Data Element stored inside a DAZ Studio node 
#define RE_DS_NODE_GUID "RealityObjID"

//! For Infinite Lights converted to meshes we need to set them at a 
//! given hight. This is the value used, in meters.
#define RE_INFINITE_LIGHT_HEIGHT 4.0 

// Data separator for Materials and Textures in the 
// textual serializer
#define RE_DATA_SEPARATOR '|'

// Used to start a texture in the scene data
#define RE_TEXTURE_CODE   "TX"

// The prefix used in the section of the stored data that
// denotes the start of the Geometry Modifieries  
#define RE_GEOMETRY_MODIFIERS_CODE "GM"

// The prefix used in the section of the stored data that
// denotes the start of the Alpha Channel data
#define RE_ALPHACHANNEL_CODE  "AC"

// Maximum level of brightness allowed for materials
// being converted
#define RE_MAX_SPECULAR_BRIGHTNESS 25

// Minimum amount of loop subdivisions for a material with displacement map
#define RE_MIN_LOOP_SUBD 1

//! IBL lights in Lux are so faint that we need to boost the gain 
//! quite a bit. The following is an emprically-set multipler used to
//! balance the IBL lights automatically
#define IBL_GAIN_MULTIPLIER 500

const double PI = 3.141592653589793238462;

//! The Mime type for textures copied to the clipboard
#define RE_TEX_CLIPBOARD_MIME_TYPE "application/reality-texture"

/*
 Behavior modification
 */

// Number of milliseconds of inactivity to wait before updating the UI
#define RE_CFG_UPDATE_INACTIVITY 600

#define RE_AIR_VOLUME "air"

//! Default IOR value for the human eye. It's a bit higher than
//! the physical one because we want to create enhanced reflections
const float RE_MAT_CFG_EYE_IOR = 1.5f;

// When a material come through from the host-app as glossy but with 
// a glossinness level of 0 we set it to have a default glossiness
// at the value listed below 
#define RE_MIN_GLOSSINESS 1200

// File extension used for the Lux include files
#define LUX_INCLUDE_EXTENSION "lxi"

#include <QColor>
//! Realistic White color
const QColor RE_WHITE_COLOR(240, 240, 240);

//! Pure white
const QColor RE_PURE_WHITE_COLOR(255, 255, 255);

//! Medium gray
const QColor RE_MEDIUM_GRAY_COLOR(127, 127, 127);

//! Realistic black color
const QColor RE_BLACK_COLOR(15, 15, 15);

//! The maximum size of the LogFile before being deleted. 12 Mb
#define RE_LOG_FILE_MAX_SIZE 12582912
 
#include <QSharedPointer>

namespace Reality {

class ReGeometryObject;
 
typedef QSharedPointer<ReGeometryObject> ReGeometryObjectPtr;

//! List of what host applications we can work with. Use to 
//! identify which host we are connected to.
enum HostAppID {
  Poser,       // 0   
  DAZStudio,   // 1
  Blender,     // 2
  ZBrush,      // 3
  Modo,        // 4
  RealityPro, // 5 Reality stand-alone
  Unknown      // 6
};

/**
 * Material types
 */
enum ReMaterialType {
  MatGlass,
  MatGlossy,
  MatHair, //*
  MatMatte,
  MatMetal,  // 4 
  MatMirror, 
  MatMix, //*
  MatNull,   
  MatSkin,  // 8
  MatVelvet, 
  MatWater,
  MatLight,
  MatCloth,
  MatUndefined
};

/**
 Enum: TextureType

 Used to identify the texture in the stream used for serialize/deserialize.
 This type is also an easy RTTI ID for other operations required by the UI

 WARNING. When updating this remember to change also the table in:
    - ReTextureAvatar.cpp
 */
enum ReTextureType {
  TexImageMap,
  TexConstant,    
  TexColorMath,   
  TexMath,
  TexBand,
  TexBricks,      
  TexCheckers,
  TexClouds,
  TexFBM,
  TexFresnelColor,
  TexMarble,
  TexMix,
  TexMultiMix,
  TexDistortedNoise,
  TexWood,
  TexInvertMap,
  TexGrayscale,
  TexUndefined
};

// Used to store the data type (color, numeric, fresnel) of each texture
// This table is used to filter what textures can be created for a given
// channel. 
// Values:
// 0x01 => color
// 0x02 => numeric
// 0x04 => fresnel
// 
// Of course these can be combined using bitwise operators
// For example 0x03 is a texture that can be either of type color or numeric
REALITY_LIB_EXPORT extern quint8 ReTextureDataTypes[TexUndefined+1];

// Bitmasks for the ReTextureDataTypes table and for use with the Texture Preview/Avatar
#define RE_TEX_DATATYPE_COLOR   0x01
#define RE_TEX_DATATYPE_NUMERIC 0x02
#define RE_TEX_DATATYPE_FRESNEL 0x04
#define RE_TEX_DATATYPE_ALL     0x07
#define RE_TEX_DATATYPE_COLOR_NUMERIC 0x03

//! OpenCL device selectors for the bitmap value. See the description of <ReSceneData::OCLDeviceNos>.
#define OCL_DEVICE_1_SELECTED 0x01
#define OCL_DEVICE_2_SELECTED 0x02
#define OCL_DEVICE_3_SELECTED 0x04


//! Textual names of the Texture types that can be used to represent those types
//! in the Reality UI.
REALITY_LIB_EXPORT extern QString ReTextureTypesForUI[TexUndefined+1];

//! Supported renderers
enum ReRenderers {
  LuxRender,
  SLG
};

enum GeometryFileFormat {
  LuxNative,
  BinaryPLY,
  TextPLY
};

enum LuxToneMapper {
  Reinhard,
  Linear,
  Autolinear
};

enum ExportImageFileFormat {
  PNG,
  EXR,
  TGA
};

enum LuxAccelerator {
  tabreckdtree,
  qbvh,
  sqbvh
};

//! LuxRender samplers available
enum RenderSampler {
  metropolis,
  sobol
};

enum RGBChannel {
  RGB_Red,
  RGB_Green,
  RGB_Blue,
  RGB_Mean
};

enum OSType {
  MAC_OS,
  WINDOWS,
  LINUX,
  iOS
};

enum ReSurfaceIntegratorType {
  SI_BIDIR,
  SI_PATH,
  SI_PATH_GPU,
  SI_SPPM,
  SI_EXPM,
  SI_DIRECT
};

//! Sizes for the texture collection
enum ReTextureSize {
  T_256, 
  T_512, 
  T_1K, 
  T_2K, 
  T_ORIGINAL, 
};

enum LUX_LOG_LEVEL {
  LUX_WARNINGS    = 101,
  LUX_INFORMATION,
  LUX_DEBUG,
  LUX_ERRORS
};


typedef float ReVectorF[3];
// typedef int ReTriangle[3];
typedef float ReUVPoint[2];

struct ReVector {
  float X,Y,Z;
};

} // namespace

#endif
