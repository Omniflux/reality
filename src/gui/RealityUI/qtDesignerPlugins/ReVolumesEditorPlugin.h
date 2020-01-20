/**
 * Plugin for using the VolumesEditor with Qt Designer.
 */

#ifndef __VOLUMESEDITORPLUGIN_H__
#define __VOLUMESEDITORPLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>


class ReVolumesEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReVolumesEditorPlugin(QObject* parent = 0);

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
