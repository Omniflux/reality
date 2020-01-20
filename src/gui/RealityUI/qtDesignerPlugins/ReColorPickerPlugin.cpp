/**
 * Qt Designer plugin.
 */

#include <QtPlugin>
#include "../ReColorPicker.h"
#include "ReColorPickerPlugin.h"


ReColorPickerPlugin::ReColorPickerPlugin(QObject* parent) : QObject(parent) {}

QString ReColorPickerPlugin::name() const        { return "ReColorPicker"; }
QString ReColorPickerPlugin::includeFile() const { return "ReColorPicker.h"; }
QString ReColorPickerPlugin::group() const       { return "Reality"; }
QIcon   ReColorPickerPlugin::icon() const        { return(QIcon()); }
QString ReColorPickerPlugin::toolTip() const     { return "The Reality 3 color picker"; }
QString ReColorPickerPlugin::whatsThis() const   { return toolTip(); }
bool    ReColorPickerPlugin::isContainer() const { return false; }
QWidget* ReColorPickerPlugin::createWidget(QWidget* parent)   { return new ReColorPicker(parent); }
