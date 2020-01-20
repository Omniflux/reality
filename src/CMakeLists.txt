#
# Reality Project generator. 
# Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
# see http://www.cmake.org/cmake/help/cmake-2-8-docs.html#policy:CMP0012
if(POLICY CMP0012)
  cmake_policy(SET CMP0012 OLD)
endif()
# see http://www.cmake.org/cmake/help/cmake-2-8-docs.html#policy:CMP0015
if(POLICY CMP0015)
  cmake_policy(SET CMP0015 OLD)
endif()

cmake_policy(SET CMP0042 OLD)
cmake_policy(SET CMP0046 OLD)

# 32 or 64 bits?
IF(NOT BITNESS)
  SET(BITNESS 32)
ENDIF()

MESSAGE("Bitness: " ${BITNESS})

# 
# Project settings 
# 

# Set the project name 
PROJECT("Reality")

# Set the path for the additional modules used for compiling ZeroMQ
SET(REALITY_MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
LIST(APPEND CMAKE_MODULE_PATH ${REALITY_MODULE_DIR})

#
# OS abbreviation for the project
#
IF(APPLE) 
  SET(OS_CODE "mac")
  SET(OS_NAME "OSX")
ELSEIF(WIN32)
  SET(OS_CODE "w${BITNESS}")
  SET(OS_NAME "WINDOWS")
ENDIF()


MESSAGE("CMAKE dir: ${CMAKE_SOURCE_DIR}")
# Don't overconfigure
SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limit configs" FORCE)

#
# Version number
#
SET(REALITY_VERSION_MAJOR "4")
SET(REALITY_VERSION_MINOR "3")
SET(REALITY_VERSION_PATCH "0")

SET(REALITY_VERSION "${REALITY_VERSION_MAJOR}.${REALITY_VERSION_MINOR}.${REALITY_VERSION_PATCH}")

IF(WIN32)
  SET(RE_QT_VERSION "4.8.6")
  IF(${BITNESS} EQUAL 64)
    SET(QT_QMAKE_EXECUTABLE "Z:\\devel\\qt-${RE_QT_VERSION}-src-w64\\bin\\qmake.exe")
  ELSE()
    SET(QT_QMAKE_EXECUTABLE "Z:\\devel\\qt-${RE_QT_VERSION}-bin-w32\\bin\\qmake.exe")
  ENDIF()
  # Needed to use main() under Windows. It allows us to avoid writing a 
  # WinMain*() entry point
  SET(QT_USE_QTMAIN TRUE)

ELSEIF(APPLE)
  SET(RE_QT_VERSION "4.8.7")
  SET(QT_INCLUDE_DIR "/usr/local/Trolltech/Qt-4.8.7/include")
  SET(
    QT_QMAKE_EXECUTABLE 
    "/usr/local/Trolltech/qt-${RE_QT_VERSION}-src-mac/bin/qmake"
  )
  message("QMAKE ${RE_QT_VERSION} PATH: ${QT_QMAKE_EXECUTABLE}")
ENDIF()

#########################################################################
# Find Qt
#########################################################################
FIND_PACKAGE( Qt4 ${RE_QT_VERSION} REQUIRED QtCore QtGui QtXml QtNetwork QtWebKit )

#########################################################################
# Find OpenCL
#########################################################################
FIND_PACKAGE( OpenCL )

MESSAGE("Qt use file: ${QT_USE_FILE}")
INCLUDE(${QT_USE_FILE})

#########################################################################
# Location of the additional libraries used by the project
# ZeroMQ, the DS SDK, Boost, QJSON, SQLite
#########################################################################
SET( PROJECT_LIBS "${CMAKE_SOURCE_DIR}/../libs")

#########################################################################
# Linking the DAZ Studio core library
# Set the name for the library file based on the OS-specific conventions
#########################################################################

IF(APPLE)
  SET( DS_CORE_LIB_BASENAME libdzcore.dylib)
  SET( DS_SDK_DIR32 "${PROJECT_LIBS}/DS4_SDK/lib/mac32" )
  SET( DS_SDK_DIR64 "${PROJECT_LIBS}/DS4_SDK/lib/mac64" )
  SET( QT_LIB_SUFFIX "")
ELSEIF(WIN32)
  SET(DS_CORE_LIB_BASENAME dzcore.lib)
  SET( DS_SDK_DIR32 "${PROJECT_LIBS}/DS4_SDK/lib/Win32" )
  SET( DS_SDK_DIR64 "${PROJECT_LIBS}/DS4_SDK/lib/Win64" )
  SET( QT_LIB_SUFFIX "4")
  # The following variables is necessary to make FIND_LIBRARY() work on Windows
  SET(CMAKE_FIND_LIBRARY_PREFIXES "")
ELSE()
  MESSAGE(FATAL_ERROR "This OS is not supported")
  RETURN()  
ENDIF()

# Set the file name of the library file for the core DS library that we need to link to
SET(DS_CORE_LIB32 "${DS_SDK_DIR32}/${DS_CORE_LIB_BASENAME}")
SET(DS_CORE_LIB64 "${DS_SDK_DIR64}/${DS_CORE_LIB_BASENAME}")

#
# Special macros
#
# The following macros have been lifted from the CMake Qt4 interface
# and modified to use the project's output dirs for the generated files.
# Specifically, the QT4_RUN_MOC macro outputs the moc output into the
# ${CMAKE_SOURCE_DIR}/mocFiles${OS_CODE} directory and the QT4_WRAP_UIC outputs
# the files into the ${CMAKE_SOURCE_DIR}/gui/uiFiles directory
#
# helper macro to set up a moc rule
macro (QT4_CREATE_MOC_COMMAND infile outfile moc_flags moc_options)
  # For Windows, create a parameters file to work around command line length limit
  if (WIN32)
    # Pass the parameters in a file.  Set the working directory to
    # be that containing the parameters file and reference it by
    # just the file name.  This is necessary because the moc tool on
    # MinGW builds does not seem to handle spaces in the path to the
    # file given with the @ syntax.
    get_filename_component(_moc_outfile_name "${outfile}" NAME)
    get_filename_component(_moc_outfile_dir "${outfile}" PATH)
    if(_moc_outfile_dir)
      set(_moc_working_dir WORKING_DIRECTORY ${_moc_outfile_dir})
    endif()
    set (_moc_parameters_file ${outfile}_parameters)
    set (_moc_parameters ${moc_flags} ${moc_options} -o "${outfile}" "${infile}")
    string (REPLACE ";" "\n" _moc_parameters "${_moc_parameters}")
    file (WRITE ${_moc_parameters_file} "${_moc_parameters}")
    add_custom_command(OUTPUT ${outfile}
                       COMMAND ${QT_MOC_EXECUTABLE} @${_moc_outfile_name}_parameters
                       DEPENDS ${infile}
                       ${_moc_working_dir}
                       VERBATIM)
  else ()
    add_custom_command(OUTPUT ${outfile}
                       COMMAND ${QT_MOC_EXECUTABLE}
                       ARGS ${moc_flags} ${moc_options} -o ${outfile} ${infile}
                       DEPENDS ${infile} VERBATIM)
  endif ()
endmacro ()

MACRO (QT4_RUN_MOC outfiles )
  # get include dirs
  QT4_GET_MOC_FLAGS(moc_flags)
  QT4_EXTRACT_OPTIONS(moc_files moc_options ${ARGN})
 
  FOREACH (it ${moc_files})
    GET_FILENAME_COMPONENT(it ${it} ABSOLUTE)
    QT4_MAKE_OUTPUT_FILE(${it} moc_ cxx outfile)
    GET_FILENAME_COMPONENT(outFileName ${it} NAME_WE)
    SET(outfile "${CMAKE_SOURCE_DIR}/mocFiles/moc_${outFileName}.cpp")
    QT4_CREATE_MOC_COMMAND(${it} ${outfile} "${moc_flags}" "${moc_options}")

    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH(it)
    
ENDMACRO (QT4_RUN_MOC)

MACRO (QT4_WRAP_UIC outfiles )
  QT4_EXTRACT_OPTIONS(ui_files ui_options ${ARGN})
  # MESSAGE(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
  # MESSAGE("Generating the UI for ${ui_files}")
  # MESSAGE(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")

  FOREACH (it ${ui_files})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    SET(outfile ${CMAKE_SOURCE_DIR}/gui/uiFiles/ui_${outfile}.h)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      DEPENDS ${infile}
      MAIN_DEPENDENCY ${infile} VERBATIM)
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)

