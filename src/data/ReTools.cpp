#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#include <Windows.h>
#endif

#include "ReTools.h"

#include <boost/any.hpp>
#include <QFileInfo>

#include "ReLogger.h"
#include "ReNodeConverter.h"
#include "exporters/lux/ReLuxMaterialExporter.h"
#include "exporters/lux/ReLuxMaterialExporterFactory.h"
#include "exporters/lux/ReLuxTextureExporter.h"
#include "exporters/lux/ReVolumeExporter.h"
#include "textures/ReImageMap.h"


namespace Reality {

// Max size for the application name. This is set to a large number to 
// allow the OS functions to store names of any reasonable length
#define RE_MAX_APP_NAME_SIZE 32000

//! One Poser Native Unit is 2.62128 meters
#define RE_PNU_TO_METERS 2.62128

#if defined(__APPLE__)

// Retrieves the full path of the current running application. Mac OS version
QByteArray getApplicationPath() {
  QByteArray appName;
  appName.reserve(RE_MAX_APP_NAME_SIZE);

  uint32_t pathNameSize = RE_MAX_APP_NAME_SIZE;
  _NSGetExecutablePath(appName.data(), &pathNameSize);
  return appName;
}

#elif defined(_WIN32)
// Retrieves the full path of the current running application. Windows version
QByteArray getApplicationPath()  {
  QByteArray appName;
  appName.reserve(RE_MAX_APP_NAME_SIZE);

  GetModuleFileNameA(NULL, appName.data(), (DWORD)RE_MAX_APP_NAME_SIZE);
  return appName;
}

#endif

const QString normalizeRuntimePath( const QString& filePath ) {
#if defined(_WIN32)
  int index = filePath.indexOf("\\runtime\\", 0, Qt::CaseInsensitive);
  // Try again in case the path is using / instead
  if (index == -1) {
    index = filePath.indexOf("/runtime/", 0, Qt::CaseInsensitive);
  }
#else
  int index = filePath.indexOf("/runtime/", 0, Qt::CaseInsensitive);
#endif
  
  if (index != -1) {
    return "@" + filePath.mid(index+1);
  }
  return filePath;
}


QString toNativeFilePath( QString filePath ) {
#if defined(_WIN32)
  return filePath.replace('/', '\\');
#else
  return filePath;
#endif  
}

QString toStandardFilePath( QString filePath ) {
#if defined(_WIN32)
  return filePath.replace('\\', '/');
#else
  return filePath;
#endif  
}

// //! This variable is used to return a sanitized file name efficiently
// QString _sanitizedFileName;

QString sanitizeFileName( const QString& fileName ) {
  QString s = fileName;
  return s.replace(':', '_')
          .replace(';','_')
          .replace('(','_')
          .replace(')','_')
          .replace('[','_')
          .replace(']','_')
          .replace('{','_')
          .replace('}','_')
          .replace('?','_')
          .replace('+','_')
          .replace('*','_')
          .replace('&','_')
          .replace('<','_')
          .replace('>','_')
          .replace('%','_')
          .replace('\'','_')
          .replace('/','_')
          .replace('"','_')
          .replace(' ','_');
}

QString sanitizeMaterialName( const QString& matName ) {
  QString s = matName;

  return s.replace('(','_')
          .replace(')','_')
          .replace('[','_')
          .replace(']','_')
          .replace('{','_')
          .replace('}','_')
          .replace('?','_')
          .replace('<','_')
          .replace('>','_')
          .replace('\'','_')
          .replace('/','_')
          .replace('"','_')
          .replace(' ','_');
}

void dimColor( QColor& clr, const float amplitude ) {
  // Save the original RGB
  QRgb originalRgb = clr.rgb();
  qreal r,g,b;
  clr.getRgbF(&r, &g, &b);
  clr.setRgbF(r * amplitude, g * amplitude, b * amplitude);
  // if the adjusted specular reaches pure black (0,0,0), restore the original
  // unless the original was also black, in that case set the default to 5,5,5
  QRgb rgb = clr.rgb();
  if (rgb == 0xFF000000) {
    if (originalRgb > 0xFF000000) {
      clr.setRgb(originalRgb);
    }
    else {
      clr.setRgb(5,5,5);        
    }
  }  
}

double convertUnit( const float value, const HostAppID scale ) {
  switch( scale ) {
    case Poser:
      return value * RE_PNU_TO_METERS;
    case DAZStudio:
      return value / 100;
    case ZBrush:
      break;
    case Modo:
      break;
    case Blender:
    case RealityPro:
      return value;
    case Unknown:
      break;
  }
  return value;
}

double valueToHostApp( const float value, const HostAppID scale ) {
  switch( scale ) {
    case Poser:
      return value / RE_PNU_TO_METERS;
    case DAZStudio:
      return value * 100;
    case ZBrush:
      break;
    case Modo:
      break;
    case Blender:
    case RealityPro:
      return value;
    case Unknown:
      break;
  }
  return value;
}

/**
 * Expand the frame number, or any number passed in the frameNo, in place of a number of pound signs in the string.
 * For example: reality_scene_###.png => reality_scene_042.png if frameNo == 42
 */
QString expandFrameNumber(const QString fileName, const uint frameNo ) {
  QRegExp exp("^([^#]+)([#]+)(.*)");
  if (exp.indexIn(fileName) != -1) {
    return(QString("%1%2%3")
           .arg(exp.cap(1))
           .arg(frameNo,exp.cap(2).length(),10,QLatin1Char('0'))
           .arg(exp.cap(3)));
  }
  return(fileName);
};

void vectorToLux( ReVector& v ) {
  auto tmp = v.Y;
  v.Y = -v.Z;
  v.Z = tmp;
}

QString searchImageMap( const ReTexturePtr tex ) {
  switch( tex->getType() ) {
    case TexImageMap:
      return tex->getNamedValue("fileName").toString();
    case TexColorMath:
    case TexMix: {
      ReTexturePtr tmpTex = tex->getNamedValue("texture1").value<ReTexturePtr>();
      if (!tmpTex.isNull()) {
        return searchImageMap(tmpTex);
      }
      tmpTex = tex->getNamedValue("texture2").value<ReTexturePtr>();
      if (!tmpTex.isNull()) {
        return searchImageMap(tmpTex);
      }
    }
    default:
      return "";
  }
  return "";
}

ReTexturePtr findImageMapTexture( const ReTexturePtr tex ) {
  switch( tex->getType() ) {
    case TexImageMap:
      return tex;
    case TexColorMath:
    case TexMix: {
      ReTexturePtr tmpTex = tex->getNamedValue("texture1").value<ReTexturePtr>();
      if (!tmpTex.isNull()) {
        return findImageMapTexture(tmpTex);
      }
      tmpTex = tex->getNamedValue("texture2").value<ReTexturePtr>();
      if (!tmpTex.isNull()) {
        return findImageMapTexture(tmpTex);
      }
    }
    default:
      return ReTexturePtr();
  }
  return ReTexturePtr();
}

void getImageMaps( const ReTexturePtr tex, QStringList& maps ) {
  if (tex.isNull()) {
    return;
  }
  switch( tex->getType() ) {
    case TexImageMap: {
      QString fname = tex->getNamedValue("fileName").toString();
      if (!maps.contains(fname)) {
        maps.append(fname);
      }
      break;
    }
    case TexBand: {
      int numBands = tex->getNamedValue("count").toInt();
      for (int i = 0; i < numBands; i++) {
        getImageMaps(
          tex->getNamedValue(QString("tex%1").arg(i)).value<ReTexturePtr>(),
          maps
        );
      }
      break;
    }
    case TexCheckers: {
      getImageMaps(tex->getNamedValue("tex1").value<ReTexturePtr>(), maps);
      getImageMaps(tex->getNamedValue("tex2").value<ReTexturePtr>(), maps);
      break;
    }
    case TexMath:
    case TexColorMath:
    case TexMix: {
      ReTexturePtr tmpTex = tex->getNamedValue("texture1").value<ReTexturePtr>();
      if (!tmpTex.isNull()) {
        getImageMaps(tmpTex, maps);
      }
      tmpTex = tex->getNamedValue("texture2").value<ReTexturePtr>();
      if (!tmpTex.isNull()) {
        getImageMaps(tmpTex, maps);
      }
      break;
    }
    case TexGrayscale: {
      getImageMaps(
        tex->getNamedValue("texture").value<ReTexturePtr>(),
        maps
      );
      break;
    }
    default: {
      //nothing;
    }
  }
}

// Utility function for getImageMaps() below

void examineTextureForImageMap( const QVariantMap& t, QStringList& maps ) {
  if (!t.contains("type")) {
    return;
  }
  switch(static_cast<ReNodeConverter::nodeTypes>(t["type"].toInt())) {
    case ReNodeConverter::ImageMap : {
      QString s =  t["fileName"].toString();
      if (s[0] == '@') {
        // Remove the first character that is the "@" used to 
        // signal the texture being relative to the runtime
        s = s.mid(1);
      }
      if (!maps.contains(s)) {
        maps << s;
      }
      break;
    }
    case ReNodeConverter::Band: {
      examineTextureForImageMap(t["tex1"].toMap(), maps);
      examineTextureForImageMap(t["tex2"].toMap(), maps);
      break;
    }    
    case ReNodeConverter::Math:
    case ReNodeConverter::ColorMath: {
      examineTextureForImageMap(t["value 1"].toMap()["map"].toMap(), maps);
      examineTextureForImageMap(t["value 2"].toMap()["map"].toMap(), maps);
      break;
    }
    case ReNodeConverter::Mix: {
      examineTextureForImageMap(t["tex1 map"].toMap(), maps);
      examineTextureForImageMap(t["tex2 map"].toMap(), maps);
      break;
    }
    default:
      break;
  }
}

void getImageMaps( const QVariantMap& shaderData, QStringList& maps ) {
  auto diffuseName  = shaderData["diffuse"].toMap()["map"].toString();
  auto diffuse2Name = shaderData["diffuse 2"].toMap()["map"].toString();
  auto alphaName    = shaderData["alpha"].toMap()["map"].toString();

  QVariantMap diffuse, diffuse2, alpha;
  if (!diffuseName.isEmpty()) {
    diffuse = shaderData["nodes"].toMap()[diffuseName].toMap();
  }
  if (!diffuse2Name.isEmpty()) {
    diffuse2 = shaderData["nodes"].toMap()[diffuse2Name].toMap();
  }
  if (!alphaName.isEmpty()) {
    alpha = shaderData["nodes"].toMap()[alphaName].toMap();
  }

  examineTextureForImageMap(diffuse, maps);
  examineTextureForImageMap(diffuse2, maps);
  examineTextureForImageMap(alpha, maps);
}

void getImageMapsOrColors( const QVariantMap& shaderData, 
                           QStringList& maps ) 
{
  auto diffuseName  = shaderData["diffuse"].toMap()["map"].toString();
  auto diffuse2Name = shaderData["diffuse 2"].toMap()["map"].toString();
  auto alphaName    = shaderData["alpha"].toMap()["map"].toString();

  QVariantMap diffuse, diffuse2, alpha;
  // If there is a map then search if there is an image file
  if (!diffuseName.isEmpty()) {
    diffuse = shaderData["nodes"].toMap()[diffuseName].toMap();
    examineTextureForImageMap(diffuse, maps);
  }
  else {
    if (shaderData["diffuse"].toMap().contains("color")) {
      maps << convertFloatColor(
                shaderData["diffuse"].toMap()["color"].toList()
              ).name();
    }
  }
  if (!diffuse2Name.isEmpty()) {
    diffuse2 = shaderData["nodes"].toMap()[diffuse2Name].toMap();
    examineTextureForImageMap(diffuse2, maps);
  }
  if (!alphaName.isEmpty()) {
    alpha = shaderData["nodes"].toMap()[alphaName].toMap();
    examineTextureForImageMap(alpha, maps);
  }
}

bool containsNormalMap( const ReTexturePtr tex ) {
  switch(tex->getType()) {
    case TexImageMap:
      return tex.staticCast<ImageMap>()->isNormalMap();
    case TexMix:
    case TexMath:
    case TexColorMath: {
      if (!containsNormalMap(tex->getNamedValue("texture1").value<ReTexturePtr>())) {
        auto tex2 = tex->getNamedValue("texture2").value<ReTexturePtr>();
        return (tex2.isNull() ? false : containsNormalMap(tex2));
      }
      else {
        return true;
      }
      break;
    }
    default:
      return false;
  }
  return false;
}

QString exportMaterialToString( ReMaterial* mat ) {
  ReLuxTextureExporter::initializeTextureCache();
  ReLuxTextureExporter::enableTextureCache(false);
  // Get the material definition
  ReLuxMaterialExporterPtr exporter = ReLuxMaterialExporterFactory
                                        ::getExporter(mat);

  boost::any textureData;
  exporter->exportTextures(mat, textureData);
  QString matDefinition;
  try {
    matDefinition = boost::any_cast<QString>(textureData);
  }
  catch( const boost::bad_any_cast& e ) {
    RE_LOG_INFO() << "Exception " << e.what();
    matDefinition = "";
  }

  boost::any matData;
  exporter->exportMaterial(mat, matData);
  matDefinition += boost::any_cast<QString>(matData);
  // Export the volumes
  QString volumeDefinition;
  ReLuxVolumeExporter volExporter;

  return volumeDefinition + matDefinition;
}

bool writeResourceToFile( const QString& resName, const QString& destDirName) {
  QFile theFile(resName);
  if (theFile.open(QIODevice::ReadOnly)) {
    auto content = theFile.readAll();
    theFile.close();

    QString fName = QFileInfo(resName).fileName();
    theFile.setFileName(QString("%1/%2").arg(destDirName).arg(fName));
    if (theFile.open(QIODevice::WriteOnly)) {
      theFile.write(content);
      theFile.close();
      return true;
    }
  }
  return false;
}

void transformImageMaps( ReTexturePtr tex, ReTextureTransfFunc f ) {
  if (tex.isNull()) {
    return;
  }
  switch( tex->getType() ) {
    case TexImageMap: {
      f(tex);
      break;
    }
    case TexBand: {
      int numBands = tex->getNamedValue("count").toInt();
      for (int i = 0; i < numBands; i++) {
        transformImageMaps(
          tex->getNamedValue(QString("tex%1").arg(i)).value<ReTexturePtr>(),
          f
        );
      }
      break;
    }
    case TexCheckers: {
      transformImageMaps(tex->getNamedValue("tex1").value<ReTexturePtr>(), f);
      transformImageMaps(tex->getNamedValue("tex2").value<ReTexturePtr>(), f);
      break;
    }
    case TexMath:
    case TexColorMath:
    case TexMix: {
      ReTexturePtr tmpTex = tex->getNamedValue("texture1").value<ReTexturePtr>();
      if (!tmpTex.isNull()) {
        transformImageMaps(tmpTex, f);
      }
      tmpTex = tex->getNamedValue("texture2").value<ReTexturePtr>();
      if (!tmpTex.isNull()) {
        transformImageMaps(tmpTex, f);
      }
      break;
    }
    case TexGrayscale: {
      transformImageMaps(tex->getNamedValue("texture").value<ReTexturePtr>(),f);
      break;
    }
    default: {
      //nothing;
    }
  }
}

uint getNumCPUs() {
   uint cpuCount = 1;

#if defined(_WIN32)

   SYSTEM_INFO    si;
   GetSystemInfo(&si);
   cpuCount = si.dwNumberOfProcessors;

#elif defined(__APPLE__)

   kern_return_t    kr;
   struct host_basic_info hostinfo;
   unsigned int     count;

   count = HOST_BASIC_INFO_COUNT;
   kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostinfo, &count);
   if (kr == KERN_SUCCESS) {
     cpuCount = hostinfo.avail_cpus;
   }

#endif

