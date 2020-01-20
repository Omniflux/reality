/*
 * Class: ReImageMapTextureEditorPlugin
 * Plugin for using the ReImageMapTextureEditor with Qt Designer.
 */

#ifndef RE_IMAGEMAP_TEXTURE_EDITOR_PLUGIN_H
#define RE_IMAGEMAP_TEXTURE_EDITOR_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>

class ReImageMapTextureEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReImageMapTextureEditorPlugin(QObject* parent = 0);

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
