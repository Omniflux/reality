/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReImageMapManagerPlugin.h"


ReImageMapManagerPlugin::ReImageMapManagerPlugin(QObject* parent) : QObject(parent) {

}

QString ReImageMapManagerPlugin::name() const        { return "ReImageMapManager"; }
QString ReImageMapManagerPlugin::includeFile() const { return "ReImageMapManager.h"; }
QString ReImageMapManagerPlugin::group() const       { return "Reality"; }
QIcon   ReImageMapManagerPlugin::icon() const        { return QIcon(); }
QString ReImageMapManagerPlugin::toolTip() const     { return "The Reality ImageMap manager"; }
QString ReImageMapManagerPlugin::whatsThis() const   { return toolTip(); }
bool    ReImageMapManagerPlugin::isContainer() const { return false; }

QWidget* ReImageMapManagerPlugin::createWidget(QWidget* parent) {
  return new ReImageMapManager(parent);
}
