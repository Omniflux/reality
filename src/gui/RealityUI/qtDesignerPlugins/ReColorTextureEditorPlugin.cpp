/**
 * Qt Designer plugin.
 */

#include "ReColorTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReColorTextureEditor.h"


ReColorTextureEditorPlugin::ReColorTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReColorTextureEditorPlugin::name() const        { return "ReColorTextureEditor"; }
QString ReColorTextureEditorPlugin::includeFile() const { return "ReColorTextureEditor.h"; }
QString ReColorTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReColorTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReColorTextureEditorPlugin::toolTip() const     { return "Editor for the Color Texture"; }
QString ReColorTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReColorTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReColorTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReColorTextureEditor(parent);
}
