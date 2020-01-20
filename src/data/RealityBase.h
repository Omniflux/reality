/**
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2010. All rights reserved.
  \file
*/

#ifndef REALITYBASE_H
#define REALITYBASE_H

#include <QtCore>
#include <QStack>
#include <QSettings>
#include <QUndoStack>
#include <qglobal.h>

#include "ReDefs.h"
#include "ReLuxRunner.h"
#include "ReProductUsage.h"
// #include "LuxApi.h"
  
namespace Reality {

// Store information about Lux, for execution and communication
struct ReRendererInfo {
  QString luxProgramPath;
  QString slgProgramPath;
};

typedef QSharedPointer<QSettings> ReConfigurationPtr;
/**
 This class implements a simple "bootstrap" sequence for the Shared Library/DLL. In here we add thinsg that need to be
 done when the shared library is loaded, which usually happens when the plug-in is loaded in the hosting
 application. 

 The code for the plug-in registration goes here.

 The logging facility is started here.
 */
class RE_LIB_ACCESS RealityBase {

private:
  //! A stack of commands for the host-side portion of Reality, like the Python-based
  //! version that runs inside Poser. This is a way of communicating things like "start rendering"
  //! or "fetch a camera parameter" or other pieces of data that rely on the Python API.
  //! A command is pushed on the stack and then it's popped when requested. If the command
  //! has parameters then the parameters are pushed onto the stack in reversed order followed
  //! by the command. For example, a pseudo-function call: render(width, height) will be
  //! passed as:
  //!
  //!   - push height
  //!   - push width
  //!   - push "render"
  //! 
  //! That's why the stack uses QVariants for its data 
  static QStack<QString> commands;

  //! The application's Undo Stack
  QUndoStack* undoStack;

  //! Identity of the host we are connected to
  HostAppID hostAppID;
  //! Version of the host app
  QString hostVersion;

  //! Identifies the OS we are using
  OSType osType;

  static ReConfigurationPtr configuration;

  //! A list of paths where content is installed and that is used
  //! to resolve runtime paths for textures. All textures loaded from
  //! a scene that are prefixed with a "@" will be expanded to be in 
  //! one of the directories listed in the libraryPath list
  static QStringList libraryPaths;

  //! Stores the library version
  QString ReLibraryVersion;

  // static LuxLibraryLoaderPtr luxLib;

  ReRendererInfo rendererInfo;

  void findRenderers();

  //! Thread that sends product usage to Pret-a-3D whenever the user 
  //! loads a Poser or DAZ Studio material preset into the scene
  ReProductUsage* productUsage;
  
public:

  RealityBase();

  ~RealityBase();
  
  static void main();

  //! Get the instance of the RealityBase object.
  static RealityBase* getRealityBase();

  //! Returns a pointer to the Reality settings
  static ReConfigurationPtr getConfiguration();
  static QUndoStack* getUndoStack();

  //! Returns a list of paths used for the content library of the host
  static QStringList& getLibraryPaths();

  //! Saves the library paths sent by the host-app.
  //! The input is list of strings listing directory names
  static void setLibraryPaths( const QStringList& newList );

  inline OSType getOSType() {
    return osType;
  }

  void commandStackPush( const QString cmd );
  QString commandStackPop();

  //! Get the number of pending commands. Returns 0 if there are no
  //! pending commands.
  qint16 getNumCommands();
  
  void setHostAppID( const HostAppID _hostAppID );
  void setHostAppID( QString idString );

  inline HostAppID getHostAppID() const {
    return hostAppID;
  }

  inline const QString& getHostVersion() const {
    return hostVersion;
  }

  inline void setHostVersion( const QString& newVer ) {
    hostVersion = newVer;
  }

  //! Returns the version of the library in a format that is compatible
  //! with many interfaces, including Python
  inline char const* getLibraryVersion() {
    return ReLibraryVersion.toAscii().data();
  }

  //! Returns the <HostAppID> corresponding to 
  //! the name of the application. This is a service
  //! method normally called by the Host App to object
  //! its own ID inside Reality
  HostAppID findHostAppID( QString hostAppName ) const;

  QString getHostAppIDAsString() const;
  QString getPrintableHostAppID() const;

  //! Called to start the IPC services
  void startHostSideServices( const HostAppID appID = Poser );
  
  //! Called in exit, to stop the communication services with the GUI
  void stopHostSideServices();

  void startACSELCaching();
  void stopACSELCaching();

  //! Returns the path where the Reality executable is stored
  QString getRealityExecutablePath();

  //! Returns the path to the executable of a selected renderer
  QString getRendererPath( const ReRenderers renderer );
};

}

#endif
