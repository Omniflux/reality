/**
 * Qt Designer plugin.
 */

#include "ReSliderPlugin.h"

#include <QtPlugin>

#include "../ReSlider.h"


ReSliderPlugin::ReSliderPlugin(QObject* parent) : QObject(parent) {}

QString ReSliderPlugin::name() const        { return "ReSlider"; }
QString ReSliderPlugin::includeFile() const { return "ReSlider.h"; }
QString ReSliderPlugin::group() const       { return "Reality"; }
QIcon   ReSliderPlugin::icon() const        { return(QIcon()); }
QString ReSliderPlugin::toolTip() const     { return "The Reality Slider"; }
QString ReSliderPlugin::whatsThis() const   { return toolTip(); }
bool    ReSliderPlugin::isContainer() const { return false; }
QWidget* ReSliderPlugin::createWidget(QWidget* parent) { 
  return new ReSlider(parent); 
}
