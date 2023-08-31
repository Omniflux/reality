/**
 * Qt Designer plugin.
 */

#include "ReTextureAvatarPlugin.h"

#include <QtPlugin>

#include "../ReTextureAvatar.h"


ReTextureAvatarPlugin::ReTextureAvatarPlugin(QObject* parent) : QObject(parent) {

}

QString ReTextureAvatarPlugin::name() const        { return "ReTextureAvatar"; }
QString ReTextureAvatarPlugin::includeFile() const { return "ReTextureAvatar.h"; }
QString ReTextureAvatarPlugin::group() const       { return "Reality"; }
QIcon   ReTextureAvatarPlugin::icon() const        { return QIcon(); }
QString ReTextureAvatarPlugin::toolTip() const     { return "The Reality Texture Avatar"; }
QString ReTextureAvatarPlugin::whatsThis() const   { return toolTip(); }
bool    ReTextureAvatarPlugin::isContainer() const { return false; }

QWidget* ReTextureAvatarPlugin::createWidget(QWidget* parent) {
  return new ReTextureAvatar(parent);
}
