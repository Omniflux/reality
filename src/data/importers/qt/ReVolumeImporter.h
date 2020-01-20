#ifndef RE_QT_VOLUME_IMPORTER_H
#define RE_QT_VOLUME_IMPORTER_H

#include "ReVolumes.h"
#include "ReLogger.h"
#include <QVariant>

/*
  Class: ReVolumeImporter

  Imports the volumes definition from a QVariantMap
 */
namespace Reality {

class ReQtVolumeImporter  {

public:
  
  void importVolume( ReVolumePtr vol, 
                     const QVariantMap& data, 
                     const bool overrideName = false ) {
    if (overrideName) {
      vol->setName( data.value("name").toString() );
    }
    vol->setType( static_cast<ReVolume::VolumeType>(data.value("volumeType").toInt()) );
    vol->setIOR( data.value("IOR").toFloat() );
    vol->setClarityAtDepth( data.value("clarityAtDepth").toFloat() );
    vol->setColor( data.value("color").toString() );
    vol->setEditable( data.value("editable").toBool() );
    vol->setScatteringColor( data.value("scatteringColor").toString() );
    vol->setAbsorptionScale( data.value("absorptionScale").toFloat() );
    vol->setScatteringScale( data.value("scatteringScale").toFloat() );
    vol->setDirectionR( data.value("directionR").toFloat() );
    vol->setDirectionG( data.value("directionG").toFloat() );
    vol->setDirectionB( data.value("directionB").toFloat() );
    vol->setCauchyB( data.value("cauchyB", 0.0).toFloat() );
  }

  ReVolumePtr importVolume( const QVariantMap& data ) {    
    ReVolumePtr vol = ReVolumePtr( new ReVolume() );
    importVolume(vol, data, true);
    // We don't load the linked materia list because at this point the materials
    // might not be already created. In addition, the loading of the materials from
    // their own configuration will restore the linkage with the volumes.
    // The important thing is that the volumes are ready in the scene database.
    return vol;
  }

};

}

#endif
