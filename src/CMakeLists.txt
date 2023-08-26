# DAZ Studio 4 Plugins on Windows must link against MSVC++ 10 runtime and export symbols getSDKVersion and getPluginDefinition
# DAZ Studio 4 Plugins on OS X must link against libstdc++.6.dylib and export ONLY symbols getSDKVersion and getPluginDefinition
# Contrary to DAZ SDK Documention it appears linking to libc++ works?


# Configure CMAKE
cmake_minimum_required (VERSION 3.25)
set (CMAKE_POLICY_DEFAULT_CMP0077 NEW)
list (APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

include (FetchContent)
include (CheckIPOSupported)

# Configure pre-project settings
set (CMAKE_CXX_STANDARD 11)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# DAZ Studio 4 is available for both 32 bit and 64 bit architectures on the Intel platform
	set (CMAKE_OSX_ARCHITECTURES "x86_64;i386" CACHE STRING "Build architectures for OS X")
	# DAZ Studio 4 runs on OS X 10.7+
	set (CMAKE_OSX_DEPLOYMENT_TARGET 10.7 CACHE STRING "Minimum OS X version to target for deployment")
endif()

# Setup Project
project (Reality VERSION 4.3.3 DESCRIPTION "Reality for DAZ Studio and Poser" HOMEPAGE_URL "https://github.com/Omniflux/reality/" LANGUAGES C CXX)

# Ensure tweak version has a value for use in generated files
if (NOT Reality_VERSION_TWEAK)
	set (Reality_VERSION_TWEAK 0)
endif()

# Enable IPO
check_ipo_supported (RESULT ipo_supported)
if (ipo_supported)
  set (CMAKE_POLICY_DEFAULT_CMP0069 NEW)
  set (CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  set (CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
endif ()

# MSVC2010 does not support C++11 but does support enum qualified names, so silence the warning
add_compile_options ($<$<AND:$<COMPILE_LANG_AND_ID:CXX,MSVC>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,17>>:/wd4482>)

# libstdc++ is selected by default when OSX_DEPLOYMENT_TARGET < 10.9 but we want libc++ instead
add_compile_options ($<$<AND:$<PLATFORM_ID:Darwin>,$<COMPILE_LANGUAGE:CXX>>:-stdlib=libc++>)

# Trim Windows headers by default
add_compile_definitions ($<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>)

# Find dependencies
set (Boost_USE_STATIC_LIBS ON)

FetchContent_Declare (qjson
	GIT_REPOSITORY    https://github.com/flavio/qjson.git
	GIT_TAG           4920592171ccf83029f004880ad02aa34a60c0ab
)

FetchContent_Declare (SQLiteCpp
	GIT_REPOSITORY    https://github.com/SRombauts/SQLiteCpp.git
	GIT_TAG           2.5.0
)

FetchContent_Declare (ZeroMQ
	GIT_REPOSITORY    https://github.com/zeromq/libzmq.git
	GIT_TAG           ec013f3a17beaa475c18e8cf5e93970800e2f94a
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare (cppzmq
	GIT_REPOSITORY    https://github.com/zeromq/cppzmq.git
	GIT_TAG           v4.10.0
	OVERRIDE_FIND_PACKAGE
)

find_package (Boost 1.67.0 REQUIRED COMPONENTS atomic program_options python27)
find_package (Qt4 4.8.7 REQUIRED COMPONENTS QtGui QtNetwork QtWebKit)
find_package (Python2 2.7.18 REQUIRED COMPONENTS Development.Module)
find_package (OpenCL REQUIRED)
find_package (DAZStudioSDK 4.5 REQUIRED COMPONENTS Plugin)

if (DAZStudioSDK_TOOLKIT_INCOMPATIBLE)
	message (SEND_ERROR)
endif()

# QJSON
block()
	set (CMAKE_POLICY_DEFAULT_CMP0026 OLD)
	set (BUILD_SHARED_LIBS OFF)
	set (LINK_SHARED_QT ON)
	set (QT4_BUILD ON)
	FetchContent_MakeAvailable (qjson)
endblock()
target_include_directories (qjson INTERFACE $<BUILD_INTERFACE:${qjson_SOURCE_DIR}/include>)
set_property (DIRECTORY ${qjson_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)

# SQLiteCpp
# Note - Warning that BLOB storage seems to be corrupted is incorrect
# See: https://github.com/SRombauts/SQLiteCpp/issues/189
block()
	set (SQLITE_ENABLE_ASSERT_HANDLER ON)
	FetchContent_MakeAvailable (SQLiteCpp)
endblock()
target_link_libraries (SQLiteCpp PRIVATE sqlite3)
get_target_property (INC_DIR sqlite3 INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties (sqlite3 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${INC_DIR}> SKIP_AUTOGEN ON)
install (TARGETS sqlite3 EXPORT SQLiteCppTargets)
set_property (DIRECTORY ${SQLiteCpp_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)

# ZeroMQ
block()
	set (CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON)
	set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
	set (BUILD_TESTS OFF)
	FetchContent_MakeAvailable (ZeroMQ)
endblock()
set_property (DIRECTORY ${ZeroMQ_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)

# cppzmq
block()
	set (CPPZMQ_BUILD_TESTS OFF)
	FetchContent_MakeAvailable (cppzmq)
endblock()


# Enable Qt Auto MOC/UIC/RCC
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTORCC ON)


###############################################################################
###############################################################################
###############################################################################


# Define file groups

set (SRC_GRP_VERSION
	core/ReVersion.cpp.in
	core/ReVersion.h
	${PROJECT_BINARY_DIR}/autogen/ReVersion.cpp
	${PROJECT_BINARY_DIR}/BuildNumber_autogen/build_number.h
)

set (SRC_GROUP_QT_UI
	gui/uiFiles/imageMapManager.ui
	gui/uiFiles/meAlpha.ui
	gui/uiFiles/meCloth.ui
	gui/uiFiles/meGlass.ui
	gui/uiFiles/meGlossy.ui
	gui/uiFiles/meMatte.ui
	gui/uiFiles/meMirror.ui
	gui/uiFiles/meMatVolumes.ui
	gui/uiFiles/meModifiers.ui
	gui/uiFiles/meSkin.ui
	gui/uiFiles/meMetal.ui
	gui/uiFiles/meWater.ui
	gui/uiFiles/meVelvet.ui
	gui/uiFiles/meVolumes.ui
	gui/uiFiles/realitypanel.ui
	gui/uiFiles/reBreadCrumbs.ui
	gui/uiFiles/reColorPicker.ui
	gui/uiFiles/reMainWindow.ui
	gui/uiFiles/reTextureEditor.ui
	gui/uiFiles/reLightEditor.ui
	gui/uiFiles/teClouds.ui
	gui/uiFiles/teMarble.ui
	gui/uiFiles/teWood.ui
	gui/uiFiles/teBand.ui
	gui/uiFiles/teDistortedNoise.ui
	gui/uiFiles/teBricks.ui
	gui/uiFiles/teColor.ui
	gui/uiFiles/teColorMath.ui
	gui/uiFiles/teCheckers.ui
	gui/uiFiles/teFBM.ui
	gui/uiFiles/teMath.ui
	gui/uiFiles/teImageMap.ui
	gui/uiFiles/teMix.ui
	gui/uiFiles/teGrayscale.ui
	gui/uiFiles/te3DMapping.ui
	gui/uiFiles/tex2DMapping.ui
	gui/uiFiles/textureAvatar.ui
	gui/uiFiles/reOutputOptions.ui
	gui/uiFiles/rePresetSelector.ui
	gui/uiFiles/reCameraEditor.ui
	gui/uiFiles/reTextureSelector.ui
	gui/uiFiles/reAbout.ui
	gui/uiFiles/reWatermark.ui
	gui/uiFiles/reExportProgress.ui
	gui/uiFiles/ReSlider.ui
	gui/uiFiles/ReUpdateNotification.ui
	gui/uiFiles/reAcselSave.ui
	gui/uiFiles/reUniShaderSave.ui
	gui/uiFiles/reAcselMgr.ui
	gui/uiFiles/reCommandLineOptions.ui
	gui/uiFiles/reAcselOverwriteConfirm.ui
	gui/uiFiles/reShaderSetSelector.ui
	gui/uiFiles/reUniversalShaderSelector.ui
	gui/uiFiles/reUniversalShaderWdg.ui
	gui/uiFiles/reSyncMats.ui
	gui/uiFiles/reConfirmationDialog.ui
	gui/uiFiles/reCssEditor.ui
)

# We need to have separate groups of sources for the texture and material 
# editors because they need to be included in the RealityWidgets library,
# the plugin library for Qt Creator, while the rest of the gui must not be
# incuded there.
set (HEADER_GROUP_TEXTURE_EDITORS
	data/ReTextureChannelDataModel.h
	gui/RealityUI/ReAbstractTextureEditor.h
	gui/RealityUI/ReColorPicker.h
	gui/RealityUI/ReImageMapManager.h
	gui/RealityUI/ReTextureAvatar.h
	gui/RealityUI/Re2DTextureMapping.h
	gui/RealityUI/ReBreadCrumbs.h
	gui/RealityUI/ReCloudsTextureEditor.h
	gui/RealityUI/ReDistortedNoiseTextureEditor.h
	gui/RealityUI/ReBricksTextureEditor.h
	gui/RealityUI/ReBandTextureEditor.h
	gui/RealityUI/ReMarbleTextureEditor.h
	gui/RealityUI/ReWoodTextureEditor.h
	gui/RealityUI/ReCheckersTextureEditor.h
	gui/RealityUI/ReFBMTextureEditor.h
	gui/RealityUI/ReMathTextureEditor.h
	gui/RealityUI/ReColorMathTextureEditor.h
	gui/RealityUI/ReColorTextureEditor.h
	gui/RealityUI/ReImageMapTextureEditor.h
	gui/RealityUI/ReMixTextureEditor.h
	gui/RealityUI/ReGrayscaleTextureEditor.h
	gui/RealityUI/Re3DMappingEditor.h
	gui/RealityUI/ReTextureEditor.h
	gui/RealityUI/ReTextureEditorAdapter.h
)

set (SRC_GROUP_TEXTURE_EDITORS
	gui/RealityUI/ReColorPicker.cpp
	gui/RealityUI/ReCloudsTextureEditor.cpp
	gui/RealityUI/ReDistortedNoiseTextureEditor.cpp
	gui/RealityUI/ReBandTextureEditor.cpp
	gui/RealityUI/ReBricksTextureEditor.cpp
	gui/RealityUI/ReMarbleTextureEditor.cpp
	gui/RealityUI/ReWoodTextureEditor.cpp
	gui/RealityUI/ReCheckersTextureEditor.cpp
	gui/RealityUI/ReFBMTextureEditor.cpp
	gui/RealityUI/ReMathTextureEditor.cpp
	gui/RealityUI/ReColorMathTextureEditor.cpp
	gui/RealityUI/ReColorTextureEditor.cpp
	gui/RealityUI/ReImageMapTextureEditor.cpp
	gui/RealityUI/ReMixTextureEditor.cpp
	gui/RealityUI/ReGrayscaleTextureEditor.cpp
	gui/RealityUI/ReTextureEditor.cpp
	gui/RealityUI/RePresetSelector.cpp
	gui/RealityUI/Re3DMappingEditor.cpp
	gui/RealityUI/ReTextureAvatar.cpp
	gui/RealityUI/ReImageMapManager.cpp
	gui/RealityUI/Re2DTextureMapping.cpp
	gui/RealityUI/ReBreadCrumbs.cpp
	data/ReTextureChannelDataModel.cpp
)
source_group ("Texture Editors" FILES ${HEADER_GROUP_TEXTURE_EDITORS} ${SRC_GROUP_TEXTURE_EDITORS})

set (HEADER_GROUP_MATERIAL_EDITORS
	gui/RealityUI/MaterialEditors/ReMaterialEditor.h
	gui/RealityUI/MaterialEditors/ReAlphaChannelEditor.h
	gui/RealityUI/MaterialEditors/ReGlassEditor.h
	gui/RealityUI/MaterialEditors/ReGlossyEditor.h
	gui/RealityUI/MaterialEditors/ReClothEditor.h
	gui/RealityUI/MaterialEditors/ReMatteEditor.h
	gui/RealityUI/MaterialEditors/ReMatVolumesEditor.h
	gui/RealityUI/MaterialEditors/ReMetalEditor.h
	gui/RealityUI/MaterialEditors/ReMirrorEditor.h
	gui/RealityUI/MaterialEditors/ReModifiers.h
	gui/RealityUI/MaterialEditors/ReSkinEditor.h
	gui/RealityUI/MaterialEditors/ReVelvetEditor.h
	gui/RealityUI/MaterialEditors/ReVolumesEditor.h
	gui/RealityUI/MaterialEditors/ReWaterEditor.h
)

set (SRC_GROUP_MATERIAL_EDITORS
	gui/RealityUI/ReTimedEditor.cpp
	gui/RealityUI/MaterialEditors/ReMaterialEditor.cpp
	gui/RealityUI/MaterialEditors/ReAlphaChannelEditor.cpp
	gui/RealityUI/MaterialEditors/ReGlassEditor.cpp
	gui/RealityUI/MaterialEditors/ReGlossyEditor.cpp
	gui/RealityUI/MaterialEditors/ReClothEditor.cpp
	gui/RealityUI/MaterialEditors/ReMatteEditor.cpp
	gui/RealityUI/MaterialEditors/ReMatVolumesEditor.cpp
	gui/RealityUI/MaterialEditors/ReMetalEditor.cpp
	gui/RealityUI/MaterialEditors/ReMirrorEditor.cpp
	gui/RealityUI/MaterialEditors/ReModifiers.cpp
	gui/RealityUI/MaterialEditors/ReSkinEditor.cpp
	gui/RealityUI/MaterialEditors/ReVelvetEditor.cpp
	gui/RealityUI/MaterialEditors/ReVolumesEditor.cpp
	gui/RealityUI/MaterialEditors/ReWaterEditor.cpp
	gui/RealityUI/ReExportProgressDialog.cpp
)
source_group ("Material Editors" FILES ${HEADER_GROUP_MATERIAL_EDITORS} ${SRC_GROUP_MATERIAL_EDITORS})

set (HEADER_GROUP_OTHER_EDITORS
	gui/RealityUI/ReLightEditor.h
	gui/RealityUI/ReCameraEditor.h
	gui/RealityUI/RePresetSelector.h
	gui/RealityUI/ReOutputOptions.h
	gui/RealityUI/ReSlider.h
)

set (SRC_GROUP_OTHER_EDITORS
	gui/RealityUI/ReOutputOptions.cpp
	gui/RealityUI/ReLightEditor.cpp
	gui/RealityUI/ReCameraEditor.cpp
)
source_group ("Other Editors" FILES ${HEADER_GROUP_OTHER_EDITORS} ${SRC_GROUP_OTHER_EDITORS})

set (HEADER_GROUP_GUI
	gui/actions/ReAction.h
	data/ReTextureChannelDataModel.h
	data/ReSceneDataModel.h
	gui/RealityPanel/RealityDataRelay.h
	gui/RealityUI/ReTextureSelector.h
	gui/ReMaterialPreview.h

	gui/RealityUI/ReExportProgressDialog.h
	gui/RealityUI/ReUpdateNotification.h
	gui/RealityPanel/realitypanel.h
	gui/ReMainWindow.h
	gui/RealityUI/ReAcselSave.h
	gui/RealityUI/ReAcselMgr.h
	gui/RealityUI/ReUniShaderSave.h
	gui/RealityUI/ReAcselShaderSetSelector.h
	gui/RealityUI/ReUniversalShaderWidget.h
	gui/RealityUI/ReUniversalShaderSelector.h
	gui/RealityUI/ReConfirmationDialog.h
	gui/RealityUI/ReCssEditor.h
)

set (SRC_GROUP_GUI
	gui/Reality.cpp

	data/ReSceneDataModel.cpp

	gui/RealityPanel/RealityDataRelay.cpp
	gui/ReMaterialPreview.cpp

	gui/actions/ReAction.cpp

	# GUI
	gui/RealityUI/ReTextureSelector.cpp
	gui/RealityUI/ReSlider.cpp
	gui/RealityUI/ReUpdateNotification.cpp
	gui/RealityUI/ReAcselSave.cpp
	gui/RealityUI/ReUniShaderSave.cpp
	gui/RealityUI/ReAcselMgr.cpp
	gui/RealityUI/ReAcselShaderSetSelector.cpp
	gui/RealityUI/ReUniversalShaderWidget.cpp
	gui/RealityUI/ReUniversalShaderSelector.cpp
	gui/RealityUI/ReConfirmationDialog.cpp

	gui/RealityPanel/realitypanel.cpp
	gui/ReMainWindow.cpp
	gui/RealityUI/ReCssEditor.cpp

	$<$<PLATFORM_ID:Windows>:gui/Reality.rc.in>
	$<$<PLATFORM_ID:Windows>:${PROJECT_BINARY_DIR}/autogen/Reality.rc>
)
source_group ("GUI" FILES ${HEADER_GROUP_GUI} ${SRC_GROUP_GUI})

set (HEADER_GROUP_QTDESIGNER
	gui/RealityUI/qtDesignerPlugins/Re2DTextureMappingPlugin.h
	gui/RealityUI/qtDesignerPlugins/Re3DMappingEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/RealityWidgets.h
	gui/RealityUI/qtDesignerPlugins/ReBandTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReBreadCrumbsPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReBricksTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReCameraEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReCheckersTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReClothEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReCloudsTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReColorMathTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReColorPickerPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReColorTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReDistortedNoiseTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReFBMTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReGlassEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReGlossyEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReGrayscaleTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReImageMapManagerPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReImageMapTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReMarbleTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReMathTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReMatteEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReMatVolumesEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReMetalEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReMirrorEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReMixTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReModifiersEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReOutputOptionsPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReSkinEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReSliderPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReTextureAvatarPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReVelvetEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReVolumesEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReWaterEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReWoodTextureEditorPlugin.h
	gui/RealityUI/qtDesignerPlugins/ReAlphaChannelEditorPlugin.h
	gui/RealityUI/ReTextureSelector.h
)

set (SRC_GROUP_QTDESIGNER
	gui/RealityUI/qtDesignerPlugins/Re2DTextureMappingPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/Re3DMappingEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/RealityWidgets.cpp
	gui/RealityUI/qtDesignerPlugins/ReBandTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReBreadCrumbsPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReBricksTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReCameraEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReCheckersTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReClothEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReCloudsTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReColorMathTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReColorPickerPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReColorTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReDistortedNoiseTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReFBMTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReGlassEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReGlossyEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReGrayscaleTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReImageMapManagerPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReImageMapTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReMarbleTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReMathTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReMatteEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReMatVolumesEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReMetalEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReMirrorEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReMixTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReModifiersEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReOutputOptionsPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReSkinEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReSliderPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReTextureAvatarPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReVelvetEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReVolumesEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReWaterEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReWoodTextureEditorPlugin.cpp
	gui/RealityUI/qtDesignerPlugins/ReAlphaChannelEditorPlugin.cpp
	gui/ReMaterialPreview.cpp
	gui/RealityUI/ReTextureSelector.cpp
	core/ReOpenCL.cpp
	gui/RealityUI/ReSlider.cpp
	gui/RealityPanel/RealityDataRelay.cpp
)
source_group ("QT Designer" FILES ${HEADER_GROUP_QTDESIGNER} ${SRC_GROUP_QTDESIGNER})

set (SRC_GROUP_TEXTURES_MATS
	# ReLight must be listed before ReSceneData or the host app will crash
	# at startup because ReScene instantiates a IBL light. If the ReLight.cpp
	# file is not listed in the right order then the lookup tables in it will
	# not be initialized at the time the IBL light is instantiate and the shared
	# library will crash potentially bringing down Poser or other hosts.
	data/ReLight.cpp
	data/ReSceneData.cpp
	data/ReTexture.cpp
	data/ReTextureCreator.cpp
	data/ReSceneResources.cpp
	data/textures/Re2DTexture.cpp
	data/textures/ReComplexTexture.cpp
	data/textures/ReBand.cpp
	data/textures/ReBricks.cpp
	data/textures/ReClouds.cpp
	data/textures/ReColorMath.cpp
	data/textures/ReConstant.cpp
	data/textures/ReDistortedNoise.cpp
	data/textures/ReFBM.cpp
	data/textures/ReMarble.cpp
	data/textures/ReWood.cpp
	data/textures/ReMix.cpp
	data/textures/ReFresnelColor.cpp
	data/textures/ReGrayscale.cpp
	data/textures/ReImageMap.cpp
	data/textures/ReMath.cpp
	data/textures/ReProceduralNoise.cpp
	data/ReMaterials.cpp
	data/ReMaterial.cpp
	data/ReCloth.cpp
	data/ReGlossy.cpp
	data/ReMix.cpp
	data/ReSkin.cpp
	data/ReMatte.cpp
	data/ReGlass.cpp
	data/ReWater.cpp
	data/ReMetal.cpp
	data/ReMirror.cpp
	data/ReVelvet.cpp
	data/ReLightMaterial.cpp
	data/ReVolumes.cpp
	data/ReDisplaceableMaterial.cpp
	data/ReAlphaChannelMaterial.cpp
	data/ReGeometryObject.cpp
	data/ReNodeConverter.cpp
	data/RealityBase.cpp
	core/LuxApi.cpp
)
source_group ("Textures and Mats" FILES ${SRC_GROUP_TEXTURES_MATS})

# The Sources for the Main Library used by the Reality UI
set (SRC_GROUP_LIBRARY
	data/ReDefs.cpp

	# Logging
	core/ReLogger.cpp
	# IPC and core features
	core/ReIPC.cpp
	# Important! The following must be listed before RealityBase.cpp.
	# The order of initialization is important.
	core/ReLuxRunner.cpp
	core/zeromqTools.cpp
	data/RealityRunner.cpp
	# Data handling
	data/ReTools.cpp
	data/ReCamera.cpp
	# ACTIONS
	gui/actions/ReAction.cpp

	core/ReRenderContext.cpp

	# Lux Format
	data/exporters/lux/ReLuxTextureExporter.cpp
	data/exporters/lux/ReImageMapExporter.cpp
	data/exporters/lux/ReLuxMaterialExporterFactory.cpp
	data/exporters/lux/ReLuxTextureExporterFactory.cpp
	data/exporters/lux/ReLightExporter.cpp
	data/exporters/lux/ReMixExporter.cpp
	data/exporters/lux/ReGlossyExporter.cpp
	data/exporters/lux/ReSkinExporter.cpp
	# Luxcore/SLG format
	data/exporters/luxcore/ReLuxcoreMaterialExporterFactory.cpp
	data/exporters/luxcore/ReGlossyExporter.cpp
	# JSON format
	data/exporters/json/ReJSONMaterialExporterFactory.cpp
	data/exporters/ReLuxSceneExporter.cpp
	data/exporters/ReSLGSceneExporter.cpp
	data/exporters/ReJSONSceneExporter.cpp
	# QVariantMap format exporters
	data/exporters/qt/ReQtTextureExporterFactory.cpp
	data/exporters/qt/ReQtMaterialExporterFactory.cpp
	data/exporters/ReQtSceneExporter.cpp

	# QVariantMap format importers
	data/importers/qt/ReQtTextureImporterFactory.cpp
	data/importers/qt/ReQtMaterialImporterFactory.cpp
	data/importers/qt/ReQtSceneImporter.cpp

	data/ReLuxGeometryExporter.cpp
	# PLY
	data/ply/rply.c
	# ACSEL
	data/ReAcsel.cpp
	# OpenCL
	core/ReOpenCL.cpp
)
source_group ("Reality Library" FILES ${SRC_GROUP_LIBRARY})

set (SRC_GROUP_POSER_ADDON
	Poser/RePythonTools.cpp
	Poser/Reality_ext.cpp
	Poser/RePoserInterface.cpp
	Poser/RePythonSceneExporter.cpp

	$<$<PLATFORM_ID:Windows>:Poser/Reality_PY.rc.in>
	$<$<PLATFORM_ID:Windows>:${PROJECT_BINARY_DIR}/autogen/Reality_PY.rc>
)
source_group ("Poser Addon" FILES ${SRC_GROUP_POSER_ADDON})

set (HEADER_GROUP_DAZ_STUDIO_PLUGIN
	Studio/Reality_DS.h
	Studio/RealityAction.h
	Studio/ReDSStorage.h
#	Studio/ReDSDebugTools.h
)

set (SRC_GROUP_DAZ_STUDIO_PLUGIN
	Studio/Reality_DS.cpp
	Studio/RealityAction.cpp
	Studio/ReDSMaterialConverter.cpp
	Studio/ReGUID.cpp
	Studio/ReDSStorage.cpp
	Studio/ReGeometryExporter.cpp
	Studio/ReDAZStudioEvent.cpp
#	Studio/ReDSDebugTools.cpp

	$<$<PLATFORM_ID:Windows>:Studio/Reality_DS.rc.in>
	$<$<PLATFORM_ID:Windows>:${PROJECT_BINARY_DIR}/autogen/Reality_DS.rc>
)
source_group ("DAZ Studio Plugin" FILES ${HEADER_GROUP_DAZ_STUDIO_PLUGIN} ${SRC_GROUP_DAZ_STUDIO_PLUGIN})

set (SRC_GROUP_RESOURCES
	gui/resources/RealityUI.qrc
)

source_group ("Resource Files" REGULAR_EXPRESSION ".*\\.(q?rc|res)")
source_group ("QT Designer UI" REGULAR_EXPRESSION ".*\\.ui")
source_group ("Templates" REGULAR_EXPRESSION ".*\\.in")

# Prepare resources embeded in Reality_Executable
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set (OS_NAME WINDOWS)
else()
	set (OS_NAME OSX)
endif()

add_custom_command (
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gui/resources/textResources/Reality.css
	COMMAND perl ${CMAKE_SOURCE_DIR}/../tools/pp.pl --define ${OS_NAME}=1 --source=${CMAKE_CURRENT_SOURCE_DIR}/gui/resources/textResources/Reality.css.in > ${CMAKE_CURRENT_SOURCE_DIR}/gui/resources/textResources/Reality.css
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gui/resources/textResources/Reality.css.in
	COMMENT "" Preprocessing Reality.css.in!
	VERBATIM
)

# Generate version files
set (BUILDNUMBER_DIR ${PROJECT_BINARY_DIR}/BuildNumber_autogen)
add_custom_target (
	IncrementBuildNumber
	COMMAND ${CMAKE_COMMAND}
		-E make_directory "${BUILDNUMBER_DIR}"
	COMMAND ${CMAKE_COMMAND}
		-DLANGUAGE:STRING="C"
		-DHEADER_DIR:PATH="${BUILDNUMBER_DIR}"
		-DCACHE_DIR:PATH="${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}"
		-P "${CMAKE_CURRENT_LIST_DIR}/cmake/build_number.cmake"
	BYPRODUCTS ${BUILDNUMBER_DIR}/build_number.h
)
set_property (SOURCE ${BUILDNUMBER_DIR}/build_number.h PROPERTY SKIP_AUTOGEN ON)

string (TIMESTAMP GENERATED_TIME UTC)
configure_file (core/ReVersion.cpp.in "${PROJECT_BINARY_DIR}/autogen/ReVersion.cpp" @ONLY)

add_library (Reality_Version INTERFACE)
target_include_directories (Reality_Version INTERFACE "${BUILDNUMBER_DIR}" core)
target_sources (Reality_Version INTERFACE ${SRC_GRP_VERSION})
add_dependencies (Reality_Version IncrementBuildNumber)

# Generate Windows resource files
configure_file (gui/Reality.rc.in "${PROJECT_BINARY_DIR}/autogen/Reality.rc" @ONLY)
configure_file (Poser/Reality_PY.rc.in "${PROJECT_BINARY_DIR}/autogen/Reality_PY.rc" @ONLY)
configure_file (Studio/Reality_DS.rc.in "${PROJECT_BINARY_DIR}/autogen/Reality_DS.rc" @ONLY)

###############################################################################
###############################################################################
###############################################################################

# Configure Reality Widgets
add_library (Reality_Widgets SHARED)
target_compile_definitions (Reality_Widgets PRIVATE RE_LIB_MAKE_SHARED $<$<PLATFORM_ID:Darwin>:QT_CUSTOM_WIDGET>)
target_include_directories (Reality_Widgets PRIVATE core data gui gui/RealityPanel gui/RealityUI gui/RealityUI/MaterialEditors)
target_link_libraries (Reality_Widgets PRIVATE Reality_Version)
target_link_libraries (Reality_Widgets PRIVATE Boost::atomic OpenCL::OpenCL Qt4::QtGui qjson SQLiteCpp cppzmq-static)
set_target_properties (Reality_Widgets PROPERTIES AUTOUIC_SEARCH_PATHS gui/uiFiles)

target_sources (Reality_Widgets PRIVATE
	${HEADER_GROUP_QTDESIGNER}
	${HEADER_GROUP_LIBRARY}
	${HEADER_GROUP_TEXTURE_EDITORS}
	${HEADER_GROUP_MATERIAL_EDITORS}
	${HEADER_GROUP_OTHER_EDITORS}
	${HEADER_GROUP_TEXTURES_MATS}
	${SRC_GROUP_QTDESIGNER}
	${SRC_GROUP_LIBRARY}
	${SRC_GROUP_TEXTURE_EDITORS}
	${SRC_GROUP_MATERIAL_EDITORS}
	${SRC_GROUP_OTHER_EDITORS}
	${SRC_GROUP_TEXTURES_MATS}
)

# Configure Reality library
add_library (Reality_LIB SHARED)
target_compile_definitions (Reality_LIB PRIVATE RE_LIB_MAKE_SHARED)
target_include_directories (Reality_LIB PUBLIC core data)
target_link_libraries (Reality_LIB PUBLIC Reality_Version)
target_link_libraries (Reality_LIB PUBLIC Boost::atomic OpenCL::OpenCL Qt4::QtGui qjson SQLiteCpp cppzmq-static)

target_sources (Reality_LIB PRIVATE
	${SRC_GROUP_RESOURCES}
	${HEADER_GROUP_LIBRARY}
	${HEADER_GROUP_TEXTURES_MATS}
	${SRC_GROUP_LIBRARY}
	${SRC_GROUP_TEXTURES_MATS}
)

# Configure DAZ Studio plugin
add_library (Reality_DS MODULE)
target_compile_definitions (Reality_DS PRIVATE RE_LIB_MAKE_SHARED)
target_include_directories (Reality_DS PRIVATE core data)
target_link_libraries (Reality_DS PRIVATE Reality_Version)
target_link_libraries (Reality_DS PRIVATE Boost::boost OpenCL::OpenCL DAZStudioSDK::Plugin qjson SQLiteCpp cppzmq-static)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set_target_properties (Reality_DS PROPERTIES
		CXX_VISIBILITY_PRESET hidden	# Necessary with XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE?
		XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING YES
		XCODE_ATTRIBUTE_STRIP_STYLE all
	)
endif()

target_sources (Reality_DS PRIVATE
	${SRC_GROUP_RESOURCES}
	${HEADER_GROUP_DAZ_STUDIO_PLUGIN}
	${HEADER_GROUP_LIBRARY}
	${HEADER_GROUP_TEXTURES_MATS}
	${SRC_GROUP_DAZ_STUDIO_PLUGIN}
	${SRC_GROUP_LIBRARY}
	${SRC_GROUP_TEXTURES_MATS}
)

# Configure Poser Addon
Python2_add_library (Reality_PY MODULE)
target_compile_definitions (Reality_PY PRIVATE BOOST_PYTHON_STATIC_LIB RE_LIB_MAKE_SHARED)
target_include_directories (Reality_PY PRIVATE core data)
target_link_libraries (Reality_PY PRIVATE Reality_Version)
target_link_libraries (Reality_PY PRIVATE Boost::python27 OpenCL::OpenCL qjson SQLiteCpp cppzmq-static)
set_target_properties (Reality_PY PROPERTIES OUTPUT_NAME Reality)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set_target_properties (Reality_PY PROPERTIES
		XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING YES
		XCODE_ATTRIBUTE_STRIP_STYLE all
	)
endif()

target_sources (Reality_PY PRIVATE
	${SRC_GROUP_RESOURCES}
	${HEADER_GROUP_POSER_ADDON}
	${HEADER_GROUP_LIBRARY}
	${HEADER_GROUP_TEXTURES_MATS}
	${SRC_GROUP_POSER_ADDON}
	${SRC_GROUP_LIBRARY}
	${SRC_GROUP_TEXTURES_MATS}
)

# Configure Reality executable
add_executable (Reality_Executable WIN32) #MACOSX_BUNDLE
target_compile_definitions (Reality_Executable PRIVATE HOMEPAGE_URL="${CMAKE_PROJECT_HOMEPAGE_URL}")
target_include_directories (Reality_Executable PRIVATE gui gui/actions gui/RealityPanel gui/RealityUI gui/RealityUI/MaterialEditors)
target_link_libraries (Reality_Executable PRIVATE Reality_LIB)
target_link_libraries (Reality_Executable PRIVATE Boost::program_options Qt4::QtWebKit)
set_target_properties (Reality_Executable PROPERTIES AUTOUIC_SEARCH_PATHS gui/uiFiles)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set_target_properties (Reality_Executable PROPERTIES
#		MACOSX_BUNDLE_BUNDLE_NAME PROJECT_NAME
#		MACOSX_BUNDLE_BUNDLE_VERSION PROJECT_VERSION	# Should append build # to this
#		MACOSX_BUNDLE_SHORT_VERSION_STRING PROJECT_VERSION
#		MACOSX_BUNDLE_GUI_IDENTIFIER
#		MACOSX_BUNDLE_COPYRIGHT "Pret-a-3D 2010 - All rights reserved"
#		MACOSX_BUNDLE_ICON_FILE gui/resources/images/Reality.icns
		XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING YES
		XCODE_ATTRIBUTE_STRIP_STYLE all
	)
endif()

target_sources (Reality_Executable PRIVATE
	${SRC_GROUP_GUI}
	${HEADER_GROUP_OTHER_EDITORS}
	${HEADER_GROUP_TEXTURE_EDITORS}
	${HEADER_GROUP_MATERIAL_EDITORS}
	${SRC_GROUP_OTHER_EDITORS}
	${SRC_GROUP_TEXTURE_EDITORS}
	${SRC_GROUP_MATERIAL_EDITORS}
)
