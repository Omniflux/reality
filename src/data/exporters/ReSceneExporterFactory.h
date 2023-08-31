/**
  \file
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#ifndef RE_SCENE_EXPORTER_FACTORY_H
#define RE_SCENE_EXPORTER_FACTORY_H

#include <boost/any.hpp>
#include <QHash>

#include "ReBaseSceneExporter.h"
#include "ReLogger.h"


namespace Reality {

class ReSceneData;

/**
 * This class is used to return a suitable value in the case the user
 * asks for an exporter that doesn't exist. In this way we avoid
 * the risk of returning a null pointer.
 */
class ReDummySceneExporter : public ReBaseSceneExporter {

public:
  //! Constructor: ReDummySceneExporter
  ReDummySceneExporter( ReSceneData* scene ) : ReBaseSceneExporter(scene) 
  {
  };

  //! Destructor: ReDummySceneExporter
 ~ReDummySceneExporter() {
  };

  void exportScene( const int /* frameNo */, boost::any& sceneData ) {
    sceneData = QVariant(QString("No exporter available for the scene type"));
  }

  void prepare() {
    // Nothing
  }
  void cleanup() {
    // Nothing
  }
};

/**
  Class ReSceneExporterFactory
 */
class ReSceneExporterFactory {

private:
  typedef QHash<QString, ReSceneExporterCreator> ExporterCreatorTable;
  typedef QHash<QString, ReBaseSceneExporterPtr > ExporterTableCache;
  //! A table to hold references to functions that create exporters
  ExporterCreatorTable exporterCreators;
  ExporterTableCache exporterCache;

  //! A smart pointer to the dummy exporter, just in case.
  QSharedPointer<ReDummySceneExporter> dummyExporter;

public:
  //! Constructor: ReSceneExporterFactory
  ReSceneExporterFactory() {
  };

  //! Destructor: ReSceneExporterFactory
 ~ReSceneExporterFactory() {
  };

  void registerExporter( const QString& exporterID, ReSceneExporterCreator creator ) {
    if (creator) {
      RE_LOG_INFO() << "Registered exporter for format " << exporterID.toStdString();
      exporterCreators[exporterID] = creator;
    }
  };


  ReBaseSceneExporter* getExporter( const QString& id ) {
    if (exporterCache.contains(id)) {
      return exporterCache.value(id).data();
    }
    ReBaseSceneExporterPtr ptr;
    if (exporterCreators.contains(id)) {
      ptr = ReBaseSceneExporterPtr(exporterCreators.value(id)());
    }
    else {
      ptr = ReBaseSceneExporterPtr(new ReDummySceneExporter(NULL));
    }
    exporterCache[id] = ptr;
    return ptr.data();
  }
};


} // namespace

#endif
