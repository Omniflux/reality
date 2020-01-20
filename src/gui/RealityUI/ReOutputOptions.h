/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef REOUTPUT_OPTIONS_H
#define REOUTPUT_OPTIONS_H

#include <QtCore>
#include <QSettings>
#include "ui_reOutputOptions.h"
#include "RealityBase.h"
#include "ReSceneData.h"

#include "ReOutputUpdater.h"
#include "ReSurfaceIntegrator.h"
#include "RePresetList.h"
#include "ReDefs.h"

using namespace Reality;

//! Table used to feed the combobox used for the Lux verbosity levels
class LuxVerbosityLevels : public RePresetList {

public:

  void initTable() {
    presetMap[LUX_WARNINGS]    = QObject::tr("Warnings only (default)");
    presetMap[LUX_INFORMATION] = QObject::tr("Information");
    presetMap[LUX_DEBUG]       = QObject::tr("Debug information");
    presetMap[LUX_ERRORS]      = QObject::tr("Errors only");
  }
};

//! Table used to feed the combobox used for the Lux geometry formats
class LuxGeometryFormats : public RePresetList {

public:

  void initTable() {
    presetMap[LuxNative] = QObject::tr("Lux Text");
    presetMap[BinaryPLY] = QObject::tr("PLY Binary");
    presetMap[TextPLY]   = QObject::tr("PLY Text");
  }
};

//! Table used to feed the combobox used for the Lux samplers
class RenderSamplers : public RePresetList {

public:

  void initTable() {
    presetMap[metropolis] = "Metropolis";
    presetMap[sobol] = "Sobol";
  }
};

//! Table used to feed the combobox used for the image file formats
class ImageFileFormats : public RePresetList {

public:

  void initTable() {
    presetMap[PNG] = QObject::tr("PNG");
    presetMap[EXR] = QObject::tr("EXR");
    presetMap[TGA] = QObject::tr("Targa");
  }
};

//! Table used to feed the combobox used to select Surface Integrators
class LuxSIPresets : public RePresetList {

public:

  void initTable() {
    presetMap[SI_BIDIR] = QObject::tr("Bidirectional");
    presetMap[SI_PATH]  = QObject::tr("Mono-directional");
  }
};

class ReOutputOptions: public QWidget, 
                       public Ui::reOutputOptions,
                       public OutputUIUpdater 
{
  Q_OBJECT

private:

  ReConfigurationPtr config;
  bool updatingUI;
  
  LuxVerbosityLevels luxLogLevels;
  LuxGeometryFormats luxGeometryFormats;
  LuxSIPresets luxSIPresets;
  ImageFileFormats imageFileFormats;
  RenderSamplers renderSamplers;

  // //! The patrameter group that is currently selected.
  // QListWidgetItem* currentParameterGroup;

public:
  explicit ReOutputOptions( QWidget* parent = 0);
  QSize sizeHint() const;

  void updateUI();

  // Implementation of the OutputUIUpdater interface
  void showGamma( const double newGamma );
  void showAlphaChannel( const bool newAlpha );
  void showMaxSPX( const quint32 newVal );
  void showOutlierRejection( const quint16 newVal );
  void showImageFileFormat( const ExportImageFileFormat newVal );
  void showPreserveLightHue( const bool newVal );
  void showFrameMultiplier( const float newVal );
  void showGeometryFormat( const GeometryFileFormat newVal );
  // void showSceneFileName( const QString newVal );
  void showImageFileName( const QString newVal );

  void setImageExtensionForFormat( ExportImageFileFormat newFormat );
  
private:
  //! Show the correct Surface Integrator editor for the 
  //! current selection of SI.
  void setSIEditor();

private slots:
  // void setLuxRenderer( bool active );

  // // Change the path to Lux
  // void updateLuxPath();

  void parameterGroupSelected();
  void updateAlphaChannel( bool val );
  void updateDisplacementFlag(bool onOf );
  //! Called when the user changed the spinner for the frame size %
  void updateFrameSize( int newPercent );
  void updateGamma( double val );
  void updateGeometryFormat(int index);
  void updateImageFormat(int index);
  //! Called to set the render mode based on the user's choice
  void updateRenderMode( bool val );
  
  //! Called to set or unset the biased rendering option.
  void updateBiasedRender( bool isON );

  void updateImageFileName( const QString& val );
  void updateLightHueFlag( bool onOff );
  void updateNumThreads(int val);
  void updateOverwriteWarning( bool onOff );
  // void updateSceneFileName( const QString& val );
  //! Called when the user selectes a new "Scene Type" or SurfaceIntegrator
  void updateSceneType( int newIndex );
  void updateMaxSamples(int val);

  //! Called when the user clicks the button to select a location for the image file
  void updateImageFileName();

  // //! Called when the user clicks the button to select a location for the scene file
  // void updateSceneFileName();

  void updateFFRemoval(int val);
  void updateNoiseAware(bool val);

  // Surface Integrator editor
  void updateSIValue(int newVal);
  void updateSIValue(double newVal);
  //! Update the Light Strategy parameter
  void updateLS( bool newVal );
  void updateExPMRendering( bool isOn );

  //! Update the sampler
  void updateSampler( int index );

  //! Update the size of the OCL workgroup
  void updateOCLGroupSize(int newVal);

  //! Enable/disable a given OCL device
  void updateOCLDeviceStatus(QListWidgetItem* item );
  
  // //! Sets the number of GPUs installed in the machine
  // void setNumGPUs(int numGPUs);
  // //! Updates the configuration to enable/disable a give OpenCL device
  // void updateOCLDeviceNo(bool onOff);

  void updateTextureCollection(bool isOn);
  void updateTextureSize();

  //! Called to set the verbosity of the Lux log
  void setLuxLogLevel(const QString & level);

  //! Called to set rendering the scene as white marble
  void updateRenderAsStatue( bool onOff );
  
  void renderAnimationWithRange();
  
public slots:
  void showFrameSize();

signals:
  void outputDataChanged();
  void renderFrame();
  void renderAnimation(int startFrame, int endFrame);

};

#endif
