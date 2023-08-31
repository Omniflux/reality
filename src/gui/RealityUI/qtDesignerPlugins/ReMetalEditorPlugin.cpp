/**
 * Qt Designer plugin.
 */

#include "ReMetalEditorPlugin.h"

#include <QtPlugin>

#include "../MaterialEditors/ReMetalEditor.h"


ReMetalEditorPlugin::ReMetalEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReMetalEditorPlugin::name() const        { return "ReMetalEditor"; }
QString ReMetalEditorPlugin::includeFile() const { return "ReMetalEditor.h"; }
QString ReMetalEditorPlugin::group() const       { return "Reality"; }
QIcon   ReMetalEditorPlugin::icon() const        { return QIcon(); }
QString ReMetalEditorPlugin::toolTip() const     { return "The Reality Metal Editor"; }
QString ReMetalEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReMetalEditorPlugin::isContainer() const { return false; }

QWidget* ReMetalEditorPlugin::createWidget(QWidget* parent) {
  return new ReMetalEditor(parent);
}
