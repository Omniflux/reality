/**
 * Collection of Reality Qt Designer plugins
 */ 

#ifndef REALITY_WIDGETS_H
#define REALITY_WIDGETS_H

#include "Re2DTextureMappingPlugin.h"
#include "ReAlphaChannelEditorPlugin.h"
#include "ReBreadCrumbsPlugin.h"
#include "ReCloudsTextureEditorPlugin.h"
#include "ReDistortedNoiseTextureEditorPlugin.h"
#include "ReBricksTextureEditorPlugin.h"
#include "ReCheckersTextureEditorPlugin.h"
#include "ReFBMTextureEditorPlugin.h"
#include "ReBandTextureEditorPlugin.h"
#include "Re3DMappingEditorPlugin.h"
#include "ReMarbleTextureEditorPlugin.h"
#include "ReWoodTextureEditorPlugin.h"
#include "ReMathTextureEditorPlugin.h"
#include "ReColorMathTextureEditorPlugin.h"
#include "ReColorPickerPlugin.h"
#include "ReColorTextureEditorPlugin.h"
#include "ReGlassEditorPlugin.h"
#include "ReWaterEditorPlugin.h"
#include "ReGlossyEditorPlugin.h"
#include "ReMatteEditorPlugin.h"
#include "ReMirrorEditorPlugin.h"
#include "ReVelvetEditorPlugin.h"
#include "ReClothEditorPlugin.h"
#include "ReMetalEditorPlugin.h"
#include "ReModifiersEditorPlugin.h"
#include "ReImageMapManagerPlugin.h"
#include "ReImageMapTextureEditorPlugin.h"
#include "ReMixTextureEditorPlugin.h"
#include "ReSkinEditorPlugin.h"
#include "ReTextureAvatarPlugin.h"
#include "ReTextureEditorPlugin.h"
#include "ReVolumesEditorPlugin.h"
#include "ReMatVolumesEditorPlugin.h"
#include "ReGrayscaleTextureEditorPlugin.h"
#include "ReOutputOptionsPlugin.h"
#include "ReCameraEditorPlugin.h"
#include "ReSliderPlugin.h"

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>

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
