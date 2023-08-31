/**********************************************************************
  Copyright © 2010-2011 Pret-A-3d
**********************************************************************/

#include "RealityAction.h"

#include <QFileInfo>
#include <QSettings>
#include <dzapp.h>

#include "RealityRunner.h"
#include "ReDefs.h"


namespace Reality {

#define RE_MENU_ICON_FILE_NAME "R-flat-logo-menu.png"
/**
 * Action to install the launch of Reality in the Render menu of Studio
 */
Reality3Action::Reality3Action() : DzAction( tr( "&Reality Render Editor" ),
                                             tr( "Launch the Reality Render Editor" ) ) 
{
  QString iconFilePath = QString("%1/Reality/%2")
                           .arg(dzApp->getPluginsPath())
                           .arg(RE_MENU_ICON_FILE_NAME);

  QFileInfo info(iconFilePath);
  if (!info.exists()) {
    QPixmap icon(QString(":/images/%1").arg(RE_MENU_ICON_FILE_NAME));
    icon.save(iconFilePath);
  }
  setIcon(iconFilePath);
  styleChange();
}

void Reality3Action::executeAction() {
  QSettings configuration(RE_CFG_DOMAIN_NAME, RE_CFG_APPLICATION_NAME);
  QString realityPath = configuration.value(RE_CFG_REALITY_DS_LOCATION).toString();
  RealityRunner::getInstance()->launchGUI(realityPath);
}

} // namespace
