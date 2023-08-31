/**
 * Qt Designer plugin.
 */

#include "ReDistortedNoiseTextureEditorPlugin.h"

#include <QtPlugin>

#include "../ReDistortedNoiseTextureEditor.h"


ReDistortedNoiseTextureEditorPlugin::ReDistortedNoiseTextureEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReDistortedNoiseTextureEditorPlugin::name() const        { return "ReDistortedNoiseTextureEditor"; }
QString ReDistortedNoiseTextureEditorPlugin::includeFile() const { return "ReDistortedNoiseTextureEditor.h"; }
QString ReDistortedNoiseTextureEditorPlugin::group() const       { return "Reality"; }
QIcon   ReDistortedNoiseTextureEditorPlugin::icon() const        { return QIcon(); }
QString ReDistortedNoiseTextureEditorPlugin::toolTip() const     { return "Editor for the DistortedNoise Texture"; }
QString ReDistortedNoiseTextureEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReDistortedNoiseTextureEditorPlugin::isContainer() const { return false; }

QWidget* ReDistortedNoiseTextureEditorPlugin::createWidget(QWidget* parent) {
  return new ReDistortedNoiseTextureEditor(parent);
}
