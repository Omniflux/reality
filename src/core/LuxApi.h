/**
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

/**
 Prototypes for the Lux API. The use of this file makes it possible to completely avoid include any
 code from the Lux codebase. The few functions prototyped here are a very small subset of the Lux
 API and they are redefined manually "ex-novo"
 */

#ifndef RE_LUX_API_H
#define RE_LUX_API_H

#include <ReDefs.h>
#include <QString>
#include <QLibrary>
#include <QSharedPointer>

// Lux Version
typedef const char* (*luxVersionFunc)();

// Init. Not sure that we need it
typedef void (*luxInitFunc)();

// Cleanup
typedef void (*luxCleanupFunc)();

// Parse a scene 
typedef int (*luxParseFunc)(const char *filename);

// Reset the status of Lux
typedef void (*luxWorldEndFunc)();

// Retrieve the framebuffer, the image rendered
typedef void (*luxUpdateFramebufferFunc)();
typedef unsigned char* (*luxFramebufferFunc)();

// Statistics allow us to know if the render is finished
typedef double (*luxStatisticsFunc)(const char *statName);

// Global variables to be initialized at runtime when the shared library is loaded
extern luxInitFunc              luxInit;
extern luxVersionFunc           getLuxVersion;
extern luxCleanupFunc           luxCleanup;
extern luxParseFunc             luxParse;
extern luxWorldEndFunc          luxWorldEnd;
extern luxFramebufferFunc       luxFramebuffer;
extern luxUpdateFramebufferFunc luxUpdateFramebuffer;
extern luxStatisticsFunc        luxStatistics;


class RE_LIB_ACCESS LuxLibraryLoader {

private:
  static QLibrary luxLibrary;

  QString luxPath;
public:

  LuxLibraryLoader( const QString& luxPath );

  ~LuxLibraryLoader();

  // Library loading
  static bool isLuxLibraryLoaded;

  static void initLuxLibrary( const QString& luxPath );
  static void unloadLuxLibrary();
};

typedef QSharedPointer<LuxLibraryLoader> LuxLibraryLoaderPtr;

#endif