/**
 * Qt Designer plugin.
 */

#include "ReBricksTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReBricksTextureEditor.h"


ReBricksTextureEditorPlugin::ReBricksTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReBricksTextureEditorPlugin::name() const        { return "ReBricksTextureEditor"; }
QString ReBricksTextureEditorPlugin::includeFile() const { return "ReBricksTextureEditor.h"; }
QString ReBricksTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReBricksTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReBricksTextureEditorPlugin::toolTip() const     { return "Editor for the Bricks Texture"; }
QString ReBricksTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReBricksTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReBricksTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReBricksTextureEditor(parent);
}
