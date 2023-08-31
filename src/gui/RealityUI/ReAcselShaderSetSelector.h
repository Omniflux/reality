/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#ifndef RE_ACSEL_SHADER_SET_SELECTOR_H
#define RE_ACSEL_SHADER_SET_SELECTOR_H

#include <QDialog>

#include "ui_reShaderSetSelector.h"


namespace Reality {

/**
 * Dialog box that handles the selection of a shader set to be applied to an object 
 */
class ReAcselShaderSetSelector : public QDialog, public Ui::shaderSetSelector
{
  Q_OBJECT
private:
  QString currentShaderSet;
  
public:
  //! Constructor
  //! \param parent The Qt widget parent of this dialog box
  ReAcselShaderSetSelector( QWidget* parent = 0 );

  inline QString getSelectedShaderSetID() {
    return currentShaderSet;
  }
  
private slots:
  void setSelectedShaderSet(QTreeWidgetItem* current,QTreeWidgetItem* previous);
};

} // namespace

#endif
