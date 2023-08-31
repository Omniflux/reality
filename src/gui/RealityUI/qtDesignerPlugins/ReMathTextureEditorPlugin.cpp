/**
 * Qt Designer plugin.
 */

#include "ReMathTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReMathTextureEditor.h"


ReMathTextureEditorPlugin::ReMathTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReMathTextureEditorPlugin::name() const        { return "ReMathTextureEditor"; }
QString ReMathTextureEditorPlugin::includeFile() const { return "ReMathTextureEditor.h"; }
QString ReMathTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReMathTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReMathTextureEditorPlugin::toolTip() const     { return "Editor for the Math Texture"; }
QString ReMathTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReMathTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReMathTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReMathTextureEditor(parent);
}
