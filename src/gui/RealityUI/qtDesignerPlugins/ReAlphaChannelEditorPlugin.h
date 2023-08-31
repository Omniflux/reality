/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_ALPHA_CHANNEL_EDITOR_PLUGIN_H
#define RE_ALPHA_CHANNEL_EDITOR_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>


/**
 * Plugin for using the ReAlphaChannelEditor widget with Qt Designer.
 */
class ReAlphaChannelEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReAlphaChannelEditorPlugin(QObject* parent = 0);

  QString name() const ;
  QString includeFile() const ;
  QString group() const;
  QIcon icon() const;
  QString toolTip() const;
  QString whatsThis() const;
  bool isContainer() const;

  QWidget* createWidget(QWidget* parent);

};

#endif
