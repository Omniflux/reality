/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReAlphaChannelEditorPlugin.h"

#include <QtPlugin>

#include "../MaterialEditors/ReAlphaChannelEditor.h"

/**
 * Qt Designer plugin.
 */
ReAlphaChannelEditorPlugin::ReAlphaChannelEditorPlugin(QObject* parent) : QObject(parent) {

}

QString ReAlphaChannelEditorPlugin::name() const        { return "ReAlphaChannelEditor"; }
QString ReAlphaChannelEditorPlugin::includeFile() const { return "ReAlphaChannelEditor.h"; }
QString ReAlphaChannelEditorPlugin::group() const       { return "Reality"; }
QIcon   ReAlphaChannelEditorPlugin::icon() const        { return QIcon(); }
QString ReAlphaChannelEditorPlugin::toolTip() const     { return "The Reality Alpha Channel Editor widget"; }
QString ReAlphaChannelEditorPlugin::whatsThis() const   { return toolTip(); }
bool    ReAlphaChannelEditorPlugin::isContainer() const { return false; }

QWidget* ReAlphaChannelEditorPlugin::createWidget(QWidget* parent) {
  return new ReAlphaChannelEditor(parent);
}