ENDMACRO (QT4_WRAP_UIC)


# QT4_ADD_RESOURCES(outfiles inputfile ... )

MACRO (QT4_ADD_RESOURCES outfiles )
  QT4_EXTRACT_OPTIONS(rcc_files rcc_options ${ARGN})
  FOREACH (it ${rcc_files})
    GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(rc_path ${infile} PATH)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/qrc_${outfilename}.cpp)
    #  parse file for dependencies 
    #  all files are absolute paths or relative to the location of the qrc file
    FILE(READ "${infile}" _RC_FILE_CONTENTS)
    STRING(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
    SET(_RC_DEPENDS)
    FOREACH(_RC_FILE ${_RC_FILES})
      STRING(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
      IF(NOT IS_ABSOLUTE "${_RC_FILE}")
        SET(_RC_FILE "${rc_path}/${_RC_FILE}")
      ENDIF(NOT IS_ABSOLUTE "${_RC_FILE}")
      SET(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
    ENDFOREACH(_RC_FILE)
    # Since this cmake macro is doing the dependency scanning for these files,
    # let's make a configured file and add it as a dependency so cmake is run
    # again when dependencies need to be recomputed.
    QT4_MAKE_OUTPUT_FILE("${infile}" "" "qrc.depends" out_depends)
    CONFIGURE_FILE("${infile}" "${out_depends}" COPYONLY)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_RCC_EXECUTABLE}
      ARGS ${rcc_options} -name ${outfilename} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile}
      DEPENDS ${_RC_DEPENDS} "${out_depends}" VERBATIM)
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)

ENDMACRO (QT4_ADD_RESOURCES)


#
# Sources
#

# 
# Ui Elements
#

SET( UI_FILES  
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/imageMapManager.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meAlpha.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meCloth.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meGlass.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meGlossy.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meMatte.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meMirror.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meMatVolumes.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meModifiers.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meSkin.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meMetal.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meWater.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meVelvet.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/meVolumes.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/realitypanel.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reBreadCrumbs.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reColorPicker.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reMainWindow.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reTextureEditor.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reLightEditor.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teClouds.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teMarble.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teWood.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teBand.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teDistortedNoise.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teBricks.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teColor.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teColorMath.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teCheckers.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teFBM.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teMath.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teImageMap.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teMix.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/teGrayscale.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/te3DMapping.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/tex2DMapping.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/textureAvatar.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reOutputOptions.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/rePresetSelector.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reCameraEditor.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reTextureSelector.ui" 
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reAbout.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reWatermark.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reExportProgress.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reProdRegDialog.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/ReSlider.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/ReUpdateNotification.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reAcselSave.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reUniShaderSave.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reAcselMgr.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reCommandLineOptions.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reAcselOverwriteConfirm.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reShaderSetSelector.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reUniversalShaderSelector.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reUniversalShaderWdg.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reSyncMats.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reConfirmationDialog.ui"
     "${CMAKE_SOURCE_DIR}/gui/uiFiles/reCssEditor.ui"
)


SET(
  REALITY_TEXTURES_MATS
  # ReLight must be listed before ReSceneData or the host app will crash
  # at startup because ReScene instantiates a IBL light. If the ReLight.cpp
  # file is not listed in the right order then the lookup tables in it will
  # not be initialized at the time the IBL light is instantiate and the shared
  # library will crash potentially bringing down Poser or other hosts.
  "${CMAKE_SOURCE_DIR}/data/ReLight.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReSceneData.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReTexture.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReTextureCreator.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReSceneResources.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/Re2DTexture.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReComplexTexture.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReBand.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReBricks.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReClouds.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReColorMath.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReConstant.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReDistortedNoise.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReFBM.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReMarble.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReWood.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReMix.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReFresnelColor.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReGrayscale.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReImageMap.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReMath.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReMix.cpp"
  "${CMAKE_SOURCE_DIR}/data/textures/ReProceduralNoise.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReMaterials.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReMaterial.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReCloth.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReGlossy.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReMix.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReSkin.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReMatte.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReGlass.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReWater.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReMetal.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReMirror.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReVelvet.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReLightMaterial.cpp"  
  "${CMAKE_SOURCE_DIR}/data/ReVolumes.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReDisplaceableMaterial.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReAlphaChannelMaterial.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReGeometryObject.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReNodeConverter.cpp"
  "${CMAKE_SOURCE_DIR}/data/RealityBase.cpp"
  "${CMAKE_SOURCE_DIR}/core/LuxApi.cpp"  
)

SOURCE_GROUP( "Textures and Mats" FILES ${REALITY_TEXTURES_MATS} )
#
# The Sources for the Main Library used by the Reality UI
#
SET( 
  REALITY_LIB_SRC_FILES 
  "${CMAKE_SOURCE_DIR}/data/ReDefs.cpp"
  "${CMAKE_SOURCE_DIR}/core/ReAppVersion.cpp"
  "${CMAKE_SOURCE_DIR}/core/RealityCRC.cpp"
  # Logging
  "${CMAKE_SOURCE_DIR}/core/ReLogger.cpp"
  # IPC and core features
  "${CMAKE_SOURCE_DIR}/core/ReIPC.cpp"
  # Important! The following must be listed before RealityBase.cpp. 
  # The order of initialization is important.
  "${CMAKE_SOURCE_DIR}/core/ReLuxRunner.cpp"  
  "${CMAKE_SOURCE_DIR}/core/zeromqTools.cpp"  
  "${CMAKE_SOURCE_DIR}/core/crc.c"  
  "${CMAKE_SOURCE_DIR}/data/RealityRunner.cpp"  
  # Data handling
  "${CMAKE_SOURCE_DIR}/data/ReTools.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReCamera.cpp"
  # ACTIONS
  "${CMAKE_SOURCE_DIR}/gui/actions/ReAction.cpp"

  ${REALITY_TEXTURES_MATS}
  "${CMAKE_SOURCE_DIR}/core/ReRenderContext.cpp"
  
  # Lux Format
  "${CMAKE_SOURCE_DIR}/data/exporters/lux/ReLuxTextureExporter.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/lux/ReImageMapExporter.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/lux/ReLuxMaterialExporterFactory.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/lux/ReLuxTextureExporterFactory.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/lux/ReLightExporter.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/lux/ReMixExporter.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/lux/ReGlossyExporter.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/lux/ReSkinExporter.cpp"
  # Luxcore/SLG format
  "${CMAKE_SOURCE_DIR}/data/exporters/luxcore/ReLuxcoreMaterialExporterFactory.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/luxcore/ReGlossyExporter.cpp"
  # JSON format
  "${CMAKE_SOURCE_DIR}/data/exporters/json/ReJSONMaterialExporterFactory.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/ReLuxSceneExporter.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/ReSLGSceneExporter.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/ReJSONSceneExporter.cpp"
  # QVariantMap format exporters
  "${CMAKE_SOURCE_DIR}/data/exporters/qt/ReQtTextureExporterFactory.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/qt/ReQtMaterialExporterFactory.cpp"
  "${CMAKE_SOURCE_DIR}/data/exporters/ReQtSceneExporter.cpp"

  # QVariantMap format importers
  "${CMAKE_SOURCE_DIR}/data/importers/qt/ReQtTextureImporterFactory.cpp"
  "${CMAKE_SOURCE_DIR}/data/importers/qt/ReQtMaterialImporterFactory.cpp"
  "${CMAKE_SOURCE_DIR}/data/importers/qt/ReQtSceneImporter.cpp"

  "${CMAKE_SOURCE_DIR}/data/ReLuxGeometryExporter.cpp"
  # PLY
  "${CMAKE_SOURCE_DIR}/data/ply/rply.c"
  # ACSEL
  "${CMAKE_SOURCE_DIR}/data/ReAcsel.cpp"  
  # OpenCL
  "${CMAKE_SOURCE_DIR}/core/ReOpenCL.cpp"
  # DRM
  "${CMAKE_SOURCE_DIR}/core/ReUserData.cpp"  
  "${CMAKE_SOURCE_DIR}/core/blowfish.cpp"  
  "${CMAKE_SOURCE_DIR}/data/ReHttpClient.cpp"  
  "${CMAKE_SOURCE_DIR}/data/ReProductUsage.cpp"  
)

