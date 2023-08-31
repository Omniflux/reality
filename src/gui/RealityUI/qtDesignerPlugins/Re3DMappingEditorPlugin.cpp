/**
 * Qt Designer plugin.
 */

#include "Re3DMappingEditorPlugin.h"

#include <QtPlugin>

#include "../Re3DMappingEditor.h"


Re3DMappingEditorPlugin::Re3DMappingEditorPlugin(QObject* parent) : QObject(parent) {}

QString Re3DMappingEditorPlugin::name() const        { return "Re3DMappingEditor"; }
QString Re3DMappingEditorPlugin::includeFile() const { return "Re3DMappingEditor.h"; }
QString Re3DMappingEditorPlugin::group() const       { return "Reality"; }
QIcon   Re3DMappingEditorPlugin::icon() const        { return(QIcon()); }
QString Re3DMappingEditorPlugin::toolTip() const     { return "The Reality 3 color picker"; }
QString Re3DMappingEditorPlugin::whatsThis() const   { return toolTip(); }
bool    Re3DMappingEditorPlugin::isContainer() const { return false; }
QWidget* Re3DMappingEditorPlugin::createWidget(QWidget* parent)   { 
  return new Re3DMappingEditor(parent); 
}
