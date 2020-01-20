/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReTextureEditorPlugin.h"
#include "../ReTextureEditor.h"


ReTextureEditorPlugin::ReTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReTextureEditorPlugin::name() const        { return "ReTextureEditor"; }
QString ReTextureEditorPlugin::includeFile() const { return "ReTextureEditor.h"; }
QString ReTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReTextureEditorPlugin::toolTip() const     { return "The Reality Texture Editor widget"; }
QString ReTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReTextureEditor(parent);
}
