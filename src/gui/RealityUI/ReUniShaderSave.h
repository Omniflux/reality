/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#ifndef RE_UNISHADER_SAVE_H
#define RE_UNISHADER_SAVE_H

#include "ReMaterial.h"
#include "ReLuxRunner.h"
#include "RePresetList.h"
#include "ReShaderCategoryPreset.h"

#include <QDialog>
#include "ui_reUniShaderSave.h"

namespace Reality {

//! A list of presets for the inner sphere of the material. This is
//! used to manage the combobox that allows the user to select what
//! material is used to render the inner sphere
class ReInnerSpherePresets : public RePresetList {
private:
  //! The inner sphere of the preview can have one out of a few
  //! pre-defined materials. To store the material definitions we
  //! use a table keyed by the numeric identifier for the type of 
  //! material. The data is kept as a string defining the LuxRender
  //! material that will be injected in the previee scene
  RePresetMap innerMaterialPresets;

public:
  enum InnerSphereMatType {
    WhiteMatte,
    DarkMatte,
    SameAsOuterSphere,
    ShinyMetal
  };

  ReInnerSpherePresets() {
    initTable();
  }

  void initTable() {
    presetMap[WhiteMatte]        = QObject::tr("White matte");
    presetMap[DarkMatte]         = QObject::tr("Dark matte");
    presetMap[SameAsOuterSphere] = QObject::tr("Same as outer sphere");
    presetMap[ShinyMetal]        = QObject::tr("Shiny metal");

    innerMaterialPresets[WhiteMatte] = 
      "MakeNamedMaterial \"luxball stand\" \"string type\" [\"matte\"]"
      " \"color Kd\" [0.8 0.8 0.8] \"float sigma\" [0.4]\n";
    innerMaterialPresets[DarkMatte] = 
      "MakeNamedMaterial \"luxball stand\" \"string type\" [\"matte\"]"
      " \"color Kd\" [0.04 0.04 0.04] \"float sigma\" [0.4]\n";
    innerMaterialPresets[SameAsOuterSphere] = "";
    innerMaterialPresets[ShinyMetal] = 
      "Texture \"shiny-metal\" \"fresnel\" \"fresnelcolor\" \"color Kr\""
      " [0.8 0.8 0.8]\n"
      "MakeNamedMaterial \"luxball stand\" \"string type\" [\"metal2\"]"
      " \"texture fresnel\" [\"shiny-metal\"]  \"float uroughness\" [0.08]"
      " \"float vroughness\" [0.08]\n";
  }

  QString getMaterial( InnerSphereMatType innerMatID ) {
    return innerMaterialPresets[innerMatID];
  }
};

/**
 * Dialog box that handles the saving of Universal Shaders
 */
class ReUniShaderSave : public QDialog, public Ui::reUniShaderSave 
{
  Q_OBJECT
private:
  //! If the user selectes an existing shader to be updated then the 
  //! ID of that shader will be stored in this variable.
  QString shaderID;

  //! Validates that the dialog is filled out correctly
  bool validate();

  //! Show a message using the QMessageBox dialog
  void showMsg( const QString& msg );

  //! Pointer to the material for which this shader has been defined
  ReMaterial* mat;

  //! The reference to the renderer process used to create the preview
  ReLuxRunnerPtr renderProc;

  //! Flag used to keep track of when Reality is in the middle of 
  //! preparing a preview for the material.
  bool creatingPreview;

  //! Used to save the text of the preview button before we change it.
  QString previewButtonOriginalText;

  //! Directory name of where the content for Reality is stored.
  QString contentDir;

  //! The interval, in seconds, after which Reality refreshes the preview
  int refreshInterval;

  QTimer refreshTimer;
  //! An image that is used to notify the user that the material preview
  //! is being rendered
  QPixmap renderingThumbNail;

  //! The name of the file on disk that holds the material preview
  QString previewFileName;

  //! The estimated time for rendering the preview is stored so that
  //! Reality can perform the real-time update of the progress bar
  int estimatedRenderTime;

  //! Used to measure the actual time spend it creating the preview
  QTime stopwatch;
  
  //! The presets for the inner sphere material. This is handled by 
  //! a combobox
  ReInnerSpherePresets innerSpherePresets;

  QSharedPointer<ReShaderCategoryPreset> categoryPresets;

public:
  //! Constructor
  //! \param setName The previously loaded Universal Shader.
  //! \param parent The Qt widget parent of this dialog box
  ReUniShaderSave( ReMaterial* mat, QWidget* parent = 0 );

  inline const QString getShaderID() {
    return shaderID;
  }

  inline const QString getAuthor() {
    return leAuthor->text();
  }

  inline const QString getName() {
    return leName->text();
  }

  inline const QString getDescription() {
    return teDescription->toPlainText();
  }

  inline const QString getPreviewFileName() {
    return previewFileName;
  }

  inline const QString getCategory() {
    return categories->currentText();
  }

private slots:
  //! Calls the ReUniversalShaderSelector dialog to find the name of a 
  //! Universal Shader to use to save the data
  void searchShader();

  //! Creates the material preview to save with the shader
  void createMaterialPreview();

  //! When a preview is requested this method refreshes the preview 
  //! thumbnail at a determined interval of time.
  void updatePreview();

  //! Adjusts the number of samples set for the preview based on the
  //! input from the user
  void setPreviewQuality(int newVal);
public slots:
  //! Overridden method to validate the dialog box
  void accept();

  //! Overridden method to clean up the timer
  void reject();
};

} // namespace

#endif
