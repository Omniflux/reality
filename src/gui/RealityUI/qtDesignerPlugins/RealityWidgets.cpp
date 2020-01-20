/**
 * The Reality Widgets for Qt Designer.
 */

#include "RealityWidgets.h"

 
RealityWidgets::RealityWidgets(QObject *parent) : QObject(parent) {
  widgets.append(new Re2DTextureMappingPlugin(this));
  widgets.append(new Re3DMappingEditorPlugin(this));
  widgets.append(new ReAlphaChannelEditorPlugin(this));
  widgets.append(new ReBandTextureEditorPlugin(this));
  widgets.append(new ReBreadCrumbsPlugin(this));
  widgets.append(new ReBricksTextureEditorPlugin(this));
  widgets.append(new ReCameraEditorPlugin(this));
  widgets.append(new ReCheckersTextureEditorPlugin(this));
  widgets.append(new ReClothEditorPlugin(this));
  widgets.append(new ReCloudsTextureEditorPlugin(this));
  widgets.append(new ReColorMathTextureEditorPlugin(this));
  widgets.append(new ReColorPickerPlugin(this));
  widgets.append(new ReColorTextureEditorPlugin(this));
  widgets.append(new ReDistortedNoiseTextureEditorPlugin(this));
  widgets.append(new ReFBMTextureEditorPlugin(this));
  widgets.append(new ReGlassEditorPlugin(this));
  widgets.append(new ReGlossyEditorPlugin(this));
  widgets.append(new ReGrayscaleTextureEditorPlugin(this));
  widgets.append(new ReImageMapManagerPlugin(this));
  widgets.append(new ReImageMapTextureEditorPlugin(this));
  widgets.append(new ReMarbleTextureEditorPlugin(this));
  widgets.append(new ReMathTextureEditorPlugin(this));
  widgets.append(new ReMatteEditorPlugin(this));
  widgets.append(new ReMatVolumesEditorPlugin(this));
  widgets.append(new ReMetalEditorPlugin(this));
  widgets.append(new ReMirrorEditorPlugin(this));
  widgets.append(new ReMixTextureEditorPlugin(this));
  widgets.append(new ReModifiersEditorPlugin(this));
  widgets.append(new ReOutputOptionsPlugin(this));
  widgets.append(new ReSkinEditorPlugin(this));
  widgets.append(new ReSliderPlugin(this));
  widgets.append(new ReTextureAvatarPlugin(this));
  widgets.append(new ReTextureEditorPlugin(this));
  widgets.append(new ReVelvetEditorPlugin(this));
  widgets.append(new ReVolumesEditorPlugin(this));
  widgets.append(new ReWaterEditorPlugin(this));
  widgets.append(new ReWoodTextureEditorPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> RealityWidgets::customWidgets() const {
  return widgets;
}

Q_EXPORT_PLUGIN2(reality_widgets, RealityWidgets)
