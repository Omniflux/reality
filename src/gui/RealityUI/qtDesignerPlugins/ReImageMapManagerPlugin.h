/**
 * Plugin for using the ReImageMapManager with Qt Designer.
 */

#ifndef __RE_IMAGE_MAP_MANAGER_PLUGIN_H__
#define __RE_IMAGE_MAP_MANAGER_PLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>

#include "ReImageMapManager.h"

class ReImageMapManagerPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReImageMapManagerPlugin(QObject* parent = 0);

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