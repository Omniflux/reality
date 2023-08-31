/**
 * Qt Designer plugin.
 */

#include "ReGlossyEditorPlugin.h"

#include <QtPlugin>

#include "../MaterialEditors/ReGlossyEditor.h"


ReGlossyEditorPlugin::ReGlossyEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReGlossyEditorPlugin::name() const        { return "ReGlossyEditor"; }
QString ReGlossyEditorPlugin::includeFile() const { return "ReGlossyEditor.h"; }
QString ReGlossyEditorPlugin::group() const       { return "Reality"; }
QIcon   ReGlossyEditorPlugin::icon() const        { return QIcon(); }
QString ReGlossyEditorPlugin::toolTip() const     { return "The Reality Glossy Editor"; }
QString ReGlossyEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReGlossyEditorPlugin::isContainer() const { return false; }

QWidget* ReGlossyEditorPlugin::createWidget(QWidget* parent) {
  return new ReGlossyEditor(parent);
}