# We need to have separate groups of sources for the texture and material 
# editors because they need to be included in the RealityWidgets library,
# the plugin library for Qt Creator, while the rest of the gui must not be
# incuded there.
SET(
  TEXTURE_EDITORS
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReColorPicker.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReCloudsTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReDistortedNoiseTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReBandTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReBricksTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReMarbleTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReWoodTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReCheckersTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReFBMTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReMathTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReColorMathTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReColorTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReImageMapTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReMixTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReGrayscaleTextureEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureEditor.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/RePresetSelector.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/Re3DMappingEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureAvatar.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReImageMapManager.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/Re2DTextureMapping.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReBreadCrumbs.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReTextureChannelDataModel.cpp"  
)

SET(
  MATERIAL_EDITORS
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTimedEditor.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMaterialEditor.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReAlphaChannelEditor.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReGlassEditor.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReGlossyEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReClothEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMatteEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMatVolumesEditor.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMetalEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMirrorEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReModifiers.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReSkinEditor.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReVelvetEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReVolumesEditor.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReWaterEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReExportProgressDialog.cpp"  
)

SET(
  OTHER_EDITORS
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReOutputOptions.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReLightEditor.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReCameraEditor.cpp"  
)

SET(
  GUI_SOURCES
  "${CMAKE_SOURCE_DIR}/core/ReAppVersion.cpp"
  # "${CMAKE_SOURCE_DIR}/core/blowfish.cpp"
  "${CMAKE_SOURCE_DIR}/core/crc.c"
  # "${CMAKE_SOURCE_DIR}/core/ReOpenCL.cpp"
  "${CMAKE_SOURCE_DIR}/data/ReSceneDataModel.cpp"

  "${CMAKE_SOURCE_DIR}/gui/RealityPanel/RealityDataRelay.cpp"
  "${CMAKE_SOURCE_DIR}/gui/ReMaterialPreview.cpp"  

  ${OTHER_EDITORS}

  #Texture editors
  ${TEXTURE_EDITORS}

  # Material Editors
  ${MATERIAL_EDITORS}

  # GUI
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureSelector.cpp"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReSlider.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReUpdateNotification.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReAcselSave.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReUniShaderSave.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReAcselMgr.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReAcselShaderSetSelector.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReUniversalShaderWidget.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReUniversalShaderSelector.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReConfirmationDialog.cpp"

  # DRM
  "${CMAKE_SOURCE_DIR}/core/ReDRM.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RegistrationDialog.cpp"

  "${CMAKE_SOURCE_DIR}/gui/RealityPanel/realitypanel.cpp"
  "${CMAKE_SOURCE_DIR}/gui/ReMainWindow.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReCssEditor.cpp"
)
# Assign a Source group for the files above just for keeping things tidy
# in the IDE
SOURCE_GROUP( "GUI" FILES ${GUI_SOURCES})

#########################################################################
# The source for the Reality executable
#########################################################################
SET( 
  MAIN_EXECUTABLE 
  "${CMAKE_SOURCE_DIR}/gui/Reality.cpp"  
)

#
# QObject-derived headers for moc
#
SET( CORE_HEADER_FILES 
    "${CMAKE_SOURCE_DIR}/core/ReLuxRunner.h" 
    "${CMAKE_SOURCE_DIR}/gui/actions/ReAction.h"
    # "${CMAKE_SOURCE_DIR}/data/ReProductUsage.h"
)

SET(
  TEXTURE_EDITOR_HEADER_FILES
  "${CMAKE_SOURCE_DIR}/data/ReTextureChannelDataModel.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReAbstractTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReColorPicker.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReImageMapManager.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureAvatar.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/Re2DTextureMapping.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReBreadCrumbs.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReCloudsTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReDistortedNoiseTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReBricksTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReBandTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReMarbleTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReWoodTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReCheckersTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReFBMTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReMathTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReColorMathTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReColorTextureEditor.h" 
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReImageMapTextureEditor.h" 
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReMixTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReGrayscaleTextureEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/Re3DMappingEditor.h"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureEditor.h"  
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureEditorAdapter.h"
)

SET(
  MATERIAL_EDITOR_HEADER_FILES
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMaterialEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReAlphaChannelEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReGlassEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReGlossyEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReClothEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMatteEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMatVolumesEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMetalEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReMirrorEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReModifiers.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReSkinEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReVelvetEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReVolumesEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors/ReWaterEditor.h"
)

SET(
  OTHER_EDITORS_HEADER_FILES
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReLightEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReCameraEditor.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/RePresetSelector.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReOutputOptions.h" 
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReSlider.h"   
)

SET(MAT_PREVIEW_HEADER_FILE "${CMAKE_SOURCE_DIR}/gui/ReMaterialPreview.h")
SET(DATA_RELAY_HEADER_FILE "${CMAKE_SOURCE_DIR}/gui/RealityPanel/RealityDataRelay.h")

SET(
  GUI_HEADER_FILES
  "${CMAKE_SOURCE_DIR}/gui/actions/ReAction.h"  
  "${CMAKE_SOURCE_DIR}/data/ReTextureChannelDataModel.h"
  "${CMAKE_SOURCE_DIR}/data/ReSceneDataModel.h"
  ${DATA_RELAY_HEADER_FILE}
  # "${CMAKE_SOURCE_DIR}/gui/RealityPanel/RealityDataRelay.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureSelector.h"
  ${MAT_PREVIEW_HEADER_FILE}

  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReExportProgressDialog.h"
  # "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReSlider.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReUpdateNotification.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityPanel/realitypanel.h"
  "${CMAKE_SOURCE_DIR}/gui/RegistrationDialog.h"
  "${CMAKE_SOURCE_DIR}/gui/ReMainWindow.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReAcselSave.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReAcselMgr.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReUniShaderSave.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReAcselShaderSetSelector.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReUniversalShaderWidget.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReUniversalShaderSelector.h"
  # "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureEditorAdapter.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReConfirmationDialog.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReCssEditor.h"
)

#########################################################################
# DS files to be MOCed
#########################################################################
SET(
  DS_HEADER_FILES 
  "${CMAKE_SOURCE_DIR}/Studio/Reality_DS.h"
  "${CMAKE_SOURCE_DIR}/Studio/RealityAction.h"
  "${CMAKE_SOURCE_DIR}/Studio/ReDSStorage.h"
  # "${CMAKE_SOURCE_DIR}/Studio/ReDSDebugTools.h"
)

SET(
  RESOURCE_FILES
  "${CMAKE_SOURCE_DIR}/gui/resources/RealityUI.qrc"
)

SET( WIDGETS_DIR "${CMAKE_SOURCE_DIR}/gui/RealityUI/qtDesignerPlugins" )

