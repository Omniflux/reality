/**
 * Qt Designer plugin.
 */

#include "ReMatVolumesEditorPlugin.h"

#include <QtPlugin>

#include "../MaterialEditors/ReMatVolumesEditor.h"


ReMatVolumesEditorPlugin::ReMatVolumesEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReMatVolumesEditorPlugin::name() const        { return "ReMatVolumesEditor"; }
QString ReMatVolumesEditorPlugin::includeFile() const { return "ReMatVolumesEditor.h"; }
QString ReMatVolumesEditorPlugin::group() const       { return "Reality"; }
QIcon   ReMatVolumesEditorPlugin::icon() const        { return QIcon(); }
QString ReMatVolumesEditorPlugin::toolTip() const     { return "The Reality MatVolumes Editor"; }
QString ReMatVolumesEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReMatVolumesEditorPlugin::isContainer() const { return false; }

QWidget* ReMatVolumesEditorPlugin::createWidget(QWidget* parent) {
  return new ReMatVolumesEditor(parent);
}
