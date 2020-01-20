/**
 * Plugin for using the ReImageMapManager with Qt Designer.
 */

#ifndef RE_GLASS_EDITOR_PLUGIN_H
#define RE_GLASS_EDITOR_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>


class ReGlassEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReGlassEditorPlugin(QObject* parent = 0);

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
