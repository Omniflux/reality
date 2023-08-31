/**
 * Qt Designer plugin.
 */

#include "ReFBMTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReFBMTextureEditor.h"


ReFBMTextureEditorPlugin::ReFBMTextureEditorPlugin(QObject* parent) : QObject(parent) {}

QString ReFBMTextureEditorPlugin::name() const        { return "ReFBMTextureEditor"; }
QString ReFBMTextureEditorPlugin::includeFile() const { return "ReFBMTextureEditor.h"; }
QString ReFBMTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReFBMTextureEditorPlugin::icon() const        { return(QIcon()); }
QString ReFBMTextureEditorPlugin::toolTip() const     { return "The Reality 3 color picker"; }
QString ReFBMTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReFBMTextureEditorPlugin::isContainer() const { return false; }
QWidget* ReFBMTextureEditorPlugin::createWidget(QWidget* parent)   { 
  return new ReFBMTextureEditor(parent); 
}
