/**
 * Plugin for using the ReFBMTextureEditor with Qt Designer.
 */

#ifndef RE_FBMTEXTURE_EDITOR_PLUGIN_H
#define RE_FBMTEXTURE_EDITOR_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>


class ReFBMTextureEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReFBMTextureEditorPlugin(QObject* parent = 0);

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