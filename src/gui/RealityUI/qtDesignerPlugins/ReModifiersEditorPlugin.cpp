/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReModifiersEditorPlugin.h"
#include "../MaterialEditors/ReModifiers.h"


ReModifiersEditorPlugin::ReModifiersEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReModifiersEditorPlugin::name() const        { return "ReModifiers"; }
QString ReModifiersEditorPlugin::includeFile() const { return "ReModifiers.h"; }
QString ReModifiersEditorPlugin::group() const       { return "Reality"; }
QIcon   ReModifiersEditorPlugin::icon() const        { return QIcon(); }
QString ReModifiersEditorPlugin::toolTip() const     { return "The Reality Modifiers Editor"; }
QString ReModifiersEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReModifiersEditorPlugin::isContainer() const { return false; }

QWidget* ReModifiersEditorPlugin::createWidget(QWidget* parent) {
  return new ReModifiers(parent);
}
