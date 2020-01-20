/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReWaterEditorPlugin.h"
#include "../MaterialEditors/ReWaterEditor.h"


ReWaterEditorPlugin::ReWaterEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReWaterEditorPlugin::name() const        { return "ReWaterEditor"; }
QString ReWaterEditorPlugin::includeFile() const { return "ReWaterEditor.h"; }
QString ReWaterEditorPlugin::group() const       { return "Reality"; }
QIcon   ReWaterEditorPlugin::icon() const        { return QIcon(); }
QString ReWaterEditorPlugin::toolTip() const     { return "The Reality Water Editor"; }
QString ReWaterEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReWaterEditorPlugin::isContainer() const { return false; }

QWidget* ReWaterEditorPlugin::createWidget(QWidget* parent) {
  return new ReWaterEditor(parent);
}
