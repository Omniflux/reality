/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_OUTPUT_OPTIONS_ACTIONS_H
#define RE_OUTPUT_OPTIONS_ACTIONS_H

#include <QFileInfo>
#include <QUndoCommand>

#include "ReOutputUpdater.h"
#include "ReSceneDataGlobal.h"


/*
 * Render tab undoable Actions
 */

using namespace Reality;

/*
class SceneFileAction : public QUndoCommand {

private:
  QString oldValue, newValue;
  OutputUIUpdater* updater;

public:

  SceneFileAction( QString value, OutputUIUpdater* updater ) : 
    newValue(value), 
    updater(updater) {
    setText(QObject::tr("Set scene file name"));
  }

  void redo() {
    oldValue = RealitySceneData->getSceneFileName();
    RealitySceneData->setSceneFileName(newValue);
  }

  void undo() {
    newValue = RealitySceneData->getSceneFileName();
    RealitySceneData->setSceneFileName(oldValue);
    updater->showSceneFileName(oldValue);
  }
};
*/

class ImageFileAction : public QUndoCommand {

private:
  QString oldImageFileName, 
          newImageFileName;
  QString oldSceneFileName, 
          newSceneFileName;
  OutputUIUpdater* updater;
public:

  ImageFileAction( QString newFileName, OutputUIUpdater* updater ) : 
    newImageFileName(newFileName), 
    updater(updater) {
    setText(QObject::tr("Set image file name"));
  }

  QString syncSceneFileName( const QString& imageFileName ) {
    QFileInfo fi(imageFileName);
    QString sfn = QString("%1/%2.lxs").arg(fi.absolutePath()).arg(fi.baseName());
    RealitySceneData->setSceneFileName(sfn);
    return sfn;
  }

  void redo() {
    oldImageFileName = RealitySceneData->getImageFileName();
    oldSceneFileName = RealitySceneData->getSceneFileName();
    RealitySceneData->setImageFileName(newImageFileName);
    newSceneFileName = syncSceneFileName(newImageFileName);
    // updater->showSceneFileName(newSceneFileName);
  }

  void undo() {
    newImageFileName = RealitySceneData->getImageFileName();
    newSceneFileName = RealitySceneData->getSceneFileName();
    RealitySceneData->setImageFileName(oldImageFileName);
    RealitySceneData->setSceneFileName(oldSceneFileName);
    updater->showImageFileName(oldImageFileName);
  }
};

class GammaAction : public QUndoCommand {

private:
  float oldValue, newValue;
  OutputUIUpdater* updater;
public:

  GammaAction( float value, OutputUIUpdater* updater ) : 
    newValue(value), 
    updater(updater) {
    setText(QObject::tr("Set Gamma"));
  }

  void redo() {
    oldValue = RealitySceneData->getGamma();
    RealitySceneData->setGamma(newValue);
  }

  void undo() {
    newValue = RealitySceneData->getGamma();
    RealitySceneData->setGamma(oldValue);
    updater->showGamma(oldValue);
  }
};

class AlphaChannelAction : public QUndoCommand {

private:
  bool oldValue, newValue;
  OutputUIUpdater* updater;

public:

  AlphaChannelAction( bool value, OutputUIUpdater* updater ) : 
    newValue(value), 
    updater(updater) {
    setText("Set alpha channel");
  }

  void redo() {
    oldValue = RealitySceneData->hasAlphaChannel();
    RealitySceneData->setAlphaChannel(newValue);
  }

  void undo() {
    newValue = RealitySceneData->hasAlphaChannel();
    RealitySceneData->setAlphaChannel(oldValue);
    updater->showAlphaChannel(oldValue);
  }
};
/*
class GPUSupportAction : public QUndoCommand {

private:
  bool oldValue, newValue;
  OutputUIUpdater* updater;

public:

  GPUSupportAction( bool value, OutputUIUpdater* updater ) : 
    newValue(value), 
    updater(updater) {
    setText("Set GPU support");
  }

  void redo() {
    oldValue = RealitySceneData->isGPUEnabled();
    RealitySceneData->setGPUFlag(newValue);
  }

  void undo() {
    newValue = RealitySceneData->isGPUEnabled();
    RealitySceneData->setGPUFlag(oldValue);
    updater->showGPUSupport(oldValue);
  }
};
*/

