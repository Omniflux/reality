/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef SETIMAGEMAPACTION_H
#define SETIMAGEMAPACTION_H

#include "ReAction.h"
#include "RealityBase.h"
#include "textures/ReImageMap.h"

#include <QtCore>
#include <QFileInfo>
#include <QDesktopServices>
#include <QFileDialog>

namespace Reality {


/**
 * Action to set the file name for an image map
 */
class ReSetImageMapAction : public ReAction {

private:
  //! The initial file name, used to change directory before opening
  //! the File | Open dialog box. In this way the user can start 
  //! browsing conveniently from the same location of the original file.
  QString fileName;

  //! Flag used to verify is the new name selected is different from
  //! what was used before.
  bool nameHasChanged;

public:
  //! Constructor: ReSetImageMapAction
  ReSetImageMapAction( const QString& fileName ) : fileName(fileName) {
  };

  inline void execute() {
    nameHasChanged = false;
    QFileInfo fInfo;
    QString location;
    ReConfigurationPtr config = RealityBase::getConfiguration();
    if (!fileName.isEmpty()) {
      fInfo.setFile(fileName);
      location = fInfo.absolutePath();
    }
    else {
      location = config->value(
        RE_CFG_LAST_BROWSED_DIR,
        QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)
      ).toString();
    }
    fInfo.setFile(location);
    if (!fInfo.exists()) {
      location = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
    }
    QDir::setCurrent(location);

    QString fName = QFileDialog::getOpenFileName(
                                  0, 
                                  tr("Select an Image file for the texture"),
                                  location,
                                  tr("Image files (*.png *.jpg *.tif *.tiff)")
                                );

    if (fName != "") {
      if (fName != fileName) {
        nameHasChanged = true;
        fileName = fName;
        QFileInfo newFInfo(fName);
        config->setValue(RE_CFG_LAST_BROWSED_DIR, newFInfo.absolutePath());
      }
    }
  }

  inline bool hasNameChanged() {
    return nameHasChanged;
  }

  inline QString getFileName() {
    return fileName;
  }
};

} // namespace

#endif
