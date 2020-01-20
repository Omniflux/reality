/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#ifndef RE_ACSEL_SAVE_H
#define RE_ACSEL_SAVE_H

#include <QDialog>
#include "ui_reAcselSave.h"
namespace Reality {

/**
 * Dialog box that handles the saving of ACSEL shaders. 
 */
class ReAcselSave : public QDialog, public Ui::reAcselSave 
{
  Q_OBJECT
private:
  //! Avoid pestering the user if she has already confirmed that she
  //! understands the use of the "default config" checkbox
  static bool confirmedAlready;

  //! Validates that the dialog is filled out correctly
  bool validate();

  //! Show a message using the QMessageBox dialog
  void showMsg( const QString& msg );

  //! Storage for the object ID
  QString objectID;
  
  //! Storage for the set ID
  QString setID;

  //! Flag used to indicate that the shader set defined in the dialog
  //! is new
  bool isNewSet;
public:
  //! Constructor
  //! \param objectID The ID of the object for which we save the ACSEL shader
  //! \param setName The previously loaded ACSEL shader set. 
  //! \param parent The Qt widget parent of this dialog box
  ReAcselSave( const QString& objectID, 
               const QString& setName = "", 
               QWidget* parent = 0 );

  inline const QString getSetID() {
    return setID;
  }

  inline const QString getAuthor() {
    return author->text();
  }

  inline const QString getSetName() {
    return teSetName->text();
  }

  inline const QString getDescription() {
    return description->toPlainText();
  }

  inline bool isDefaultConfig() {
    return defaultConfig->isChecked();
  }

  inline bool mustSetMaterialTypeOnly() {
    return setMaterialTypeOnly->isChecked();
  }

  inline bool isNew() {
    return isNewSet;
  }

private slots:
  //! Resets all the fields to the initial state. This is used when the user 
  //! needs the "Save as..." feature for saving an ACSEL Shader. When creating
  //! variations of a set, like when applying makeup textures, it is necessary
  //! to save a shader in its own set. If we have a shader set already applied
  //! Reality will detect it and fill this form with the previous data. In 
  //! addition, Reality will prevent editing of the set name. While this is 
  //! very convenient and it reduces user's error, it would prevent the ability
  //! to save a shader for a make up texture. The Reset button is used to 
  //! reset the form to its black state and allow the user to save the shader
  //! into a different set.
  void resetForm();  

public slots:
  
  void updateDefaultConfig( bool onOff );

  void accept();
};

} // namespace

#endif
