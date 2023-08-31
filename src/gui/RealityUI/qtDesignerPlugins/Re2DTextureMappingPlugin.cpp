/**
 * Qt Designer plugin.
 */

#include "Re2DTextureMappingPlugin.h"

#include <QtPlugin>

#include "../Re2DTextureMapping.h"


Re2DTextureMappingPlugin::Re2DTextureMappingPlugin(QObject* parent) : QObject(parent) {

}

QString Re2DTextureMappingPlugin::name() const        { return "Re2DTextureMapping"; }
QString Re2DTextureMappingPlugin::includeFile() const { return "Re2DTextureMapping.h"; }
QString Re2DTextureMappingPlugin::group() const       { return "Reality"; }
QIcon   Re2DTextureMappingPlugin::icon() const        { return QIcon(); }
QString Re2DTextureMappingPlugin::toolTip() const     { return "The Reality texture mapping widget"; }
QString Re2DTextureMappingPlugin::whatsThis() const   { return toolTip(); }
bool    Re2DTextureMappingPlugin::isContainer() const { return false; }

QWidget* Re2DTextureMappingPlugin::createWidget(QWidget* parent) {
  return new Re2DTextureMapping(parent);
}
