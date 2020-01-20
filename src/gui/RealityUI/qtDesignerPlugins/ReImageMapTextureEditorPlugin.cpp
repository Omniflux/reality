/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReImageMapTextureEditorPlugin.h"
#include "../ReImageMapTextureEditor.h"


ReImageMapTextureEditorPlugin::ReImageMapTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReImageMapTextureEditorPlugin::name() const        { return "ReImageMapTextureEditor"; }
QString ReImageMapTextureEditorPlugin::includeFile() const { return "ReImageMapTextureEditor.h"; }
QString ReImageMapTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReImageMapTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReImageMapTextureEditorPlugin::toolTip() const     { return "Editor for the ImageMap Texture"; }
QString ReImageMapTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReImageMapTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReImageMapTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReImageMapTextureEditor(parent);
}