class MaxSPXAction : public QUndoCommand {

private:
  quint32 oldValue, newValue;
  OutputUIUpdater* updater;

public:

  MaxSPXAction( quint32 value , OutputUIUpdater* updater ) : 
    newValue(value), 
    updater(updater) {

    setText("Set max samples");

  }

  void redo() {
    oldValue = RealitySceneData->getMaxSPX();
    RealitySceneData->setMaxSPX(newValue);
  }

  void undo() {
    newValue = RealitySceneData->getMaxSPX();
    RealitySceneData->setMaxSPX(oldValue);
    updater->showMaxSPX(oldValue);
  }
};

class OutlierRejectionAction : public QUndoCommand {

private:
  quint16 oldValue, newValue;
  OutputUIUpdater* updater;

public:

  OutlierRejectionAction( quint16 value, OutputUIUpdater* updater ) : 
    newValue(value),
    updater(updater) 
  {
    setText("Set FFs removal");
  }

  void redo() {
    oldValue = RealitySceneData->getOutlierRejection();
    RealitySceneData->setOutlierRejection(newValue);
  }

  void undo() {
    newValue = RealitySceneData->getOutlierRejection();
    RealitySceneData->setOutlierRejection(oldValue);
    updater->showOutlierRejection(oldValue);
  }
};

class ImageFileFormatAction : public QUndoCommand {

private:
  ExportImageFileFormat oldValue, newValue;
  OutputUIUpdater* updater;

public:

  ImageFileFormatAction( ExportImageFileFormat value, OutputUIUpdater* updater ) : 
    newValue(value),
    updater(updater)
  {
    setText(QObject::tr("Set Image file format"));
  }

  void redo() {
    oldValue = RealitySceneData->getImageFileFormat();
    RealitySceneData->setImageFileFormat(newValue);
    updater->setImageExtensionForFormat(newValue);
  }

  void undo() {
    newValue = RealitySceneData->getImageFileFormat();
    RealitySceneData->setImageFileFormat(oldValue);
    updater->showImageFileFormat(oldValue);
    updater->setImageExtensionForFormat(oldValue);
  }
};

class PreserveLightHueAction : public QUndoCommand {

private:
  bool oldValue, newValue;
  OutputUIUpdater* updater;
public:

  PreserveLightHueAction( bool value, OutputUIUpdater* updater ) : 
    newValue(value), 
    updater(updater)  {

    setText(QObject::tr("Set preserve light hue"));

  }

  void redo() {
    oldValue = RealitySceneData->getPreserveLightHue();
    RealitySceneData->setPreserveLightHue(newValue);
  }

  void undo() {
    newValue = RealitySceneData->getPreserveLightHue();
    RealitySceneData->setPreserveLightHue(oldValue);
    updater->showPreserveLightHue(oldValue);
  }
};

class FrameMultiplierAction : public QUndoCommand {

private:
  float oldValue, newValue;
  OutputUIUpdater* updater;

public:

  FrameMultiplierAction( float value, OutputUIUpdater* updater ) : 
    newValue(value),
    updater(updater) {
    setText(QObject::tr("Set frame size"));
  }

  void redo() {
    oldValue = RealitySceneData->getFrameMultiplier();
    RealitySceneData->setFrameMultiplier(newValue);
  }

  void undo() {
    newValue = RealitySceneData->getFrameMultiplier();
    RealitySceneData->setFrameMultiplier(oldValue);
    updater->showFrameMultiplier(oldValue);
  }
};

class GeometryFormatAction : public QUndoCommand {

private:
  GeometryFileFormat oldValue, newValue;
  OutputUIUpdater* updater;
public:

  GeometryFormatAction( GeometryFileFormat value, OutputUIUpdater* updater ) : 
    newValue(value),
    updater(updater) {
    setText("Set geometry format");
  }

  void redo() {
    oldValue = RealitySceneData->getGeometryFormat();
    RealitySceneData->setGeometryFormat(newValue);
  }

  void undo() {
    newValue = RealitySceneData->getGeometryFormat();
    RealitySceneData->setGeometryFormat(oldValue);
    updater->showGeometryFormat(oldValue);
  }
};

#endif
