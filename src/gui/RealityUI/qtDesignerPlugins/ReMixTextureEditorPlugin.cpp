/**
 * Qt Designer plugin.
 */

#include "ReMixTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReMixTextureEditor.h"


ReMixTextureEditorPlugin::ReMixTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReMixTextureEditorPlugin::name() const        { return "ReMixTextureEditor"; }
QString ReMixTextureEditorPlugin::includeFile() const { return "ReMixTextureEditor.h"; }
QString ReMixTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReMixTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReMixTextureEditorPlugin::toolTip() const     { return "Editor for the Mix Texture"; }
QString ReMixTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReMixTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReMixTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReMixTextureEditor(parent);
}
