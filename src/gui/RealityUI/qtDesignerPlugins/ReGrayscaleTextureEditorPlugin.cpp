/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReGrayscaleTextureEditorPlugin.h"
#include "../ReGrayscaleTextureEditor.h"


ReGrayscaleTextureEditorPlugin::ReGrayscaleTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReGrayscaleTextureEditorPlugin::name() const        { return "ReGrayscaleTextureEditor"; }
QString ReGrayscaleTextureEditorPlugin::includeFile() const { return "ReGrayscaleTextureEditor.h"; }
QString ReGrayscaleTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReGrayscaleTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReGrayscaleTextureEditorPlugin::toolTip() const     { return "Editor for the Grayscale Texture"; }
QString ReGrayscaleTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReGrayscaleTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReGrayscaleTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReGrayscaleTextureEditor(parent);
}
