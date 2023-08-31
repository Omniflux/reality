/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "RealityUI/ReUniversalShaderSelector.h"

#include <QPushButton>

#include "RealityUI/ReUniversalShaderWidget.h"


using namespace Reality;

ReUniversalShaderSelector::ReUniversalShaderSelector( QWidget* parent ) : 
  QDialog(parent)
{
  setupUi(this);
  auto vLayout = new QVBoxLayout(selectorContainer);
  vLayout->setObjectName(QString::fromUtf8("selectorLayout"));
  selector = new ReUniversalShaderWidget(selectorContainer);
  vLayout->addWidget(selector);
  selector->setMessage(tr("Select a shader set to apply to the selected material"));

  buttonBox->button(QDialogButtonBox::Ok)
    ->setEnabled(false);
  connect(selector, SIGNAL(shaderSelected(const QString&)), 
          this, SLOT(shaderSelectedHandler(const QString&)));

}

void ReUniversalShaderSelector::shaderSelectedHandler( const QString& shaderID ) {
  if (!shaderID.isEmpty()) {
    buttonBox->button(QDialogButtonBox::Ok)
      ->setEnabled(true);
  }
}
