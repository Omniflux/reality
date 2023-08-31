/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef ReQtSceneImporter_H
#define ReQtSceneImporter_H

#include <QVariantMap>


namespace Reality {

/**
 * Restores the settings for the scene by loading the data from the host-app scene file
 */
class ReQtSceneImporter {

public:
  // Constructor: ReQtSceneImporter
  ReQtSceneImporter() {
  };
  // Destructor: ReQtSceneImporter
 ~ReQtSceneImporter() {
  };

  void restoreSceneData( const QVariantMap& data,
                         const int mainVersion, 
                         const int subVersion );
};

} // namespace

#endif
