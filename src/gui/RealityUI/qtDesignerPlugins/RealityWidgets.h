/**
 * Collection of Reality Qt Designer plugins
 */ 

#ifndef REALITY_WIDGETS_H
#define REALITY_WIDGETS_H

#include <QtDesigner/QtDesigner>
#include <QtPlugin>

#include "Re2DTextureMappingPlugin.h"
#include "Re3DMappingEditorPlugin.h"
#include "ReAlphaChannelEditorPlugin.h"
#include "ReBandTextureEditorPlugin.h"
#include "ReBreadCrumbsPlugin.h"
#include "ReBricksTextureEditorPlugin.h"
#include "ReCameraEditorPlugin.h"
#include "ReCheckersTextureEditorPlugin.h"
#include "ReClothEditorPlugin.h"
#include "ReCloudsTextureEditorPlugin.h"
#include "ReColorMathTextureEditorPlugin.h"
#include "ReColorPickerPlugin.h"
#include "ReColorTextureEditorPlugin.h"
#include "ReDistortedNoiseTextureEditorPlugin.h"
#include "ReFBMTextureEditorPlugin.h"
#include "ReGlassEditorPlugin.h"
#include "ReGlossyEditorPlugin.h"
#include "ReGrayscaleTextureEditorPlugin.h"
#include "ReImageMapManagerPlugin.h"
#include "ReImageMapTextureEditorPlugin.h"
#include "ReMarbleTextureEditorPlugin.h"
#include "ReMathTextureEditorPlugin.h"
#include "ReMatteEditorPlugin.h"
#include "ReMatVolumesEditorPlugin.h"
#include "ReMetalEditorPlugin.h"
#include "ReMirrorEditorPlugin.h"
#include "ReMixTextureEditorPlugin.h"
#include "ReModifiersEditorPlugin.h"
#include "ReOutputOptionsPlugin.h"
#include "ReSkinEditorPlugin.h"
#include "ReSliderPlugin.h"
#include "ReTextureAvatarPlugin.h"
#include "ReTextureEditorPlugin.h"
#include "ReVelvetEditorPlugin.h"
#include "ReVolumesEditorPlugin.h"
#include "ReWaterEditorPlugin.h"
#include "ReWoodTextureEditorPlugin.h"


class RealityWidgets: public QObject, 
                      public QDesignerCustomWidgetCollectionInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    RealityWidgets(QObject *parent = 0);
    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> widgets;
};

#endif
