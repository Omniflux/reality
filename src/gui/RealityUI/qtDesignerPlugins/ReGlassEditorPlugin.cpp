/**
 * Qt Designer plugin.
 */

#include "ReGlassEditorPlugin.h"

#include <QtPlugin>

#include "../MaterialEditors/ReGlassEditor.h"


ReGlassEditorPlugin::ReGlassEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReGlassEditorPlugin::name() const        { return "ReGlassEditor"; }
QString ReGlassEditorPlugin::includeFile() const { return "ReGlassEditor.h"; }
QString ReGlassEditorPlugin::group() const       { return "Reality"; }
QIcon   ReGlassEditorPlugin::icon() const        { return QIcon(); }
QString ReGlassEditorPlugin::toolTip() const     { return "The Reality Glass Editor"; }
QString ReGlassEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReGlassEditorPlugin::isContainer() const { return false; }

QWidget* ReGlassEditorPlugin::createWidget(QWidget* parent) {
  return new ReGlassEditor(parent);
}
