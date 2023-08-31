/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
 */

#ifndef RE_BREADCRUMBS_H
#define RE_BREADCRUMBS_H

#include <QWidget>
#include <QVector>

#include "ui_reBreadCrumbs.h"

namespace Reality {
  class ReTexture;
  typedef QSharedPointer<ReTexture> ReTexturePtr;
}


using namespace Reality;

/**
 Class: ReBreadCrumbs

 This widget is used to show the user the levels of editing of a given texture.
 Textures in Reality can be nested at any level and so it's important to provide
 a mean for the use to know her level of nesting. The breadcrumbs system implements
 a classic "trail" of breadcrumbs. Each breadcrumb is a level of nesting.
 */
class ReBreadCrumbs: public QWidget, public Ui::wdg_breadcrumbs {
  Q_OBJECT

private:
  QVector<ReTexturePtr> crumbs;
  QVector<QString> crumbLabels;

public:
  /**
   * ctor
   */
  explicit ReBreadCrumbs( QWidget* parent = 0);
  QSize sizeHint() const;

  void clearTrail();

public slots:

  void addCrumb(const QString& label, const ReTexturePtr crumb);
  void removeCrumb();
  void updateCrumbs();

signals:
  //! Signal: back
  //!   When the user pressed the back button to go up one level
  //!   the class emits this signal and passes the pointer to the 
  //!   texture that becomes current.
  void back(ReTexturePtr tex);

  //! Signal: atHome
  //!   Emitted when the user presses the back button all the way to the
  //!   beginning of the trail.
  void atHome();
};

#endif
