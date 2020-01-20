/**
 * Plugin for using the Re2DTextureMapping with Qt Designer.
 */

#ifndef RE2DTEXTUREMAPPINGPLUGIN_H
#define RE2DTEXTUREMAPPINGPLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>

#include "Re2DTextureMapping.h"

class Re2DTextureMappingPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  Re2DTextureMappingPlugin(QObject* parent = 0);

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