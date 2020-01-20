/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReMatteEditorPlugin.h"
#include "../MaterialEditors/ReMatteEditor.h"


ReMatteEditorPlugin::ReMatteEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReMatteEditorPlugin::name() const        { return "ReMatteEditor"; }
QString ReMatteEditorPlugin::includeFile() const { return "ReMatteEditor.h"; }
QString ReMatteEditorPlugin::group() const       { return "Reality"; }
QIcon   ReMatteEditorPlugin::icon() const        { return QIcon(); }
QString ReMatteEditorPlugin::toolTip() const     { return "The Reality Matte Editor"; }
QString ReMatteEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReMatteEditorPlugin::isContainer() const { return false; }

QWidget* ReMatteEditorPlugin::createWidget(QWidget* parent) {
  return new ReMatteEditor(parent);
}
