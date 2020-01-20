/**
 * The Reality texture manager.
 */
#include <QObject>
#include "ReBreadCrumbs.h"

#define RE_BREADCRUMB_SEP "|"

ReBreadCrumbs::ReBreadCrumbs(QWidget* parent) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  connect(btnPrev, SIGNAL(clicked()), this, SLOT(removeCrumb()));
}

QSize ReBreadCrumbs::sizeHint() const {
  return QSize(230,22);
}

void ReBreadCrumbs::addCrumb(const QString& label, const ReTexturePtr crumb) {
  crumbs.append(crumb);
  crumbLabels.append(label);
  updateCrumbs();
}

void ReBreadCrumbs::removeCrumb() {
  crumbs.remove(crumbs.count()-1);
  crumbLabels.remove(crumbLabels.count()-1);
  updateCrumbs();
  if (crumbs.count() > 0 ) {
    emit back(crumbs.last());
  }
  else {
    emit atHome();
  }
}

void ReBreadCrumbs::updateCrumbs() {
  trail->clear();
  QString trailStr;
  qint16 i = 0;
  foreach(QString crumbLbl, crumbLabels) {
    if (i++ > 0) {
      trailStr += RE_BREADCRUMB_SEP;
    }
    trailStr += crumbLbl;
  }
  trail->setText(trailStr);
}

void ReBreadCrumbs::clearTrail() {
  trail->setText("");
  crumbs.clear();
  crumbLabels.clear();
}
