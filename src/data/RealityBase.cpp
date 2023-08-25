/*
 File: RealityBase.cpp

 We implement the log by simply redirecting Qt's Debug output to a
 file on disk.
 The log file location is set to be in the user's Documents folder.
 */
#include <QDesktopServices>
#include <QJson/Parser>

#include "ReVersion.h"
#include "RealityBase.h"
#include "ReIPC.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "RealityRunner.h"
#include "ReLogger.h"
#include "ReAcsel.h"

using namespace Reality;

#define SET_DEFAULT_CONFIG( configEntry, cfgValue) \
  if (!configuration->contains(configEntry)) {  \
    configuration->setValue(configEntry, cfgValue); \
  }

// Static variables definitions
QStack<QString> RealityBase::commands;
ReConfigurationPtr RealityBase::configuration;
QStringList RealityBase::libraryPaths;
// LuxLibraryLoaderPtr RealityBase::luxLib;

//! Location of the Library in the file system
QString RealityLibPath;

/***************************************************
 * Initialization of the library path
 ***************************************************/

#ifdef WIN32 

#include <windows.h>

// From: http://www.codeguru.com/cpp/w-p/dll/tips/article.php/c3635/Tip-Detecting-a-HMODULEHINSTANCE-Handle-Within-the-Module-Youre-Running-In.htm
#if _MSC_VER >= 1300    // for VC 7.0
  // from ATL 7.0 sources
  #ifndef _delayimp_h
  extern "C" IMAGE_DOS_HEADER __ImageBase;
  #endif

HMODULE getCurrentModule(){  
  return reinterpret_cast<HMODULE>(&__ImageBase);
}
#endif

void initLibraryPath() {
  TCHAR dllPath[_MAX_PATH];
  // GetModuleFileName(GetModuleHandle("Reality_3.dll"), dllPath, _MAX_PATH);
  GetModuleFileName(getCurrentModule(), dllPath, _MAX_PATH);
  RealityLibPath = QString(dllPath);
}

#elif defined(__APPLE__)

#include <dlfcn.h>

void initLibraryPath() {
  Dl_info dl_info;
  dladdr((void *)initLibraryPath, &dl_info);
  RealityLibPath = QString(dl_info.dli_fname);
}

#endif

RealityBase::RealityBase() {
  hostAppID = Unknown;
  hostVersion = "";
  ReLibraryVersion = QString("%1.%2.%3.%4")
                       .arg(REALITY_MAIN_VERSION)
                       .arg(REALITY_SUB_VERSION)
                       .arg(REALITY_PATCH_VERSION)
                       .arg(REALITY_BUILD_NUMBER);

# ifdef Q_OS_MAC
  osType = MAC_OS;
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
  osType = WINDOWS;
#elif defined(Q_OS_LINUX)    
  osType = LINUX;
#endif
}

/**
 The Dynamic Library "main" function
 */
void RealityBase::main() {
  // Start the logging of events right here. The "H" indicates the host side and will
  // be used to mark the log messages so that it will be clear which ones are from
  // the host and which ones are from the UI.
  RE_createLogger();
  initLibraryPath();

  auto self = RealityBase::getRealityBase();
  self->undoStack = NULL;

  configuration = ReConfigurationPtr(
                    new QSettings(RE_CFG_DOMAIN_NAME, RE_CFG_APPLICATION_NAME)
                  );
  configuration->setValue(RE_CFG_VERSION, 
                          QString("%1.%2")
                            .arg(REALITY_MAIN_VERSION)
                            .arg(REALITY_SUB_VERSION));

  RE_LOG_INFO() << "=====================================";
  RE_LOG_INFO() << " Reality library loaded";
  RE_LOG_INFO() << "=====================================";
  RE_LOG_INFO() << QString("The library path is: %1").arg(RealityLibPath);
  RE_LOG_INFO() << QString("Reality v.%1.%2.%3.%4")
                     .arg(REALITY_MAIN_VERSION)
                     .arg(REALITY_SUB_VERSION)
                     .arg(REALITY_PATCH_VERSION)
                     .arg(REALITY_BUILD_NUMBER);  

  RE_LOG_INFO() << "SQLite v." << SQLite::VERSION;
  RE_LOG_INFO() << "SQLiteC++ v." << SQLITECPP_VERSION;
  RE_LOG_INFO() << "ACSEL library v." << RE_ACSEL_VERSION;
  RE_LOG_INFO() << "Reality executable in: " 
                << self->getRealityExecutablePath();
  self->findRenderers();
  RE_LOG_INFO() << "Embedded Lux in: " 
                << self->getRendererPath(LuxRender);

  auto slgPath = self->getRendererPath(SLG);
  if (!slgPath.isEmpty()) {
    RE_LOG_INFO() << "SLG in: " << slgPath;
  }
  realityIPC = NULL;


  // Initialize the configuration
  SET_DEFAULT_CONFIG(RE_CFG_CONVERT_INFINITE, false)

  // By default we ignore infinite lights in the scene
  SET_DEFAULT_CONFIG(RE_CFG_IGNORE_INFINITE, false)

  SET_DEFAULT_CONFIG(RE_CFG_USE_REAL_SPOTS, false)

  SET_DEFAULT_CONFIG(RE_CFG_LUX_DISPLAY_REFRESH, 15)
  SET_DEFAULT_CONFIG(RE_CFG_LUX_WRITE_INTERVAL,  60)
  SET_DEFAULT_CONFIG(RE_CFG_OVERWRITE_WARNING,   true)
  SET_DEFAULT_CONFIG(RE_CFG_MAT_PREVIEW_CACHE_SIZE, 5)
  // SET_DEFAULT_CONFIG(RE_CFG_USE_GPU, false)
  SET_DEFAULT_CONFIG(RE_CFG_OCL_GROUP_SIZE, 0)
  SET_DEFAULT_CONFIG(RE_CFG_KEEP_UI_RESPONSIVE, false)
  SET_DEFAULT_CONFIG(
    RE_CFG_DEFAULT_SCENE_LOCATION, 
    QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)
  )
  // SET_DEFAULT_CONFIG(RE_CFG_USE_NATIVE_UI, false)
  SET_DEFAULT_CONFIG(RE_CFG_MAX_SAMPLES_FIRST_TIME, true)
  // Remind the user about the consequence of flagging a shader set as 
  // generic
  SET_DEFAULT_CONFIG(RE_CFG_GENERIC_FLAG_REMINDER, true)

  // luxLib = LuxLibraryLoaderPtr(new LuxLibraryLoader(luxPath));

  // Create the global variable used to access the scene data
  RealitySceneData = new ReSceneData();
}

