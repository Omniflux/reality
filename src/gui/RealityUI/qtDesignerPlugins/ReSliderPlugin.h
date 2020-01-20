/**
 * Plugin for using the ReColorPicker with Qt Designer.
 */

#ifndef RESLIDER_PLUGIN_H
#define RESLIDER_PLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>


class ReSliderPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReSliderPlugin(QObject* parent = 0);

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