  if ( cpuCount < 1 ) {
    cpuCount = 1;
  }
    
  return cpuCount;
}

//! Debugging tools
QString listDebug( const QVariantList& l ) {
  QString str;
  int numItems = l.count();
  for (int i = 0; i < numItems; i++) {
    QVariant v = l[i];
    switch( v.type() ) {
      case QMetaType::Bool: {
        str += QString("%1").arg(v.toBool());
        break;
      }
      case QMetaType::QString: {
        str += QString("%1").arg(v.toString());
        break;
      }
      case QMetaType::Double:
      case QMetaType::Float: {
        str += QString("%1").arg(v.toDouble());
        break;
      }
      case QMetaType::QVariantList : {
        QString listStr = listDebug(v.toList());
        str += QString("( %1 )").arg(listStr);
        break;
      }
      case QMetaType::QVariantMap:
        str += reMapDebug(v.toMap());
        break;
      default: {
        RE_LOG_INFO() << "Unhandled: " << QVariant::typeToName(v.type());
        str += QString("%1").arg(v.toString());
        break;
      }
    }
    if (i < numItems-1) {
      str += ", ";
    }
  }
  return str;
}

QString reMapDebug( const QVariantMap& map, int level ) {
  QString blanks;
  for (int i = 0; i < level; i++) {
    blanks += " ";
  }
  QMapIterator<QString, QVariant> i(map);
  QString msg;
  while( i.hasNext() ) {
    i.next();
    QString key = i.key();
    int keyLength = key.length();
    QString keyIndent;
    for (int j = 0; j < keyLength; j++) {
      keyIndent += " ";
    }
    QVariant value = i.value();
    QString valStr;
    switch( value.type() ) {
      case QMetaType::QVariantList:
        valStr = "[" + listDebug(value.toList()) + "]";
        break;
      case QMetaType::QVariantMap:
        valStr = "{\n" + 
                  keyIndent + blanks + 
                  reMapDebug(value.toMap(), keyLength + level+2) +
                  keyIndent + blanks + "}\n";
        break;
      default:
        valStr = value.toString();
    }
    msg += QString("%1%2: %3\n").arg(blanks).arg(key).arg(valStr);
  }
  return msg;
}

}
