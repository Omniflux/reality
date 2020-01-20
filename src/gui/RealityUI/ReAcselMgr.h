/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#ifndef RE_ACSEL_MGR_H
#define RE_ACSEL_MGR_H

#include <QDialog>
#include "ui_reAcselMgr.h"
#include "ReUniversalShaderWidget.h"

namespace Reality {

/**
 * The ACSEL Manager dialog
 */
class ReAcselManager : public QDialog, public Ui::reAcselMgr
{
  Q_OBJECT

public:
  //! Constructor: ReAcselManager
  ReAcselManager( QWidget* parent = 0 );

  //! Destructor: ReAcselManager
 ~ReAcselManager() {
  };

protected:
  void closeEvent( QCloseEvent *event );

private:
  //! The instance of the selector widget
  ReUniversalShaderWidget* usSelector;

  //! Loads the data into the UI
  void updateUI();

  //! Variable sued to keep track if we loaded the ACSEL web page
  bool urlLoaded;

  void loadAcselShaders();

private slots:
  void deleteShaderSet();

  //! Delete a group of selected Universal Shaders
  void deleteUniversalShaders();

  void importBundle();

  void exportShaderSets() {
    exportShaders(true, false);
  };

  void exportUniversalShaders() {
    exportShaders(false, true);
  };


  //! Exports the selected ACSEL or Universal Shaders to a bundle  
  //! \param saveAcselShaders If true the selected ACSEL shaders will be
  //!                         saved in the bundle
  //! \param saveUniversalShaders If true the selected Universal shaders will
  //!                             be saved in the bundle
  void exportShaders( const bool saveAcselShaders, 
                      const bool saveUniversalShaders );
  
  void editShaderSet();

  //! Called when the user clicks on a tab
  void switchTab( int which );

  //! Handler for when the selection of shader sets changes
  void handleSelection(const QItemSelection& selected, 
                       const QItemSelection& deselected);

  //! Copies the list of shaders to the Clipboard
  void copyShaderList();
};

} // namespace

#endif
