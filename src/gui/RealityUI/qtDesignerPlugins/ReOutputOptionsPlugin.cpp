/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReOutputOptionsPlugin.h"
#include "../ReOutputOptions.h"


ReOutputOptionsPlugin::ReOutputOptionsPlugin(QObject* parent) : QObject(parent) {

}

QString ReOutputOptionsPlugin::name() const        { return "ReOutputOptions"; }
QString ReOutputOptionsPlugin::includeFile() const { return "ReOutputOptions.h"; }
QString ReOutputOptionsPlugin::group() const       { return "Reality"; }
QIcon   ReOutputOptionsPlugin::icon() const        { return QIcon(); }
QString ReOutputOptionsPlugin::toolTip() const     { return "Editor for the Output options"; }
QString ReOutputOptionsPlugin::whatsThis() const   { return toolTip(); }
bool    ReOutputOptionsPlugin::isContainer() const { return false; }

QWidget* ReOutputOptionsPlugin::createWidget(QWidget* parent) {
  return new ReOutputOptions(parent);
}
