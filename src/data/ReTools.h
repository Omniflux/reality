/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RETOOLS_H
#define RETOOLS_H

#include "textures/ReConstant.h"
#include "ReLogger.h"
#include "ReMaterial.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACX)
#   include <unistd.h>
#elif defined(Q_OS_MACX)
#   include <mach/mach.h>
#   include <mach/machine.h>
#endif

//! Inserts debug statements in the Lux scene
#ifdef DEBUG_LUX
#define LUX_DEBUG(str) str += QString("#! DEBUG: %1 @ %2\n").arg(__FILE__).arg(__LINE__);
#else 
#define LUX_DEBUG(str)
#endif

//! Poser and Studio use the Ambient color to create a glow for a material
//! While Reality can convert a material to be an emitter, this is best
//! avoided unless the material is really glowing with strength. To
//! verify this situation, a brightness threshold is defined. Any color 
//! that has brighness within this threshold is not converted to be
//! an emitter.
#define RE_COLOR_EMISSION_THRESHOLD 25

//! Function prototype for the second parameters of transformImageMaps()
typedef void (*ReTextureTransfFunc)(Reality::ReTexturePtr t);

namespace Reality {
  
/**
 Converts a color from a list of three floats to QColor  

 Parameters:
   srcColor - A QVariantList that holds three values. [0] == Red, [1] == Green, [2] == Blue

 Returns:
   A corresponding QColor object.
 */

RE_LIB_ACCESS inline QColor convertFloatColor(const QVariantList srcColor ) {
  QColor clr;
  if (srcColor.count() == 3) {
    clr.setRgbF(srcColor[0].toDouble(), srcColor[1].toDouble(), srcColor[2].toDouble());
  }
  return clr;
}


RE_LIB_ACCESS inline bool isPureWhite( const QColor clr ) {
  int r,g,b;
  clr.getRgb(&r,&g,&b);
  return(r == 255 && g == 255 && b == 255);
}

RE_LIB_ACCESS inline bool isPureWhite( const ReTexturePtr tex ) {
  if (tex->getType() != TexConstant) {
    return false;
  }

  int r,g,b;
  tex->getNamedValue("color").value<QColor>().getRgb(&r,&g,&b);
  return(r == 255 && g == 255 && b == 255);
}

RE_LIB_ACCESS inline bool isPureBlack( const ReTexturePtr tex ) {
  if (tex->getType() != TexConstant) {
    return false;
  }

  int r,g,b;
  tex->getNamedValue("color").value<QColor>().getRgb(&r,&g,&b);
  return(r == 0 && g == 0 && b == 0);
}

RE_LIB_ACCESS inline bool isColorBlack( const QColor clr ) {
  int r,g,b;
  clr.getRgb(&r,&g,&b);
  return(r == g && g == b && r < 10);
}

RE_LIB_ACCESS inline bool isColorPureBlack( const QColor clr ) {
  int r,g,b;
  clr.getRgb(&r,&g,&b);
  return(r == g && g == b && r < 0);
}

RE_LIB_ACCESS inline bool isColorEmitting( const QColor clr ) {
  int r,g,b;
  clr.getRgb(&r,&g,&b);
  return(r == g && g == b && r > RE_COLOR_EMISSION_THRESHOLD);
}


RE_LIB_ACCESS inline bool isGrayscale( const QColor& clr ) {
  int r,g,b;
  clr.getRgb(&r, &g, &b);
  return (r == g == b);
}

RE_LIB_ACCESS inline void colorToGray( QColor& clr ) {
  QColor tmpClr = clr.toHsv();
  int h,s,v;
  tmpClr.getHsv(&h, &s, &v);
  tmpClr.setHsv(h, 0, v);
  clr = tmpClr.toRgb();  
}

RE_LIB_ACCESS inline bool isNullTexture( const Reality::ReTexturePtr ptr ) {
  return ptr.isNull();
}

RE_LIB_ACCESS inline ReTexturePtr createColorTexture( const QColor color, const QString name ) {
  ReTexturePtr tex = ReTexturePtr(new ReConstant(name, 0, color));
  return tex;
}

RE_LIB_ACCESS inline void gammaCorrect( QColor& clr, double gamma ) {
  double r,g,b;
  clr.getRgbF(&r, &g, &b);
  clr.setRgbF( pow(r, gamma),  pow(g, gamma), pow(b, gamma));
}

//! Lowers the brightness of a color by a multiplier
RE_LIB_ACCESS void dimColor( QColor& clr, const float amplitude );

//! Takes a complete file name and returns its path based on the
//! enclosing Runtime, if there is one. Otherwise the original
//! path is returned.
RE_LIB_ACCESS const QString normalizeRuntimePath( const QString& filePath );

/**
 * Converts, if necessary, a file path from the standard format to 
 * the native format. This means that, under Windows, it converts
 * from '/' to '\'
 */
RE_LIB_ACCESS QString toNativeFilePath( QString filePath );

/**
 * Converts, if necessary, a file path from the native format to 
 * the standard format. This means that, under Windows, it converts
 * from '\' to '/'
 */
RE_LIB_ACCESS QString toStandardFilePath( QString filePath );

//! Replaces illegal characters in a file name with underscores. This can
//! be used to convert a file name obtained from the object or material
//! name found in a scene.
RE_LIB_ACCESS QString sanitizeFileName( const QString& fileName );

//! Replaces illegal characters in a materials name with underscores. 
RE_LIB_ACCESS QString sanitizeMaterialName( const QString& fileName );

//! Given a texture, it return wheter that texture is or contains a Normal Map texture
RE_LIB_ACCESS bool containsNormalMap( const ReTexturePtr tex );
  
RE_LIB_ACCESS double convertUnit( const float value, const HostAppID scale );

/**
 * Convert a metric value, the native unit of measure of Lux and Reality,
 * to the unit used by the host-app.
 * Returns the value converted to the host-app metrics.
 */
RE_LIB_ACCESS double valueToHostApp( const float value, const HostAppID scale );

RE_LIB_ACCESS QString expandFrameNumber(const QString fileName, const uint frameNo );

RE_LIB_ACCESS uint getNumCPUs();

/**
 * Converts a vector from the Reality format to the LuxRender format.
 * Reality follows the conventions used by Poser and DAZ Studio: 
 *   - X moves to right
 *   - Y points up
 *   - Z moves toward the viewer
 * Luxrender uses this other format:
 *   - X moves to right
 *   - Z points up
 *   - Y moves away from the viewer
 */

RE_LIB_ACCESS void vectorToLux( ReVector& v );

//! Debugging tool that traverses a QVariantMap and returns a semi-formatted 
//! string with the representation of the map
RE_LIB_ACCESS QString reMapDebug( const QVariantMap& map, int level = 0 );

//! Debugging tool that traverses a QVariantList and returns a semi-formatted 
//! string with the representation of the list.RE_LIB_ACCESS 
RE_LIB_ACCESS QString listDebug( const QVariantList& l );

//! Uses the internal OS API to retrieve the full, absolute path of the 
//! running process.
RE_LIB_ACCESS QByteArray getApplicationPath();

/**
 * Scans a texture to find the image map that is contained.
 * If the texture is a compound one then the method scans recursively
 * until it finds a subtexture that has an image map.
 *
 * \return A string with the file name or and empty string if no image map was found.
 */
RE_LIB_ACCESS QString searchImageMap( const ReTexturePtr tex );

/**
 * Search a given texture for the first instance of an image map texture.
 * If the passed texture is an Image Map then the same texture is returned.
 * If the passed texture is either a Color Math or Mixer texture, then the
 * texture is traversed until the first Image Map texture is found
 * \return The Image Map texture, if found, a Null pointer otherwise.
 */
RE_LIB_ACCESS ReTexturePtr findImageMapTexture( const ReTexturePtr tex );


//! Given a texture it returns all the image maps used by that texture or all 
//! the sub-textures
RE_LIB_ACCESS void getImageMaps( const ReTexturePtr tex, QStringList& maps);

//! Given a texture it returns all the image maps used by that texture or all 
//! the sub-textures. If maps are not found in the diffuse channels then 
//! the colors are returned as identifiers for those channels.
//! This function is used to compute the correct ACSEL ID even in the case
//! of a material that uses solid colors instead of image maps.
RE_LIB_ACCESS void getImageMapsOrColors( const QVariantMap& shaderData, 
                                         QStringList& maps );
//! Overloaded version that works on data from the shader code saved
//! in ACSEL, but which has been converted to QVariantMap
//! \param shaderData The full QVariantMap object containing the shader
//!                   data as saved in the ACSEL database
//! \param maps A QStringList that will receive the file names of the 
//!             imagemaps found in the channel, if any. The list is not 
//!             emptied by this function prior adding the file names. 
RE_LIB_ACCESS void getImageMaps( const QVariantMap& shaderData, 
                                 QStringList& maps );

//! Applies a transformation to all the image map textures pointed by tex
RE_LIB_ACCESS void transformImageMaps( Reality::ReTexturePtr tex, ReTextureTransfFunc f );

//! Export a material, including its textures and linked volumes,
//! to a string
RE_LIB_ACCESS QString exportMaterialToString( ReMaterial* mat );

//! Writes a resource file to an external file
RE_LIB_ACCESS bool writeResourceToFile( const QString& resName, 
                                        const QString& destDirName );
}
#endif