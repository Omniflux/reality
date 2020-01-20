/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "../ReMarbleTextureEditor.h"
#include "ReMarbleTextureEditorPlugin.h"


ReMarbleTextureEditorPlugin::ReMarbleTextureEditorPlugin(QObject* parent) : QObject(parent) {}

QString ReMarbleTextureEditorPlugin::name() const        { return "ReMarbleTextureEditor"; }
QString ReMarbleTextureEditorPlugin::includeFile() const { return "ReMarbleTextureEditor.h"; }
QString ReMarbleTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReMarbleTextureEditorPlugin::icon() const        { return(QIcon()); }
QString ReMarbleTextureEditorPlugin::toolTip() const     { return "The Reality 3 color picker"; }
QString ReMarbleTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReMarbleTextureEditorPlugin::isContainer() const { return false; }
QWidget* ReMarbleTextureEditorPlugin::createWidget(QWidget* parent)   { 
  return new ReMarbleTextureEditor(parent); 
}
