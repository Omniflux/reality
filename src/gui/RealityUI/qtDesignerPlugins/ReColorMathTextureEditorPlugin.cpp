/**
 * Qt Designer plugin.
 */

#include "ReColorMathTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReColorMathTextureEditor.h"


ReColorMathTextureEditorPlugin::ReColorMathTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReColorMathTextureEditorPlugin::name() const        { return "ReColorMathTextureEditor"; }
QString ReColorMathTextureEditorPlugin::includeFile() const { return "ReColorMathTextureEditor.h"; }
QString ReColorMathTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReColorMathTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReColorMathTextureEditorPlugin::toolTip() const     { return "Editor for the ColorMath Texture"; }
QString ReColorMathTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReColorMathTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReColorMathTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReColorMathTextureEditor(parent);
}
