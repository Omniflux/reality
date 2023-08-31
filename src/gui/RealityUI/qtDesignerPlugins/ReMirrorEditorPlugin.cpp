/**
 * Qt Designer plugin.
 */

#include "ReMirrorEditorPlugin.h"

#include <QtPlugin>

#include "../MaterialEditors/ReMirrorEditor.h"


ReMirrorEditorPlugin::ReMirrorEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReMirrorEditorPlugin::name() const        { return "ReMirrorEditor"; }
QString ReMirrorEditorPlugin::includeFile() const { return "ReMirrorEditor.h"; }
QString ReMirrorEditorPlugin::group() const       { return "Reality"; }
QIcon   ReMirrorEditorPlugin::icon() const        { return QIcon(); }
QString ReMirrorEditorPlugin::toolTip() const     { return "The Reality Mirror Editor"; }
QString ReMirrorEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReMirrorEditorPlugin::isContainer() const { return false; }

QWidget* ReMirrorEditorPlugin::createWidget(QWidget* parent) {
  return new ReMirrorEditor(parent);
}
