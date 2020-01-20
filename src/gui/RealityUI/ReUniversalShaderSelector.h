/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#ifndef RE_UNIVERSAL_SHADER_SELECTOR_H
#define RE_UNIVERSAL_SHADER_SELECTOR_H

#include <QDialog>
#include "ui_reUniversalShaderSelector.h"
#include "ReUniversalShaderWidget.h"

namespace Reality {

/**
 * Dialog box that handles the selection of a universal shader to be 
 * applied to a material
 */
class ReUniversalShaderSelector : public QDialog, 
                                  public Ui::universalShaderSelector
{
  Q_OBJECT
private:
  //! The instance of the selector widget
  ReUniversalShaderWidget* selector;
public:
  //! Constructor
  //! \param parent The Qt widget parent of this dialog box
  ReUniversalShaderSelector( QWidget* parent = 0 );

  inline QString getSelectedShaderID() {
    return selector->getSelectedShaderID();
  }
private slots:

  void shaderSelectedHandler( const QString& shaderID );
};

} // namespace

#endif
