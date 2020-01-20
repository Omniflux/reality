/*
 * Class: ReMathTextureEditorPlugin
 * Plugin for using the ReColorTextureEditor with Qt Designer.
 */

#ifndef __RE_RMATH_TEXTURE_EDITOR_PLUGIN_H__
#define __RE_RMATH_TEXTURE_EDITOR_PLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>

class ReMathTextureEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReMathTextureEditorPlugin(QObject* parent = 0);

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
