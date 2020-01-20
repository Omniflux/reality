/**
 * Plugin for using the Grayscale texture editor with Qt Designer.
 */

#ifndef __RE_GRAYSCALE_TEXTURE_EDITOR_PLUGIN_H__
#define __RE_GRAYSCALE_TEXTURE_EDITOR_PLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>


class ReGrayscaleTextureEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReGrayscaleTextureEditorPlugin(QObject* parent = 0);

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
