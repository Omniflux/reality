/**
 * Qt Designer plugin.
 */

#include "ReCloudsTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReCloudsTextureEditor.h"


ReCloudsTextureEditorPlugin::ReCloudsTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReCloudsTextureEditorPlugin::name() const        { return "ReCloudsTextureEditor"; }
QString ReCloudsTextureEditorPlugin::includeFile() const { return "ReCloudsTextureEditor.h"; }
QString ReCloudsTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReCloudsTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReCloudsTextureEditorPlugin::toolTip() const     { return "Editor for the Clouds Texture"; }
QString ReCloudsTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReCloudsTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReCloudsTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReCloudsTextureEditor(parent);
}
