/*
 * Class: ReTextureEditorPlugin
 * Plugin for using the ReTextureEditor with Qt Designer.
 */

#ifndef __RE_TEXTUREEDITOR_PLUGIN_H__
#define __RE_TEXTUREEDITOR_PLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>


class ReTextureEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReTextureEditorPlugin(QObject* parent = 0);

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
