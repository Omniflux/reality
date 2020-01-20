/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "../ReWoodTextureEditor.h"
#include "ReWoodTextureEditorPlugin.h"


ReWoodTextureEditorPlugin::ReWoodTextureEditorPlugin(QObject* parent) : QObject(parent) {}

QString ReWoodTextureEditorPlugin::name() const        { return "ReWoodTextureEditor"; }
QString ReWoodTextureEditorPlugin::includeFile() const { return "ReWoodTextureEditor.h"; }
QString ReWoodTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReWoodTextureEditorPlugin::icon() const        { return(QIcon()); }
QString ReWoodTextureEditorPlugin::toolTip() const     { return "The Reality 3 color picker"; }
QString ReWoodTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReWoodTextureEditorPlugin::isContainer() const { return false; }
QWidget* ReWoodTextureEditorPlugin::createWidget(QWidget* parent)   { 
  return new ReWoodTextureEditor(parent); 
}
