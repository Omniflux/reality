/**
 * Qt Designer plugin.
 */

#include "ReSkinEditorPlugin.h"

#include <QtPlugin>

#include "../MaterialEditors/ReSkinEditor.h"


ReSkinEditorPlugin::ReSkinEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReSkinEditorPlugin::name() const        { return "ReSkinEditor"; }
QString ReSkinEditorPlugin::includeFile() const { return "ReSkinEditor.h"; }
QString ReSkinEditorPlugin::group() const       { return "Reality"; }
QIcon   ReSkinEditorPlugin::icon() const        { return QIcon(); }
QString ReSkinEditorPlugin::toolTip() const     { return "The Reality Skin Editor"; }
QString ReSkinEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReSkinEditorPlugin::isContainer() const { return false; }

QWidget* ReSkinEditorPlugin::createWidget(QWidget* parent) {
  return new ReSkinEditor(parent);
}
