/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_EXPORT_PROGRESS_DLG_H
#define RE_EXPORT_PROGRESS_DLG_H

#include <QDialog>
#include "ui_reExportProgress.h"

class ReExportProgressDialog : public QDialog, public Ui::reExportProgress
{
   Q_OBJECT

public:
   ReExportProgressDialog(QWidget *parent = 0) : QDialog(parent) {
     setupUi(this);     
   }

public:

  void showMessage( const QString& msg ) {
    message->setText(msg);
  }

  void increment() {
    progressBar->setValue(progressBar->value()+1);
  }

  void setUpperLimit( int limit ) {
    progressBar->setMaximum(limit);
  }

  void reset() {
    progressBar->setValue(0);
  }
};

#endif
