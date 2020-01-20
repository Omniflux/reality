/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef QT_CHECKERS_IMPORTER_H
#define QT_CHECKERS_IMPORTER_H

#include "textures/ReCheckers.h"
#include "ReQtTextureImporter.h"

namespace Reality {

/*
  Class: ReQtCheckersExporter
 */

class RE_LIB_ACCESS ReQtCheckersImporter : public ReQtTextureImporter {

public:

  void restoreTexture( ReTexturePtr baseTex, const QVariantMap& data ) {
    ReCheckersPtr tex = baseTex.staticCast<ReCheckers>();
    restoreBasicProperties(tex, data);

    tex->set3DFlag(data.value("is3D").toBool());
    restoreChannels(tex,data);
    restore3DMapping(tex,data);    
  };
};

}
#endif
