/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_UNIVERSAL_SHADER_WDG_H
#define RE_UNIVERSAL_SHADER_WDG_H

#include <QSharedPointer>

#include "ui_reUniversalShaderWdg.h"

namespace Reality {
  class ReShaderCategoryPreset;
  typedef QSharedPointer<ReShaderCategoryPreset> ReShaderCategoryPresetPtr;
}


namespace Reality {

/**
 * Embeddable, re-usable widget that shows the Universal Shaders 
 */

class ReUniversalShaderWidget : public QWidget, 
                                public Ui::universalShaderWidget
{
  Q_OBJECT
private:
  QString currentShader;
  ReShaderCategoryPresetPtr categoryPreset;

  typedef QList<QStringList> ShaderCollection;

  bool updatingUI;
public:
  //! Constructor
  //! \param parent The Qt widget parent of this dialog box
  ReUniversalShaderWidget( QWidget* parent = 0 );

  inline QString getSelectedShaderID() {
    return currentShader;
  }

  inline QString getSelectedCategory() {
    return categories->currentText();
  }

  QStringList getSelectedShaderIDs();

  inline void setMessage( const QString& msg ) {
    message->setText(msg);
  }

  void displayShaders( const QString& category = "" );

  inline void enableMultipleSelections( bool enabled ) {
    if (enabled) {
      shaderList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
    else {
      shaderList->setSelectionMode(QAbstractItemView::SingleSelection);
    }
  }
private slots:
  void setSelectedShaderSet(QTreeWidgetItem* current, 
                            QTreeWidgetItem* previous);
  //! Updates the list of available shaders in response to the user 
  //! selecting a different category
  void updateShaderList( const QString& newCat );

signals:
  //! Signaled when a shader is selected
  void shaderSelected( const QString& shaderID );

};

}

#endif
