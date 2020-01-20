/**
 * Plugin for using the ReMirrorEditor with Qt Designer.
 */

#ifndef RE_MIRROR_EDITOR_PLUGIN_H
#define RE_MIRROR_EDITOR_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>


class ReMirrorEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReMirrorEditorPlugin(QObject* parent = 0);

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