#
# QTD == Qt Designer
# These lists are the sources used to build the widget library for Qt Designer
#
SET(
  QTD_HEADERS
  "${WIDGETS_DIR}/Re2DTextureMappingPlugin.h"
  "${WIDGETS_DIR}/Re3DMappingEditorPlugin.h"
  "${WIDGETS_DIR}/RealityWidgets.h"
  "${WIDGETS_DIR}/ReBandTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReBreadCrumbsPlugin.h"
  "${WIDGETS_DIR}/ReBricksTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReCameraEditorPlugin.h"
  "${WIDGETS_DIR}/ReCheckersTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReClothEditorPlugin.h"
  "${WIDGETS_DIR}/ReCloudsTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReColorMathTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReColorPickerPlugin.h"
  "${WIDGETS_DIR}/ReColorTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReDistortedNoiseTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReFBMTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReGlassEditorPlugin.h"
  "${WIDGETS_DIR}/ReGlossyEditorPlugin.h"
  "${WIDGETS_DIR}/ReGrayscaleTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReImageMapManagerPlugin.h"
  "${WIDGETS_DIR}/ReImageMapTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReMarbleTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReMathTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReMatteEditorPlugin.h"
  "${WIDGETS_DIR}/ReMatVolumesEditorPlugin.h"
  "${WIDGETS_DIR}/ReMetalEditorPlugin.h"
  "${WIDGETS_DIR}/ReMirrorEditorPlugin.h"
  "${WIDGETS_DIR}/ReMixTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReModifiersEditorPlugin.h"
  "${WIDGETS_DIR}/ReOutputOptionsPlugin.h"
  "${WIDGETS_DIR}/ReSkinEditorPlugin.h"
  "${WIDGETS_DIR}/ReSliderPlugin.h"
  "${WIDGETS_DIR}/ReTextureAvatarPlugin.h"
  "${WIDGETS_DIR}/ReTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReVelvetEditorPlugin.h"
  "${WIDGETS_DIR}/ReVolumesEditorPlugin.h"
  "${WIDGETS_DIR}/ReWaterEditorPlugin.h"
  "${WIDGETS_DIR}/ReWoodTextureEditorPlugin.h"
  "${WIDGETS_DIR}/ReAlphaChannelEditorPlugin.h"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureSelector.h"  
)

SET( 
  QTD_SOURCES
  "${WIDGETS_DIR}/Re2DTextureMappingPlugin.cpp"
  "${WIDGETS_DIR}/Re3DMappingEditorPlugin.cpp"
  "${WIDGETS_DIR}/RealityWidgets.cpp"
  "${WIDGETS_DIR}/ReBandTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReBreadCrumbsPlugin.cpp"
  "${WIDGETS_DIR}/ReBricksTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReCameraEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReCheckersTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReClothEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReCloudsTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReColorMathTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReColorPickerPlugin.cpp"
  "${WIDGETS_DIR}/ReColorTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReDistortedNoiseTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReFBMTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReGlassEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReGlossyEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReGrayscaleTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReImageMapManagerPlugin.cpp"
  "${WIDGETS_DIR}/ReImageMapTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReMarbleTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReMathTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReMatteEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReMatVolumesEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReMetalEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReMirrorEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReMixTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReModifiersEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReOutputOptionsPlugin.cpp"
  "${WIDGETS_DIR}/ReSkinEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReSliderPlugin.cpp"
  "${WIDGETS_DIR}/ReTextureAvatarPlugin.cpp"
  "${WIDGETS_DIR}/ReTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReVelvetEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReVolumesEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReWaterEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReWoodTextureEditorPlugin.cpp"
  "${WIDGETS_DIR}/ReAlphaChannelEditorPlugin.cpp"
  "${CMAKE_SOURCE_DIR}/gui/ReMaterialPreview.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReTextureSelector.cpp"
  "${CMAKE_SOURCE_DIR}/core/ReOpenCL.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/ReSlider.cpp"
  "${CMAKE_SOURCE_DIR}/gui/RealityPanel/RealityDataRelay.cpp"
)

#
# Generate the files from the .ui sources and the moc files from the headers
# The NO_OPTIONS argument is necessary or the macro will omit the first 
# file name in the list
#
QT4_WRAP_UIC(UI_LIBRARY "NO_OPTIONS" ${UI_FILES})
QT4_RUN_MOC(MOC_FILES "NO_OPTIONS" ${CORE_HEADER_FILES} )
QT4_RUN_MOC(GUI_MOC_FILES "NO_OPTIONS" ${GUI_HEADER_FILES} )
QT4_RUN_MOC(GUI_MOC_FILES "NO_OPTIONS" ${TEXTURE_EDITOR_HEADER_FILES} )
QT4_RUN_MOC(GUI_MOC_FILES "NO_OPTIONS" ${MATERIAL_EDITOR_HEADER_FILES} )
QT4_RUN_MOC(GUI_MOC_FILES "NO_OPTIONS" ${OTHER_EDITORS_HEADER_FILES} )
QT4_RUN_MOC(DS_MOC_FILES "NO_OPTIONS" ${DS_HEADER_FILES} )
QT4_ADD_RESOURCES(RESOURCES "NO_OPTIONS" ${RESOURCE_FILES})

QT4_RUN_MOC(QTC_PLUGINS_MOC_FILES "NO_OPTIONS" ${QTD_HEADERS} )
QT4_RUN_MOC(QTC_PLUGINS_MOC_FILES "NO_OPTIONS" ${TEXTURE_EDITOR_HEADER_FILES} )
QT4_RUN_MOC(QTC_PLUGINS_MOC_FILES "NO_OPTIONS" ${MATERIAL_EDITOR_HEADER_FILES} )
QT4_RUN_MOC(QTC_PLUGINS_MOC_FILES "NO_OPTIONS" ${OTHER_EDITORS_HEADER_FILES} )
QT4_RUN_MOC(QTC_PLUGINS_MOC_FILES "NO_OPTIONS" ${MAT_PREVIEW_HEADER_FILE} )
QT4_RUN_MOC(QTC_PLUGINS_MOC_FILES "NO_OPTIONS" ${DATA_RELAY_HEADER_FILE} )

MESSAGE("Resources: ${RESOURCES}")
# MESSAGE("UI files: ${UI_LIBRARY}")
MESSAGE("Moc files: ${MOC_FILES}")
# MESSAGE("GUI Moc files: ${GUI_MOC_FILES}")
# MESSAGE("DS Moc files: ${DS_MOC_FILES}")

# 
# Tell CMake that we are building a shared library
#
SET(LIB_TYPE SHARED)

#########################################################################
# The Library name
#########################################################################

SET(REALITY_LIB "Reality_lib")

#########################################################################
# The Qt Designer plugins
#########################################################################
SET(REALITY_WIDGETS "RealityWidgets")

#########################################################################
#
# The main program, our target
#
#########################################################################
SET(REALITY_EXECUTABLE "Reality" )

##
# Find Python 
##

# Set the path for WIndows to be where we built Python. Mac OS
# comes with Python pre-installed and with the build files in the framework

# The version of Python needed for our project. This is based on what
# Python is included with Poser
# SET(RE_PYTHON_VERSION 2.7)

IF(WIN32)
  SET(
    PYTHON_LIBRARY  
    "${CMAKE_SOURCE_DIR}/../libs/python/w${BITNESS}/PCbuild/python27.lib"
  )
  SET(
    PYTHON_INCLUDE_DIRS 
    "${CMAKE_SOURCE_DIR}/../libs/python/w${BITNESS}/include"
  )
