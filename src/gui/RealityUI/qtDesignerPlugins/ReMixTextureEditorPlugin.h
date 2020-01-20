/*
 * Class: ReMixTextureEditorPlugin
 * Plugin for using the ReMixTextureEditor with Qt Designer.
 */

#ifndef __RE_MIX_TEXTURE_EDITOR_PLUGIN_H__
#define __RE_MIX_TEXTURE_EDITOR_PLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>

class ReMixTextureEditorPlugin : public QObject, public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  
  ReMixTextureEditorPlugin(QObject* parent = 0);

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
