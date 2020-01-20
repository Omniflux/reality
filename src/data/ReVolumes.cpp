/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "ReVolumes.h"
#include "ReDefs.h"
#include "ReTools.h"
#include "ReMaterial.h"

using namespace Reality;


void ReVolume::serialize( QDataStream& dataStream ) const {
  dataStream << (quint16) volumeType  << name 
             << IOR << clarityAtDepth << color 
             << absorptionScale 
             << (quint16) editable    << scatteringColor 
             << scatteringScale       << directionR            
             << directionG            << directionB
             << cauchyB;
}


void ReVolume::deserialize( QDataStream& dataStream ) {
  quint16 volumeTypeInt, editableInt;
  dataStream >> volumeTypeInt
             >> name            >> IOR 
             >> clarityAtDepth  >> color 
             >> absorptionScale
             >> editableInt     >> scatteringColor 
             >> scatteringScale >> directionR     
             >> directionG      >> directionB
             >> cauchyB;

  editable = static_cast<bool>(editableInt);
  volumeType = static_cast<ReVolume::VolumeType>(volumeTypeInt);
}