ELSE()
  # FIND_PACKAGE(PythonLibs ${RE_PYTHON_VERSION})
  SET(PYTHON_LIBRARY  "${CMAKE_SOURCE_DIR}/../libs/python/mac/libpython2.7.dylib")
  SET(PYTHON_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../libs/python/mac/include")
ENDIF()

MESSAGE("Python found? ${PYTHONLIBS_FOUND}")
# IF(${PYTHONLIBS_FOUND})
  MESSAGE("Python include files: ${PYTHON_INCLUDE_DIRS}")
  MESSAGE("Python libs: ${PYTHON_LIBRARY}")
# ENDIF()
#
# Include directories
# 
INCLUDE_DIRECTORIES( 
  "${CMAKE_SOURCE_DIR}/gui"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/MaterialEditors"
  "${CMAKE_SOURCE_DIR}/gui/RealityUI/qtDesignerPlugins"
  "${CMAKE_SOURCE_DIR}/gui/RealityPanel"
  "${CMAKE_SOURCE_DIR}/gui/actions"
  "${CMAKE_SOURCE_DIR}/data"
  "${CMAKE_SOURCE_DIR}/gui/uiFiles" 
  "${PROJECT_LIBS}" 
  "${PROJECT_LIBS}/zeromq4/include"
  "${PROJECT_LIBS}/DS4_SDK/include"
  "${PROJECT_LIBS}/boost"
  "${PROJECT_LIBS}/sqlite"
  "${PROJECT_LIBS}/SQLiteCpp/include"
  "${PROJECT_LIBS}/GLTools/include"
  "${PROJECT_LIBS}/yaml-cpp/include"
  ${OpenCL_INCLUDE_DIR}
  "${CMAKE_SOURCE_DIR}/core"
  ${PYTHON_INCLUDE_DIRS}
  # The following include is for the platform-dependent platform.hpp from 0MQ
  # and other config files that are generated on the fly
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Set the output for the Debug/Release binaries to be above the 
# source directory, both for the executable and for the library.
# By using the explicit DEBUG|RELEASE causes the rules become indipendent
# from the IDE/Build system used
SET(
  CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE
  "${CMAKE_SOURCE_DIR}/../bin/${OS_CODE}/Release"
)
SET(
  CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG
  "${CMAKE_SOURCE_DIR}/../bin/${OS_CODE}/Debug"
)
SET(
  CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE
  "${CMAKE_SOURCE_DIR}/../bin/${OS_CODE}/Release"
)
SET(
  CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG
  "${CMAKE_SOURCE_DIR}/../bin/${OS_CODE}/Debug"
)

############################################################################
# ZEROMQ. We compile it from the sources to avoid linking problems with
# the library. ZeroMQ 4 is not yet configured to be easy to be linked
# in Mac OS and Windows. 
############################################################################
INCLUDE(CheckCSourceRuns)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)

INCLUDE(zeromq)

############################################################################
# Boost
############################################################################
IF(APPLE)
  SET(
    BOOST_PROGRAM_OPTIONS
    "${PROJECT_LIBS}/boost/stage/lib/mac/libboost_program_options.a"
  )
  SET(
    BOOST_PYTHON
    "${PROJECT_LIBS}/boost/stage/lib/mac/libboost_python.a"
  )
  SET(
    BOOST_THREAD
    "${PROJECT_LIBS}/boost/stage/lib/mac/libboost_thread.a"
  )
  SET(
    BOOST_SYSTEM
    "${PROJECT_LIBS}/boost/stage/lib/mac/libboost_system.a"
  )
  SET(
    BOOST_ATOMIC
    "${PROJECT_LIBS}/boost/stage/lib/mac/libboost_atomic.a"
  )
  SET(
    BOOST_DATE_TIME
    "${PROJECT_LIBS}/boost/stage/lib/mac/libboost_date_time.a"
  )
  SET(
    BOOST_REGEX
    "${PROJECT_LIBS}/boost/stage/lib/mac/libboost_regex.a"
  )
  SET(
    BOOST_CHRONO
    "${PROJECT_LIBS}/boost/stage/lib/mac/libboost_chrono.a"
  )

ELSE()
  SET(
    BOOST_PROGRAM_OPTIONS 
    "${PROJECT_LIBS}/boost/stage/lib/w${BITNESS}/libboost_program_options-vc100-mt-1_54.lib"
  )
  SET(
    BOOST_PYTHON 
    "${PROJECT_LIBS}/boost/stage/lib/w${BITNESS}/libboost_python-vc100-mt-1_54.lib"
  )
  SET(
    BOOST_THREAD
    "${PROJECT_LIBS}/boost/stage/lib/w${BITNESS}/libboost_thread-vc100-mt-1_54.lib"
  )
  SET(
    BOOST_SYSTEM
    "${PROJECT_LIBS}/boost/stage/lib/w${BITNESS}/libboost_system-vc100-mt-1_54.lib"
  )
  SET(
    BOOST_ATOMIC
    "${PROJECT_LIBS}/boost/stage/lib/w${BITNESS}/libboost_atomic-vc100-mt-1_54.lib"
  )
  SET(
    BOOST_DATE_TIME
    "${PROJECT_LIBS}/boost/stage/lib/w${BITNESS}/libboost_date_time-vc100-mt-1_54.lib"
  )
  SET(
    BOOST_REGEX
    "${PROJECT_LIBS}/boost/stage/lib/w${BITNESS}/libboost_regex-vc100-mt-1_54.lib"
  )
  SET(
    BOOST_CHRONO
    "${PROJECT_LIBS}/boost/stage/lib/w${BITNESS}/libboost_chrono-vc100-mt-1_54.lib"
  )

ENDIF()

############################################################################
# YAML-CPP
############################################################################
IF(APPLE)
  SET(YAML_LIB_NAME "libyaml-cpp.a")
ELSEIF(WIN32)
  SET(YAML_LIB_NAME "libyaml-cppmd.lib")
ENDIF()

SET(YAML_LIB "${PROJECT_LIBS}/yaml-cpp/${OS_CODE}/${YAML_LIB_NAME}")

############################################################################
# QJSON. Included as sources instead of using a library, it's less hassle
############################################################################
SET(QJSON_PATH "${PROJECT_LIBS}/qjson")

SET(QJSON_MOC_HEADERS
  "${QJSON_PATH}/src/parserrunnable.h"
  "${QJSON_PATH}/src/serializerrunnable.h"
)

SET(
  QSJON_SOURCES

  "${QJSON_PATH}/src/parser.cpp"
  "${QJSON_PATH}/src/qobjecthelper.cpp"
  "${QJSON_PATH}/src/json_scanner.cpp"
  "${QJSON_PATH}/src/json_parser.cc"
  "${QJSON_PATH}/src/parserrunnable.cpp"
  "${QJSON_PATH}/src/serializer.cpp"
  "${QJSON_PATH}/src/serializerrunnable.cpp"
)
# MOC the headers
QT4_RUN_MOC(QJSON_MOC_FILES "NO_OPTIONS" ${QJSON_MOC_HEADERS} )
MESSAGE("QJSon mocs: ${QJSON_MOC_FILES}")

SOURCE_GROUP(QJSON FILES ${QSJON_SOURCES})

############################################################################
# SQLite
############################################################################

SET( SQLITE_DIR "${PROJECT_LIBS}/sqlite" )
SET( SQLITE_CPP_DIR "${PROJECT_LIBS}/SQLiteCpp" )
FILE(GLOB SQLITE_SOURCES "${SQLITE_CPP_DIR}/src/*.cpp")
SET( 
  SQLITE_SOURCES
  "${SQLITE_SOURCES}"
  "${SQLITE_DIR}/sqlite3.c" 
) 
IF(APPLE)
  # Create a universal, 32 and 64 bits, executable
  SET(CMAKE_OSX_ARCHITECTURES "x86_64;i386")

  # Set the base SDK at 10.7 
  # SET(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk)
  SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.7) # keep this @ 10.7 to archieve bw-compatibility by weak-linking !  
ENDIF()

#########################################################################
# Enable C++11 for GCC/MinGW
#########################################################################
IF(MINGW) 
  ADD_DEFINITIONS("-std=c++11 -stdlib=libc++")
ENDIF()

#########################################################################
# Targets
#########################################################################
ADD_CUSTOM_TARGET(REBUILD_UI COMMAND echo "Rebuilding the UI files...")
ADD_DEPENDENCIES(REBUILD_UI ${UI_FILES})

#########################################################################
# The Reality Shared library
#########################################################################
ADD_LIBRARY(
  ${REALITY_LIB} 
  ${LIB_TYPE}
  ${REBUILD_UI}
  ${MOC_FILES} 
  ${REALITY_LIB_SRC_FILES} 
  ${ZMQ_SOURCES}
  ${QSJON_SOURCES}
  ${SQLITE_SOURCES}
  ${QJSON_MOC_FILES}
  ${RESOURCES}
)
SET_PROPERTY(TARGET ${REALITY_LIB} PROPERTY POSITION_INDEPENDENT_CODE 1)  

#########################################################################
# The Reality Widgets for Qt Designer
#########################################################################
ADD_LIBRARY(
  ${REALITY_WIDGETS} 
  ${LIB_TYPE}
  ${REBUILD_UI}
  ${MOC_FILES} 
  ${REALITY_LIB_SRC_FILES} 
  ${ZMQ_SOURCES}
  ${QSJON_SOURCES}
  ${SQLITE_SOURCES}
  ${QJSON_MOC_FILES}

  ${QTC_PLUGINS_MOC_FILES}

  ${TEXTURE_EDITORS}
  ${MATERIAL_EDITORS}
  ${OTHER_EDITORS}

  ${QTD_SOURCES}
  ${RESOURCES}
)

