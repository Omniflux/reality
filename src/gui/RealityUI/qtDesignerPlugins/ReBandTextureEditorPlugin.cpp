/**
 * Qt Designer plugin.
 */

#include "ReBandTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReBandTextureEditor.h"


ReBandTextureEditorPlugin::ReBandTextureEditorPlugin(QObject* parent) : QObject(parent) {}

QString ReBandTextureEditorPlugin::name() const        { return "ReBandTextureEditor"; }
QString ReBandTextureEditorPlugin::includeFile() const { return "ReBandTextureEditor.h"; }
QString ReBandTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReBandTextureEditorPlugin::icon() const        { return(QIcon()); }
QString ReBandTextureEditorPlugin::toolTip() const     { return "The Reality 3 color picker"; }
QString ReBandTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReBandTextureEditorPlugin::isContainer() const { return false; }
QWidget* ReBandTextureEditorPlugin::createWidget(QWidget* parent)   { 
  return new ReBandTextureEditor(parent); 
}
