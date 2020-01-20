/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "ReClothEditorPlugin.h"
#include "../MaterialEditors/ReClothEditor.h"


ReClothEditorPlugin::ReClothEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReClothEditorPlugin::name() const        { return "ReClothEditor"; }
QString ReClothEditorPlugin::includeFile() const { return "ReClothEditor.h"; }
QString ReClothEditorPlugin::group() const       { return "Reality"; }
QIcon   ReClothEditorPlugin::icon() const        { return QIcon(); }
QString ReClothEditorPlugin::toolTip() const     { return "The Reality Cloth Editor"; }
QString ReClothEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReClothEditorPlugin::isContainer() const { return false; }

QWidget* ReClothEditorPlugin::createWidget(QWidget* parent) {
  return new ReClothEditor(parent);
}
