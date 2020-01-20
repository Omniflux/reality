#include "LuxApi.h"
#include "ReLogger.h"

luxInitFunc              luxInit              = NULL;
luxVersionFunc           getLuxVersion        = NULL;
luxCleanupFunc           luxCleanup           = NULL;
luxParseFunc             luxParse             = NULL;
luxWorldEndFunc          luxWorldEnd          = NULL;
luxUpdateFramebufferFunc luxUpdateFramebuffer = NULL;
luxFramebufferFunc       luxFramebuffer       = NULL;
luxStatisticsFunc        luxStatistics        = NULL;

bool LuxLibraryLoader::isLuxLibraryLoaded = false;
QLibrary LuxLibraryLoader::luxLibrary;

void LuxLibraryLoader::initLuxLibrary( const QString& libraryPath ) {
  if (isLuxLibraryLoaded) {
    return;
  }

#if defined(Q_WS_MAC)  
  luxLibrary.setFileName(QString("%1/LuxRender.app/Contents/MacOS/liblux").arg(libraryPath));
  luxLibrary.load();
#elif defigned(Q_WS_WIN)
  luxLibrary(QString("%1/lux.dll").arg(libraryPath));
  luxlibrary.load();  
#endif
  luxLibrary.load();
  isLuxLibraryLoaded = luxLibrary.isLoaded();
  if (isLuxLibraryLoaded) {
    getLuxVersion        = (luxVersionFunc)           luxLibrary.resolve("luxVersion");
    luxParse             = (luxParseFunc)             luxLibrary.resolve("luxParse");
    luxInit              = (luxInitFunc)              luxLibrary.resolve("luxInit");
    luxCleanup           = (luxCleanupFunc)           luxLibrary.resolve("luxCleanup");
    luxWorldEnd          = (luxWorldEndFunc)          luxLibrary.resolve("luxWorldEnd");
    luxUpdateFramebuffer = (luxUpdateFramebufferFunc) luxLibrary.resolve("luxUpdateFramebuffer");
    luxFramebuffer       = (luxFramebufferFunc)       luxLibrary.resolve("luxFramebuffer");
    luxStatistics        = (luxStatisticsFunc)        luxLibrary.resolve("luxStatistics");
    RE_LOG_INFO() << "Lux library found: " << getLuxVersion();
  }
}

LuxLibraryLoader::LuxLibraryLoader( const QString& luxPath ) : 
  luxPath(luxPath)
{
  initLuxLibrary(luxPath);
}

LuxLibraryLoader::~LuxLibraryLoader() {
  unloadLuxLibrary();
}

void LuxLibraryLoader::unloadLuxLibrary() {
  if (luxLibrary.isLoaded()) {
    isLuxLibraryLoaded =  !luxLibrary.unload();

    getLuxVersion        = NULL;
    luxParse             = NULL;
    luxInit              = NULL;
    luxCleanup           = NULL;
    luxWorldEnd          = NULL;
    luxUpdateFramebuffer = NULL;
    luxFramebuffer       = NULL;
    luxStatistics        = NULL;
  }
}

