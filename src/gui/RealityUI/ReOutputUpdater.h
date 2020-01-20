/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_OUTPUT_UPDATER_H
#define RE_OUTPUT_UPDATER_H

#include <QtCore>
#include "ReDefs.h"  

namespace Reality {

//! Interface class that defines the protocol 
//! to communicate the modification of values to the
//! UI. This is used by the Commands for Undoing a
//! given operation and to reflect that operation in the 
//! Output tab UI.
class OutputUIUpdater {

public:

  virtual void showGamma( const double newGamma ) = 0;
  virtual void showAlphaChannel( const bool newAlpha ) = 0;
  virtual void showMaxSPX( const quint32 newVal ) = 0;
  virtual void showOutlierRejection( const quint16 newVal ) = 0;
  virtual void showImageFileFormat( const ExportImageFileFormat newVal ) = 0;
  virtual void showPreserveLightHue( const bool newVal ) = 0;
  virtual void showFrameMultiplier( const float newVal ) = 0;
  virtual void showGeometryFormat( const GeometryFileFormat newVal ) = 0;
  // virtual void showSceneFileName( const QString newVal ) = 0;
  virtual void showImageFileName( const QString newVal ) = 0;
  // virtual void showGPUSupport( const bool newVal ) = 0;

  //! Given a file format this function sets the appropriate extension for
  //! the image file to be generated for the render.
  virtual void setImageExtensionForFormat( ExportImageFileFormat newFormat ) = 0;
};

} // namespace


#endif