// Destructor: ~RealityBase
RealityBase::~RealityBase() {  
  delete RealitySceneData;
  RE_closeLogger();
}

void RealityBase::startHostSideServices( const HostAppID appID ) {
  CustomLogMessage::setPrefix("H");
  // Start the command polling thread. This is our "event loop"
  hostAppID = appID;
  // initExclusionList();
  RE_LOG_INFO() << "Starting the host-side services for " 
                << getPrintableHostAppID();
  realityIPC = new CommandPollingThread(appID);
  realityIPC->start();
}

void RealityBase::stopHostSideServices() {
  if (realityIPC) {
    RE_LOG_INFO() << "Host-side services shutting down...";

    realityIPC->shutDown();
    if (!realityIPC->wait(500)) {
      // If the thread doesn't quit in the allocated time then forcibly terminate it
      realityIPC->terminate();
    }
    // Erase the temporary UUIDs at the end of the session
    ReAcsel::getInstance()->eraseTempData();
    delete realityIPC;
  }
}

QStringList& RealityBase::getLibraryPaths() {
  return libraryPaths;
};

void RealityBase::setLibraryPaths( const QStringList& newList ) {
  libraryPaths = newList;
};

// Method: getConfiguration
ReConfigurationPtr RealityBase::getConfiguration() {
  return configuration;
}

void RealityBase::commandStackPush( const QString cmd ) {
  commands.push(cmd);
}

QString RealityBase::commandStackPop() {
  return commands.pop();
}

qint16 RealityBase::getNumCommands() {
  return commands.count();
}

void RealityBase::setHostAppID( const HostAppID _hostAppID ) {
  hostAppID = _hostAppID;
}

void RealityBase::setHostAppID( QString idString ) {
  idString = idString.toLower();
  if (idString == "poser") {
    hostAppID = Poser;
  }
  else if (idString == "dazstudio") {
    hostAppID = DAZStudio;
  }
  else if (idString == "blender") {
    hostAppID = Blender;
  }
  else if (idString == "zbrush") {
    hostAppID = ZBrush;
  }
  else if (idString == "modo") {
    hostAppID = Modo;
  }
  else if (idString == "reality") {
    hostAppID = RealityPro;
  }
  else {
    hostAppID = Unknown;
  }
}

HostAppID RealityBase::findHostAppID( QString hostAppNameStr ) const {
  hostAppNameStr = hostAppNameStr.toLower();
  if (hostAppNameStr == "poser") {
    return Poser;
  }
  else if (hostAppNameStr == "dazstudio") {
    return DAZStudio;
  }
  else if (hostAppNameStr == "blender") {
    return Blender;
  }
  else if (hostAppNameStr == "zbrush") {
    return ZBrush;
  }
  else if (hostAppNameStr == "modo") {
    return Modo;
  }
  else if (hostAppNameStr == "reality") {
    return RealityPro;
  }
  return Unknown;
}

QString RealityBase::getHostAppIDAsString() const {
  switch(hostAppID) {
    case RealityPro:
      return "Reality";
    case Poser:
      return "Poser";
    case DAZStudio:
      return "DAZStudio";
    case Blender:
      return "Blender";
    case ZBrush:
      return "ZBrush";
    case Modo:
      return "Modo";
    default:
      return "Unknown";
  }
}

QString RealityBase::getPrintableHostAppID() const {
  switch(hostAppID) {
    case RealityPro:
      return "Standalone";
    case Poser:
      return "Poser";
    case DAZStudio:
      return "DAZ Studio";
    case Blender:
      return "Blender";
    case ZBrush:
      return "ZBrush";
    case Modo:
      return "Modo";
    default:
      return "Unknown";
  }
}

