/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReVolumesEditorPlugin.h"
#include "../MaterialEditors/ReVolumesEditor.h"


ReVolumesEditorPlugin::ReVolumesEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReVolumesEditorPlugin::name() const        { return "ReVolumesEditor"; }
QString ReVolumesEditorPlugin::includeFile() const { return "ReVolumesEditor.h"; }
QString ReVolumesEditorPlugin::group() const       { return "Reality"; }
QIcon   ReVolumesEditorPlugin::icon() const        { return QIcon(); }
QString ReVolumesEditorPlugin::toolTip() const     { return "The Reality Volumes Editor"; }
QString ReVolumesEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReVolumesEditorPlugin::isContainer() const { return false; }

QWidget* ReVolumesEditorPlugin::createWidget(QWidget* parent) {
  return new ReVolumesEditor(parent);
}
