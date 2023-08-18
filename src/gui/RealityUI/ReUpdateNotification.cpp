/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2013. All rights reserved.    
*/

#include "ReUpdateNotification.h"
#include <QtGui/QDesktopServices>

ReUpdateNotification::ReUpdateNotification( QWidget* parent ) : QDialog(parent) {
  url = QString::fromUtf8(HOMEPAGE_URL);
  ui.setupUi(this);
  connect(ui.btnClose,SIGNAL(clicked()),this,SLOT(reject()));
  connect(ui.btnDownload,SIGNAL(clicked()),this,SLOT(download()));
};

ReUpdateNotification::~ReUpdateNotification() {

};

void ReUpdateNotification::download() {
  QDesktopServices::openUrl(url);
}