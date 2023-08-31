/**
 * Reality plug-in.
 * Copyright (c) 2010-2011 and later Pret-a-3D/Paolo Ciccone. All rights reserved.
 */

/**
 * \file
 * Dialog box to notify of updates for Reality
 */

#ifndef RE_UPDATE_NOTIFICATION_H
#define RE_UPDATE_NOTIFICATION_H

#include "ui_ReUpdateNotification.h"


extern quint32 packVersionNumber( const QString vnum );
  
class ReUpdateNotification : public QDialog {
  Q_OBJECT

protected:
  Ui::ReUpdateNotification ui;

private:
    QString url;

public:
  ReUpdateNotification( QWidget* parent = 0 );
  ~ReUpdateNotification();

public slots:
  
  void setDescription(const QString desc ) {
    ui.revisionChanges->setHtml(desc);
  }
  void setBuildInfo(const QString currentBuildNo,
                    const QString newBuildNo  ) {
    ui.buildNumbers->setText(ui.buildNumbers->text().arg(newBuildNo).arg(currentBuildNo));
  };
  void setUrl(const QString url) {
      this->url = url;
  }

  void download();
};

#endif