#########################################################################
#
# Reality Python extension
#
#########################################################################

SET( RE_PYTHON_LIB_NAME "Reality" )
SET( RE_PYTHON_EXTENSION "${RE_PYTHON_LIB_NAME}X" )
# Python extensions have specific suffix. On Unix systems the must
# end in .so, on Windows they must end in .pyd
IF(APPLE)
  SET(RE_PYTHON_LIB_NAME_SUFFIX ".so")
ELSEIF(WIN32)
  SET(RE_PYTHON_LIB_NAME_SUFFIX ".pyd")
ENDIF()

SET( 
  RE_PYTHON_LIB_SOURCES 
  "${CMAKE_SOURCE_DIR}/Poser/RePythonTools.cpp"
  "${CMAKE_SOURCE_DIR}/Poser/Reality_ext.cpp"
  "${CMAKE_SOURCE_DIR}/Poser/RePoserInterface.cpp"
  "${CMAKE_SOURCE_DIR}/Poser/RePythonSceneExporter.cpp"
)

# The 64-bit version
ADD_LIBRARY(
  ${RE_PYTHON_EXTENSION}
  SHARED
  ${RE_PYTHON_LIB_SOURCES}
)
SET_TARGET_PROPERTIES(
  ${RE_PYTHON_EXTENSION}
  PROPERTIES
  OUTPUT_NAME ${RE_PYTHON_LIB_NAME}
  PREFIX ""
  SUFFIX ${RE_PYTHON_LIB_NAME_SUFFIX}
)

#########################################################################
#
# Reality for DAZ Studio
#
#########################################################################

###############################################################
# The Studio library plugin
# Studio does not use universal binary so we need to split the 
# building of the library in 32 and 64 bit options
###############################################################
#
# The product name
#
SET(REALITY_DS_LIB64 "Reality_DS64")
SET(REALITY_DS_LIB32 "Reality_DS32")

SET( 
  DS_LIB_SRC_FILES 
  "${CMAKE_SOURCE_DIR}/Studio/Reality_DS.cpp"
  "${CMAKE_SOURCE_DIR}/Studio/RealityAction.cpp"
  "${CMAKE_SOURCE_DIR}/Studio/ReDSMaterialConverter.cpp"
  # "${CMAKE_SOURCE_DIR}/Studio/ReObjectAdder.cpp"
  "${CMAKE_SOURCE_DIR}/Studio/ReGUID.cpp"
  "${CMAKE_SOURCE_DIR}/Studio/ReDSStorage.cpp"
  "${CMAKE_SOURCE_DIR}/Studio/ReGeometryExporter.cpp"
  "${CMAKE_SOURCE_DIR}/Studio/ReDAZStudioEvent.cpp"
  # "${CMAKE_SOURCE_DIR}/Studio/ReDSDebugTools.cpp"

  ${REALITY_LIB_SRC_FILES}
  ${RESOURCES}  
)

SET(
  DS_UNIT_TEST_SRC
  "${CMAKE_SOURCE_DIR}/Studio/unitTest.cpp"  
)

# Windows App icon
SET(WRES_FILE "${CMAKE_SOURCE_DIR}/gui/Reality.res")

#
# Extra files
#
IF(WIN32)
  # The Windows version needs a .def file to export the plugin's entry points
  SET(DS_PLUGIN_EXTRA_FILES "${CMAKE_SOURCE_DIR}/Reality.def")
ELSE()
  SET(DS_PLUGIN_EXTRA_FILES "")
ENDIF()

# The following components are common for both the 32 and 64-bit
# versions of the library
SET(
  REALITY_DS_COMPONENTS
  ${MOC_FILES} 
  ${DS_LIB_SRC_FILES}
  ${DS_MOC_FILES}
  ${ZMQ_SOURCES}  
  ${QJSON_MOC_FILES}
  ${QSJON_SOURCES}
  ${SQLITE_SOURCES}  
  ${DS_PLUGIN_EXTRA_FILES}
)
# The 32-bit version
ADD_LIBRARY(
  ${REALITY_DS_LIB32}
  ${LIB_TYPE}
  ${REALITY_DS_COMPONENTS}
)

# The 64-bit version
ADD_LIBRARY(
  ${REALITY_DS_LIB64}
  ${LIB_TYPE}
  ${REALITY_DS_COMPONENTS}
)

SET_PROPERTY(TARGET ${REALITY_DS_LIB32} PROPERTY POSITION_INDEPENDENT_CODE 1)  
SET_PROPERTY(TARGET ${REALITY_DS_LIB64} PROPERTY POSITION_INDEPENDENT_CODE 1)  

FIND_LIBRARY(DS_QT_CORE32 QtCore${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR32} NO_DEFAULT_PATH)
FIND_LIBRARY(DS_QT_GUI32 QtGui${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR32} NO_DEFAULT_PATH)
FIND_LIBRARY(DS_QT_NETWORK32 QtNetwork${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR32} NO_DEFAULT_PATH)
FIND_LIBRARY(DS_QT_SCRIPT32 QtScript${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR32} NO_DEFAULT_PATH)
FIND_LIBRARY(DS_QT_XML32 QtXml${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR32} NO_DEFAULT_PATH)
# FIND_LIBRARY(DS_QT_SQL32 QtSql${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR32} NO_DEFAULT_PATH)

FIND_LIBRARY(DS_QT_CORE64 QtCore${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR64} NO_DEFAULT_PATH)
FIND_LIBRARY(DS_QT_GUI64 QtGui${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR64} NO_DEFAULT_PATH)
FIND_LIBRARY(DS_QT_NETWORK64 QtNetwork${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR64} NO_DEFAULT_PATH)
FIND_LIBRARY(DS_QT_SCRIPT64 QtScript${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR64} NO_DEFAULT_PATH)
FIND_LIBRARY(DS_QT_XML64 QtXml${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR64} NO_DEFAULT_PATH)
# FIND_LIBRARY(DS_QT_SQL64 QtSql${QT_LIB_SUFFIX} PATHS ${DS_SDK_DIR64} NO_DEFAULT_PATH)

IF( ${DS_QT_CORE32} STREQUAL "DS_QT_CORE32-NOTFOUND" )
  MESSAGE( FATAL_ERROR "Error: could not find Qt")
  return()
ENDIF()

#############################################
# Regenerate the Qt resource file every time
#############################################
SET(REBUILD_RESOURCES RebuildResources)
GET_FILENAME_COMPONENT(RESOURCE_NAME ${RESOURCE_FILES} NAME_WE)
  
# Generate the Reality.css file from the template, via the pre-processor
ADD_CUSTOM_TARGET(
  ${REBUILD_RESOURCES}
  COMMAND echo "Rebuilding the resource file ${RESOURCES}"
  COMMAND perl ${CMAKE_SOURCE_DIR}/../tools/pp.pl --define ${OS_NAME}=1 --source=${CMAKE_SOURCE_DIR}/gui/resources/textResources/Reality.css.in > ${CMAKE_SOURCE_DIR}/gui/resources/textResources/Reality.css
  COMMAND ${QT_RCC_EXECUTABLE} -name ${RESOURCE_NAME} -o ${RESOURCES} ${RESOURCE_FILES}
)

# Assert definition for the SQLiteCPP library
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSQLITECPP_ENABLE_ASSERT_HANDLER=1")

