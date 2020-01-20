/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_COFIRMATION_DIALOG_H
#define RE_COFIRMATION_DIALOG_H

#include <QDialog>
#include "ui_reConfirmationDialog.h"

/**
 *  Simple dialog that asks for a confirmation but that also provides a checkbox
 *  to let the user signal that she doesn't want to see this message again.
 */
class ReConfirmationDialog : public QDialog, public Ui::reConfirmationDialog 
{
  Q_OBJECT
public:
  //! Constructor: ReConfirmationDialog
  ReConfirmationDialog( QWidget* parent = 0 ) : QDialog(parent) {
    setupUi(this);
  };

  //! Destructor: ReConfirmationDialog
 ~ReConfirmationDialog() {
  };

  void setMessage( const QString& msg ) {
    message->setText(msg);
  }

  void setShowAgainMessage( const QString& msg ) {
    cbStopAsking->setText(msg);
  }

  bool showAgain() {
    return !cbStopAsking->isChecked();
  }

  static bool ask( QWidget* parent, 
                   bool& showAgain,
                   const QString& caption, 
                   const QString& msg,
                   const QString& showAgainMsg = ""
                 ) 
  {
    auto dlg = new ReConfirmationDialog( parent );
    dlg->setWindowTitle(caption);
    dlg->setMessage(msg);
    if (!showAgainMsg.isEmpty()) {
      dlg->setShowAgainMessage(showAgainMsg);
    }
    bool result = dlg->exec();
    showAgain = dlg->showAgain();
    return result;
  }
};


#endif
