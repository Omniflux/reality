/**
 * Qt Designer plugin.
 */

#include "ReVelvetEditorPlugin.h"

#include <QtPlugin>

#include "../MaterialEditors/ReVelvetEditor.h"


ReVelvetEditorPlugin::ReVelvetEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReVelvetEditorPlugin::name() const        { return "ReVelvetEditor"; }
QString ReVelvetEditorPlugin::includeFile() const { return "ReVelvetEditor.h"; }
QString ReVelvetEditorPlugin::group() const       { return "Reality"; }
QIcon   ReVelvetEditorPlugin::icon() const        { return QIcon(); }
QString ReVelvetEditorPlugin::toolTip() const     { return "The Reality Velvet Editor"; }
QString ReVelvetEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReVelvetEditorPlugin::isContainer() const { return false; }

QWidget* ReVelvetEditorPlugin::createWidget(QWidget* parent) {
  return new ReVelvetEditor(parent);
}
