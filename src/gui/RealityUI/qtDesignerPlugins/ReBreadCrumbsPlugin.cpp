/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include <QtPlugin>
#include "ReBreadCrumbsPlugin.h"
#include "../ReBreadCrumbs.h"

/**
 * Qt Designer plugin.
 */
ReBreadCrumbsPlugin::ReBreadCrumbsPlugin(QObject* parent) : QObject(parent) {

}

QString ReBreadCrumbsPlugin::name() const        { return "ReBreadCrumbs"; }
QString ReBreadCrumbsPlugin::includeFile() const { return "ReBreadCrumbs.h"; }
QString ReBreadCrumbsPlugin::group() const       { return "Reality"; }
QIcon   ReBreadCrumbsPlugin::icon() const        { return QIcon(); }
QString ReBreadCrumbsPlugin::toolTip() const     { return "The Reality Breadcrumb widget"; }
QString ReBreadCrumbsPlugin::whatsThis() const   { return toolTip(); }
bool    ReBreadCrumbsPlugin::isContainer() const { return false; }

QWidget* ReBreadCrumbsPlugin::createWidget(QWidget* parent) {
  return new ReBreadCrumbs(parent);
}
