/**
 * Plugin for using the ReColorPicker with Qt Designer.
 */

#ifndef __RECOLORPICKERPLUGIN_H__
#define __RECOLORPICKERPLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>


class ReColorPickerPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReColorPickerPlugin(QObject* parent = 0);

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