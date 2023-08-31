  /**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "RealityUI/ReAcselSave.h"

#include <QMessageBox>
#include <QSettings>

#include "ReAcsel.h"
#include "RealityBase.h"
#include "RealityUI/ReConfirmationDialog.h"


using namespace Reality;

// static vars initialization
bool ReAcselSave::confirmedAlready = false;

ReAcselSave::ReAcselSave( const QString& objectID, 
                          const QString& setID, 
                          QWidget* parent ) 
  : QDialog(parent)
  , objectID(objectID)
  , setID(setID)
{
  setupUi(this);
  isNewSet = true;
  description->setAcceptRichText(false);

  auto cfg = RealityBase::getConfiguration();
  author->setText(cfg->value(RE_CFG_AUTHOR_NAME,"").toString());
  if (!setID.isEmpty()) {
    auto acsel = ReAcsel::getInstance();
    QVariantMap data;
    if (acsel->findShaderSet(setID, data)) {
      teSetName->setText(data.value("Name").toString());
      teSetName->setEnabled(false);
      isNewSet = false;
      description->setText(data.value("Description").toString());
    }
  }
  confirmedAlready = !cfg->value(RE_CFG_GENERIC_FLAG_REMINDER).toBool();

  connect(defaultConfig, SIGNAL(toggled(bool)), this, SLOT(updateDefaultConfig(bool)));
  connect(btnReset, SIGNAL(clicked()), this, SLOT(resetForm()));

  setMaterialTypeOnly->setEnabled(false);
};

void ReAcselSave::updateDefaultConfig( bool onOff ) {
  if (onOff && !confirmedAlready) {
    bool showAgain = false;
    if ( !ReConfirmationDialog::ask(
            this,
            showAgain,
            tr("Confirmation needed"),
            tr(
             "Enabling this option will make this preset "
             "load for this model regardless of what textures are applied.\n"
             "Are you sure that you want this?"
            )
         )
       )
    {
      defaultConfig->setChecked(false);
      return;
    }
    confirmedAlready = true;
    RealityBase::getConfiguration()->setValue(RE_CFG_GENERIC_FLAG_REMINDER, 
                                              showAgain);
  }
  teSetName->setEnabled(!onOff);
  if (onOff) {
    // Find out if there is a default shader for this object already
    ReAcsel* acsel = ReAcsel::getInstance();
    QVariantMap data;
    if (acsel->findDefaultShaderSet(objectID, data)) {
      setID = data.value("SetID").toString();
      teSetName->setText(data.value("SetName").toString());
      description->setText(data.value("Description").toString());
      defaultConfig->setEnabled(false);
      isNewSet = false;
    }
    else {
      setID = "";
    }
  }
  setMaterialTypeOnly->setEnabled(defaultConfig->isChecked());
}

void ReAcselSave::accept() {
  if (validate()) {
    QDialog::accept();
  }
}

void ReAcselSave::showMsg( const QString& msg ) {
  QMessageBox::information( this, tr("Information"), msg);
}

bool ReAcselSave::validate() {
  if (!defaultConfig->isChecked()) {
    if (teSetName->text().isEmpty()) {
      teSetName->setFocus();
      showMsg( tr( "The name for the set must be specified" ) );
      return false;
    }
  }
  if (description->toPlainText().isEmpty()) {
    showMsg(tr("You need to enter a description"));
    description->setFocus();
    return false;
  }
  if (author->text().isEmpty()) {
    showMsg(tr("You need to enter the author's name"));
    author->setFocus();
    return false;
  }

  return true;
}

void ReAcselSave::resetForm() {
  auto retval = QMessageBox::question(
         this,
         tr("Confirmation"),
         tr("Are you sure that you want to empty this form?"),
         QMessageBox::Yes | QMessageBox::No,
         QMessageBox::No
       );
  if (retval != QMessageBox::Yes ) {
    return;
  }
  teSetName->setText("");
  teSetName->setEnabled(true);
  description->setText("");
  isNewSet = true;
}
