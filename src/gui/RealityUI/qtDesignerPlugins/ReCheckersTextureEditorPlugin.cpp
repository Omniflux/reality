/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "../ReCheckersTextureEditor.h"
#include "ReCheckersTextureEditorPlugin.h"


ReCheckersTextureEditorPlugin::ReCheckersTextureEditorPlugin(QObject* parent) : QObject(parent) {}

QString ReCheckersTextureEditorPlugin::name() const        { return "ReCheckersTextureEditor"; }
QString ReCheckersTextureEditorPlugin::includeFile() const { return "ReCheckersTextureEditor.h"; }
QString ReCheckersTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReCheckersTextureEditorPlugin::icon() const        { return(QIcon()); }
QString ReCheckersTextureEditorPlugin::toolTip() const     { return "The Reality 3 color picker"; }
QString ReCheckersTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReCheckersTextureEditorPlugin::isContainer() const { return false; }
QWidget* ReCheckersTextureEditorPlugin::createWidget(QWidget* parent)   { 
  return new ReCheckersTextureEditor(parent); 
}