IF(APPLE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

  ##############################
  # The Reality executable
  ##############################
  ADD_EXECUTABLE(${REALITY_EXECUTABLE} MACOSX_BUNDLE 
                 ${GUI_MOC_FILES} 
                 ${GUI_HEADER_FILES}
                 ${MAIN_EXECUTABLE} 
                 ${GUI_SOURCES} 
                 ${ZMQ_SOURCES}
                 ${SQLITE_SOURCES}
                 ${UI_LIBRARY}
                 ${UI_FILES})

  # Set the properties for the Info.plist file of the app
  SET_TARGET_PROPERTIES(
    ${REALITY_EXECUTABLE} PROPERTIES 
    MACOSX_BUNDLE_BUNDLE_NAME "Reality"
    MACOSX_BUNDLE_ICON_FILE "Reality.icns"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${REALITY_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "Pret-a-3D 2010 - All rights reserved"
    # Necessary to load the Reality library embedded in the bundle at the same level
    # of the executable. The library has an install name of @rpath
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/."
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    # XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING "YES"
  )

  #
  # Reality's main library
  #
  SET_TARGET_PROPERTIES(
    ${REALITY_LIB} PROPERTIES 
    # Make sure that the library is built with the public symbols exported
    DEFINE_SYMBOL "RE_LIB_MAKE_SHARED;QJSON_MAKEDLL"
    # Change the install path of the dynamic library so that it can be loaded by
    # the executable from inside the bundle
    XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
    # Make all the symbols in the shared libraries hidden by default. We explicitly 
    # export what we need to be visible.
    XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN "YES"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    # XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING "YES"
    # Remove the lib prefix
    PREFIX ""    
  )

  #
  # Reality Widgets Library for Qt Creator
  #
  SET_TARGET_PROPERTIES(
    ${REALITY_WIDGETS} PROPERTIES 
    # Make sure that the library is built with the public symbols exported
    DEFINE_SYMBOL "RE_LIB_MAKE_SHARED;QJSON_MAKEDLL;QT_CUSTOM_WIDGET"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    # Remove the lib prefix
    PREFIX ""    
  )


  #
  # Properties for the Python Extension
  #
  SET_TARGET_PROPERTIES(
    ${RE_PYTHON_EXTENSION}
    PROPERTIES
    XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "YES"
#    XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING "YES"
    XCODE_ATTRIBUTE_STRIP_STYLE "all"
#    VERSION "${REALITY_VERSION}"
  )

  #
  # Set the architecture for the 32-bit version of the DS plugin
  #
  SET_TARGET_PROPERTIES(
    ${REALITY_DS_LIB32} PROPERTIES 
    XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD_32_BIT)"
    XCODE_ATTRIBUTE_VALID_ARCHS "i386"
    # Make the library relocatable
    XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
    XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${CMAKE_SOURCE_DIR}/exportedPluginSymbols.txt"
    XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "YES"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    # XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING "YES"
    XCODE_ATTRIBUTE_STRIP_STYLE "all"
    # Export the symbols that are explicitly marked
    DEFINE_SYMBOL "RE_LIB_MAKE_SHARED"
    # Remove the "lib" prefix
    PREFIX ""    
#    VERSION "${REALITY_VERSION}"
  )

  #
  # Set the architecture for the 64-bit version of the DS plugin
  #
  SET_TARGET_PROPERTIES(
    ${REALITY_DS_LIB64} PROPERTIES 
    XCODE_ATTRIBUTE_CURRENT_ARCH "x86_64"
    XCODE_ATTRIBUTE_VALID_ARCHS "x86_64"
    # Make the library relocatable
    XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
    XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${CMAKE_SOURCE_DIR}/exportedPluginSymbols.txt"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    # XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING "YES"
    XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "YES"
    XCODE_ATTRIBUTE_STRIP_STYLE "all"
    # Export the symbols that are explicitly marked
    DEFINE_SYMBOL "RE_LIB_MAKE_SHARED"
    # Remove the "lib" prefix
    PREFIX ""    
#    VERSION "${REALITY_VERSION}"
  )

  # Delete the qrc resource file to force rebuilding it every time
  ADD_CUSTOM_COMMAND(
    TARGET ${REALITY_EXECUTABLE} 
    PRE_BUILD
    COMMAND rm -rf "${CMAKE_CURRENT_BINARY_DIR}/qrc_RealityUI.cpp"
  )

  #
  # Create the .app bundle for the Reality executable
  #
  ADD_CUSTOM_COMMAND(TARGET ${REALITY_EXECUTABLE} POST_BUILD
    COMMAND rm -rf "$<TARGET_FILE_DIR:${REALITY_EXECUTABLE}>/../Resources"
    COMMAND mkdir "$<TARGET_FILE_DIR:${REALITY_EXECUTABLE}>/../Resources"
    COMMAND cp "${CMAKE_SOURCE_DIR}/gui/resources/images/Reality.icns" "$<TARGET_FILE_DIR:${REALITY_EXECUTABLE}>/../Resources" 
    COMMAND cp "${CMAKE_SOURCE_DIR}/../assets/info-plist/Info.plist" "$<TARGET_FILE_DIR:${REALITY_EXECUTABLE}>/.." 
    COMMAND mv "$<TARGET_FILE_DIR:${REALITY_EXECUTABLE}>/../../../${REALITY_LIB}.dylib" "$<TARGET_FILE_DIR:${REALITY_EXECUTABLE}>"
  )

  ###########################################################
  # Patching the Mac OS version of the DS plugin to relocate 
  # where the Qt libraries are loaded from
  ###########################################################

  ADD_CUSTOM_COMMAND(
    TARGET ${REALITY_DS_LIB32} POST_BUILD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/${OS_CODE}/$(CONFIGURATION)"
    COMMAND python "${CMAKE_SOURCE_DIR}/fixDSPluginQtPaths.py" ${REALITY_DS_LIB32}.dylib
    COMMENT "Adjusting the load path for ${REALITY_DS_LIB32}.dylib -- $(CONFIGURATION_BUILD_DIR)"
  )

  ADD_CUSTOM_COMMAND(
    TARGET ${REALITY_DS_LIB64} POST_BUILD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/${OS_CODE}/$(CONFIGURATION)"
    COMMAND python "${CMAKE_SOURCE_DIR}/fixDSPluginQtPaths.py" ${REALITY_DS_LIB64}.dylib
    COMMENT "Adjusting the load path for ${REALITY_DS_LIB64}.dylib -- $(CONFIGURATION_BUILD_DIR)"
  )

  # Copy the Widgets library to Qt Designer
  ADD_CUSTOM_COMMAND(
    TARGET ${REALITY_WIDGETS} POST_BUILD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/${OS_CODE}/$(CONFIGURATION)"
    COMMAND cp "${REALITY_WIDGETS}.dylib" "/usr/local/Trolltech/Qt-${RE_QT_VERSION}/plugins/designer"
    COMMENT "Copying the Reality widgets plugins to Qt Designer"
  )

  # # Remove the "lib" prefix from the DS plugin 
  # SET_PROPERTY(TARGET ${REALITY_DS_LIB} PROPERTY PREFIX "")

ELSEIF(WIN32)
  # Flags for enabling the parallel compilation under Windows
  IF(MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")  
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")  
  ENDIF()

  # The Reality library
  SET_TARGET_PROPERTIES(
    ${REALITY_LIB} 
    PROPERTIES 
    # Necessary to compile the DLL with the correct import/export modifiers.
    # The BOOST_ALL_NO_LIB symbol is needed to disable the blasted auto-linking of Boost
    # The BOOST_PYTHON_STATIC_LIB is necessary to make it possible to link
    # Boost.Python statically.
    # _CRT_SECURE_NO_WARNINGS is defined to avoid a bunch of warning when 
    # compiling ZeroMQ 
    # DEFINE_SYMBOL "RE_LIB_MAKE_SHARED;QJSON_MAKEDLL;DLL_EXPORT;BOOST_ALL_NO_LIB;_CRT_SECURE_NO_WARNINGS"
    COMPILE_DEFINITIONS "RE_LIB_MAKE_SHARED;QJSON_MAKEDLL;DLL_EXPORT;BOOST_ALL_NO_LIB;_CRT_SECURE_NO_WARNINGS"
  )

  # The Reality for DS library
  SET_TARGET_PROPERTIES(
    ${REALITY_DS_LIB32} 
    PROPERTIES 
    # Necessary to compile the DLL with the correct import/export modifiers.
    # DEFINE_SYMBOL "RE_LIB_MAKE_SHARED;QJSON_MAKEDLL;DLL_EXPORT"
    COMPILE_DEFINITIONS "RE_LIB_MAKE_SHARED;QJSON_MAKEDLL;DLL_EXPORT"
  )
  # The Reality for DS library
  SET_TARGET_PROPERTIES(
    ${REALITY_DS_LIB64} 
    PROPERTIES 
    # Necessary to compile the DLL with the correct import/export modifiers.
    # DEFINE_SYMBOL "RE_LIB_MAKE_SHARED;QJSON_MAKEDLL;DLL_EXPORT"
    COMPILE_DEFINITIONS "RE_LIB_MAKE_SHARED;QJSON_MAKEDLL;DLL_EXPORT"
  )

  ##############################
  # The Reality executable
  ##############################
  ADD_EXECUTABLE( ${REALITY_EXECUTABLE} 
                  ${GUI_MOC_FILES} 
                  ${GUI_HEADER_FILES} 
                  ${MAIN_EXECUTABLE} 
                  ${GUI_SOURCES}
                  ${SQLITE_SOURCES}
                  ${UI_LIBRARY} 
                  ${UI_FILES} )
  
  # It must be "windows" in lowercase due to a cmake bug. Otherwise it will
  # not work
  SET_TARGET_PROPERTIES(  
    ${REALITY_EXECUTABLE} 
    PROPERTIES LINK_FLAGS "/SUBSYSTEM:windows"
  )

  # ADD_CUSTOM_COMMAND(TARGET ${REALITY_EXECUTABLE} POST_BUILD
  #   COMMAND C:\\perl\\perl\\bin\\perl ${CMAKE_SOURCE_DIR}/../LEM_db_formatter "${CMAKE_SOURCE_DIR}/../assets/configFiles/LEM_excludeList.txt" w > "$<TARGET_FILE_DIR:${REALITY_EXECUTABLE}>/LEM_excludeList.txt"
  # )

ENDIF()

#
# Linking of libraries for the executable
#
ADD_DEPENDENCIES(${REALITY_LIB} ${REALITY_LIB} ${REBUILD_RESOURCES})
ADD_DEPENDENCIES(${REALITY_EXECUTABLE} ${REALITY_LIB} ${REBUILD_RESOURCES})

##############################
# Link libraries for Reality
##############################

# The Reality Python extension for Poser
TARGET_LINK_LIBRARIES(
  ${RE_PYTHON_EXTENSION} 
  ${BOOST_PYTHON}
  ${PYTHON_LIBRARY}
  ${REALITY_LIB}
)
SET_TARGET_PROPERTIES(
  ${RE_PYTHON_EXTENSION} 
  PROPERTIES 
  # Necessary to compile the dylib/DLL with the correct import/export modifiers.
  # The BOOST_ALL_NO_LIB symbol is needed to disable the blasted auto-linking of Boost
  # DEFINE_SYMBOL "BOOST_ALL_NO_LIB;BOOST_PYTHON_STATIC_LIB"
  COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB;BOOST_PYTHON_STATIC_LIB"
)

IF(APPLE)
  TARGET_LINK_LIBRARIES(
    ${REALITY_LIB}
    ${QT_QTCORE_LIBRARY} 
    ${QT_QTNETWORK_LIBRARY} 
    ${QT_QTGUI_LIBRARY}
    ${BOOST_THREAD}
    ${BOOST_SYSTEM}
  )

  TARGET_LINK_LIBRARIES(
    ${REALITY_WIDGETS} 
    ${QT_QTCORE_LIBRARY} 
    ${QT_QTNETWORK_LIBRARY} 
    ${QT_QTGUI_LIBRARY}
    ${BOOST_THREAD}
    ${BOOST_SYSTEM}
  )

  TARGET_LINK_LIBRARIES(
    ${REALITY_EXECUTABLE} 
    ${QT_LIBRARIES} 
    ${QT_QTNETWORK_LIBRARY} 
    ${REALITY_LIB} 
    ${BOOST_PROGRAM_OPTIONS}
  )

  ADD_CUSTOM_COMMAND(
    TARGET ${RE_PYTHON_EXTENSION} POST_BUILD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin/${OS_CODE}/$(CONFIGURATION)"
    COMMAND install_name_tool -change "@rpath/$<TARGET_FILE_NAME:${REALITY_LIB}>" "@loader_path/$<TARGET_FILE_NAME:${REALITY_LIB}>" "${RE_PYTHON_LIB_NAME}${RE_PYTHON_LIB_NAME_SUFFIX}"
  )

  # Dependencies for the DS plugin library 32-bit
  TARGET_LINK_LIBRARIES(
    ${REALITY_DS_LIB32} 
    ${DS_CORE_LIB32}
    ${DS_QT_CORE32}
    ${DS_QT_GUI32}
    ${DS_QT_SCRIPT32}
    ${DS_QTNETWORK32}     
    ${BOOST_THREAD}
    ${BOOST_SYSTEM}    
  )

  # Dependencies for the DS plugin library 64-bit
  TARGET_LINK_LIBRARIES(
    ${REALITY_DS_LIB64} 
    ${DS_CORE_LIB64}
    ${DS_QT_CORE64} 
    ${DS_QT_GUI64} 
    ${DS_QT_SCRIPT64}
    ${DS_QTNETWORK64}
    ${BOOST_THREAD}
    ${BOOST_SYSTEM}    
  )

ELSE()
  # Windows setup

  # Reality DLL
  TARGET_LINK_LIBRARIES(
    ${REALITY_LIB} 
    # ${QT_LIBRARIES} 
    ${QT_QTCORE_LIBRARY} 
    ${QT_QTNETWORK_LIBRARY} 
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${BOOST_THREAD}
    ${BOOST_SYSTEM}        
    "ws2_32.lib" 
    "RpcRT4.lib" 
    # ${BOOST_PYTHON}    
    # ${PYTHON_LIBRARY}
  )

  # Reality exe
  TARGET_LINK_LIBRARIES(
    ${REALITY_EXECUTABLE} 
    ${QT_LIBRARIES} 
    ${WRES_FILE}
    # debug "Debug/${REALITY_LIB}" ${BOOST_LIBS_DEBUG}
    # optimized "Release/${REALITY_LIB}" ${BOOST_LIBS} 
    ${REALITY_LIB}
    ${BOOST_PROGRAM_OPTIONS}
    ${BOOST_ATOMIC}
    ${BOOST_DATE_TIME}
    ${BOOST_REGEX}
    ${BOOST_CHRONO}
  )

  # Reality for DS DLL 32-bit
  TARGET_LINK_LIBRARIES(
    ${REALITY_DS_LIB32} 
    ${DS_QT_CORE32} 
    ${DS_CORE_LIB32}
    ${DS_QT_GUI32}
    # ${DS_QT_SQL32} 
    ${DS_QT_SCRIPT32}
    ${DS_QTNETWORK32}
    ${BOOST_THREAD}    
    ${BOOST_SYSTEM}   
    ${BOOST_DATE_TIME}
    ${BOOST_REGEX}
    ${BOOST_CHRONO}       
    "ws2_32.lib" 
    "RpcRT4.lib" 
  )

  # Reality for DS DLL64-bit
  TARGET_LINK_LIBRARIES(
    ${REALITY_DS_LIB64} 
    ${DS_QT_CORE64} 
    ${DS_CORE_LIB64}
    ${DS_QT_GUI64} 
    # ${DS_QT_SQL64} 
    ${DS_QT_SCRIPT64} 
    ${DS_QTNETWORK64}
    ${BOOST_THREAD}    
    ${BOOST_SYSTEM}    
    ${BOOST_DATE_TIME}
    ${BOOST_REGEX}
    ${BOOST_CHRONO}       
    "ws2_32.lib" 
    "RpcRT4.lib"
  )

ENDIF()  

# Reality uses a CRC value to verify that the program has not being
# tampered with. The value is a CRC of the whole executable. Since
# storing a value in the executable would change the computed crc,
# the control value, calulated at build time, is stored in the
# Reality dynamic library. The cycle therefore goes like this:
#
# - Compile the library and the executable
# - Compute the crc value for the executable and store it
#   in a C source whihc is included in the build of the library
# - Re-compile the library
#
# The tool used to compute the CRC for Reality is called compute_crc
# and it's built by the following commands

SET( COMPUTE_CRC_APP "compute_crc")
ADD_EXECUTABLE( ${COMPUTE_CRC_APP} tools/compute_crc.cpp core/crc.c )
ADD_CUSTOM_COMMAND(
  TARGET ${COMPUTE_CRC_APP}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${COMPUTE_CRC_APP}> ${CMAKE_SOURCE_DIR}/../tools
)
TARGET_LINK_LIBRARIES( ${COMPUTE_CRC_APP} ${QT_LIBRARIES} )
