/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReCameraEditorPlugin.h"
#include "ReCameraEditor.h"


ReCameraEditorPlugin::ReCameraEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReCameraEditorPlugin::name() const        { return "ReCameraEditor"; }
QString ReCameraEditorPlugin::includeFile() const { return "ReCameraEditor.h"; }
QString ReCameraEditorPlugin::group() const       { return "Reality"; }
QIcon   ReCameraEditorPlugin::icon() const        { return QIcon(); }
QString ReCameraEditorPlugin::toolTip() const     { return "Editor for the Output options"; }
QString ReCameraEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReCameraEditorPlugin::isContainer() const { return false; }

QWidget* ReCameraEditorPlugin::createWidget(QWidget* parent) {
  return new ReCameraEditor(parent);
}