/**
 * Static variable used to automatically execute our main()
 * !StaticGlobalObject
 */
static RealityBase* realityBase = NULL;

RealityBase* RealityBase::getRealityBase() {
  if (!realityBase)
  {
      realityBase = new RealityBase();
      realityBase->main();
  }

  return realityBase;
}

QUndoStack* RealityBase::getUndoStack() {
  // We delay creating the undo stack until the first request for it
  if (!realityBase->undoStack) {
    realityBase->undoStack = new QUndoStack();
    realityBase->undoStack->setUndoLimit(30);
  }
  return realityBase->undoStack;
}

void RealityBase::startACSELCaching() {
  ReAcsel::getInstance()->startCaching();
};

void RealityBase::stopACSELCaching() {
  ReAcsel::getInstance()->stopCaching();
};

QString RealityBase::getRealityExecutablePath() {
  QString baseDir;
  // We start checking if this is the Poser version. In that case
  // both the plugin library and the executable are in the same
  // dir, inside the Runtime/Python/addons dir of Poser
  baseDir = QFileInfo(RealityLibPath).absolutePath();
  #ifdef __APPLE__
    QFileInfo fi(QString("%1/Reality.app/Contents/MacOS/Reality").arg(baseDir));
  #elif defined(WIN32)
    QFileInfo fi(QString("%1/Reality.exe").arg(baseDir));    
  #endif
  if (fi.exists() && fi.isExecutable()) {
    return fi.absolutePath();
  }
  // Let's check if this is the instance loaded by the Reality executable.
  // On OS X the library is inside the bundle
  #ifdef __APPLE__
    fi.setFile((QString("%1/Reality").arg(baseDir)));
    if (fi.exists() && fi.isExecutable()) {
      return fi.absolutePath();
    }
  #endif

  // If we get here then it means that we are using the Studio version.
  // In this case we check the configuration, the installer sets it.
  baseDir = getConfiguration()->value(RE_CFG_REALITY_DS_LOCATION).toString();
  RE_LOG_INFO() << "  Searching " << baseDir;
  #ifdef __APPLE__
  fi.setFile(QString("%1/Reality.app/Contents/MacOS/Reality").arg(baseDir));
  #elif defined(WIN32)
  fi.setFile(QString("%1/Reality.exe").arg(baseDir));
  #endif
  if (fi.exists()) {
    RE_LOG_INFO() << "  Found " << fi.absoluteFilePath();
    return fi.absolutePath();
  }

  return "";
}

QString RealityBase::getRendererPath( const ReRenderers renderer ) {
  switch(renderer) {
    case LuxRender: {
      return rendererInfo.luxProgramPath;
    }
    case SLG: {
      return rendererInfo.slgProgramPath;
    }
  }
}

QString findLuxLocation( const QString& basePath ) {
  #ifdef __APPLE__
  QFileInfo appInfo(QString("%1/%2/LuxRender.app/Contents/MacOS/luxrender")
                      .arg(basePath)
                      .arg(LUX_SUBDIR));
  #elif defined(WIN32)
  QFileInfo appInfo(QString("%1/%2/luxrender.exe")
                      .arg(basePath)
                      .arg(LUX_SUBDIR));

  #endif
  if (appInfo.exists()) {
    return appInfo.absoluteFilePath();
  }
  return "";
}

QString findSLGLocation( const QString& basePath ) {
  QDir luxDir(basePath);
  luxDir.setFilter(QFlags<QDir::Filter>(
                      QDir::Files | QDir::Dirs | QDir::DirsLast | 
                      QDir::NoDotAndDotDot | QDir::Executable
                   ));
  auto entries = luxDir.entryInfoList();
  int numEntries = entries.count();
  for (int i = 0; i < numEntries; i++) {
    QString fName = entries[i].absoluteFilePath();
    if (entries[i].isFile() && entries[i].baseName().startsWith("slg")) {
      return fName;
    }
    else if (entries[i].isDir()) {
      fName = findSLGLocation(entries[i].absoluteFilePath());
      if (!fName.isEmpty()) {
        return fName;
      }
    }
  }
  return "";
}


void RealityBase::findRenderers() {
  QString searchRoot = getRealityExecutablePath();
  #if __APPLE__
    // The basePath is expected to be 
    // "[install root]/Reality.app/Contents/MacOS"
    searchRoot += "/../../..";
  #endif 
  rendererInfo.luxProgramPath = findLuxLocation(searchRoot);
  rendererInfo.slgProgramPath = findSLGLocation(searchRoot);
}

/** 
 * Code executed when the library is unloaded
 */
/*
#include <fstream>
__attribute__((destructor))
static void finalizer() {
  ofstream of("/Users/paolo/debug.txt", std::ofstream::app);
  of << QDateTime::currentDateTime()
          .toString("yy-M-dd hh:mm:ss").toAscii().data()
     << " Reality lib closing: " << __FILE__ << " -- " << __FUNCTION__ << endl;
  of.close();
}

*/
