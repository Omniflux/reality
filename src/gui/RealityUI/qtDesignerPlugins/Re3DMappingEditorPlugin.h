/**
 * Plugin for using the Re3DMappingEditor with Qt Designer.
 */

#ifndef RE_3DMAPPING_EDITOR_PLUGIN_H
#define RE_3DMAPPING_EDITOR_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>


class Re3DMappingEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  Re3DMappingEditorPlugin(QObject* parent = 0);

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