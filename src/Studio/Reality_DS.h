/**
 *  Reality_DS.h
 *  
 *  Startup code for the Reality plugin for DAZ Studio proper
 *
 *  Created by Paolo H Ciccone on 7/12/11 and moved to Reality 3 on 7/1/2013
 *  Copyright 2011 Pret-A-3D. All rights reserved.
 *
 */

#ifndef REALITY_FOR_STUDIO_H
#define REALITY_FOR_STUDIO_H
#include "zmq.hpp"

#include <QLibrary>
#include <QThread>

#include "dzplugin.h"
#include "dzpluginmgr.h"
#include "dzcamera.h"
#include "dzlight.h"
#include "dzskeleton.h"
#include "dzbasiccamera.h"
#include "dzapp.h"
#include "dzmainwindow.h"
#include "dzviewportmgr.h"
#include "dzviewport.h"
#include "dz3dviewport.h"

#include "ReDefs.h"
#include "ReLogger.h"
#include "ReDAZStudioEvent.h"
#include "ReDSMatCollection.h"
#include "ReGUID.h"

#include <ctime>

#define REALITY_DS_LIBRARY_VERSION "0.6"

#define REALITY_PLUGIN_NAME "Reality DS"
//! The interval, in milliseconds, at which we update the camera data 
//! via a timer
#define RE_REALITY_TIMER_INTERVAL 1000

//! The number of seconds we wait to add a figure since the last time it 
//! had been updated by Studio.
#define RE_FIGURE_ADDITION_DELAY 3

#define RE_DS_LIGHT_SPREAD_ANGLE "Spread Angle"
#define RE_DS_LIGHT_INTENSITY    "Intensity"
#define RE_DS_LIGHT_COLOR        "Color"
#define RE_DS_LIGHT_ILLUMINATION "Illumination"
//! Prefix used by the Uber lights. Reality checks if the node starts with
//! this prefix to determine if the node should be interepreted to be a 
//! mesh light 
#define RE_DS_UBER_LIGHT_PREFIX  "omLightPlane"

#define RE_DS_HEADLAMP_BLOCKER   "AreaLightSupport"

enum Reality2MaterialTypes {
  GLOSSY     =  10,
  MATTE      =  60,
  MATTETRANS =  70,
  GLASS      =  30,
  METAL      =  80,
  VELVET     =  90,
  MIRROR     = 100,
  MIX        = 110,
  FOG        = 120,
  WATER      = 160,
  NULL_MAT   = 150,
  LIGHT      = 160
};

typedef QSharedPointer<QVariantMap> ReVariantMapPtr;

// typedef QList<DzMaterial*> ReDSMaterialList;
// typedef QListIterator<DzMaterial*> ReDSMaterialListIterator;

//using namespace Reality::DS;

using namespace Reality;

class RE_LIB_ACCESS Reality_DS: public DzPlugin {

  Q_OBJECT

  private:
    //! Path to where Reality is installed
    QString realityPath;

    //! Identifies the OS we are using
    OSType osType;

    //! Timer used to update camera data
    QTimer* cameraTimer;

    //! To avoid sending camera data to the UI continuuosly, even when 
    //! a camera has not changed, we keep the data into a dictionary
    //! keyed by the camera ID. If the current camera data is different
    //! from the one in the dictionary then we send the update over the 
    //! wire.
    //! The disctionary hold data obtained with the ReCamera::toString()
    //! method, which is a "poor man's serializer."
    QHash<QString, QString> cameraData;

    //! Additional flag that signal if the Reality executable has been
    //! located correctly.
    bool realityActive;

    //! Used to inhibit processing some signals while Studio is shutting down
    bool shuttingDown;

    //! List of nodes to be added to the scene
    DzNodeList nodeList;

    //! List of nodes that need to be monitored. This list is populated
    //! when reading the data from a saved scene. Objects and materials
    //! are not added via the standard Studio procedure, instead they are 
    //! loaded from that Reality data. This list is used to set the monitoring
    //! for those nodes.
    // DzNodeList monitoringList;
    QList<int> monitoringList;

    //! Indicator of how many scene are being loaded. When this number
    //! reaches 0 and there is a ReDAZStudioEvent event in the queue
    //! then the nodes in nodeList will be added to Reality.
    int sceneLevel;

    //! This flag indicates if we are currently during a scene loading
    //! sequence. The flag is then used by Reality to collect the references
    //! to Studio nodes to be added to the Reality database when the scene
    //! is finally loaded.
    bool isSceneLoading;

    bool sceneIsMerging;

    QString orderNo,
            serialNo;
    //! An unsigned int that stores the size of the data sent by the server
    int dataSize;
    //! The server response data when we ask for updates/license verification
    QByteArray response;

    /**
     * A flag to signal if the addition of nodes should be suspended
     * This is used when we load data from the scene. Studio notifies
     * the addition of a node the moment the empty instance of a node
     * class is added to the scene, instead of notifying when the 
     * complete node is available. This means that we might receive the
     * signal before the node data of Reality is loaded from the scene file.
     * The GUID of a node is stored in the scene and if Reality gets called
     * to add a node before its GUID is available the program will add two
     * instances of the same node.
     * In addition to the above, going through the addition process for 
     * nothing is just wasteful. This flag, and related static function to
     * set it, is used to act intellingently during the data load.
     */
    static bool nodeAdditionActive;

    /**
     * Anti-tamper system to detect possible hacking of the binary code.
     */
    enum tamperFlags {
      TAMPER_START            = 0xA0,
      NEW_REGISTRATION        = 0xF0,
      NEW_REGISTRATION_PASSED = 0xC0,
      OLD_REGISTRATION,
      OLD_REGISTRATION_PASSED,
      TAMPER_CLEAR            = 0xA1
    };
    //! Table used to list the materials that need to be refreshed.
    //! The table is index by <object id>::<material name> and it 
    //! contains the material index provided by Studio
    QHash<QString, int> materialRefreshTable;

    //! Flat material list that give us a way to access all the materials
    //! regardless of which object owns it
    ReDSMatCollection flatMatList;

    //! Updates the flatMatList object with the materials in the scene
    void updateFlatMaterialList();

    /**
     * Connects to the scene objects and sets all the functions to respond to
     * the signals that report changes to the scene.
     */
    void installHooks();

    //! Remove all the signal handlers installed 
    void removeHooks();

    //! Adds an instance of the our storage class to the DzScene instance
    void addSceneStorage();

    //! Collectall the elements in the scene and add them to the
    //! Reality database
    void scanScene( const bool installMonitor = false );

    //! Verifies that a valid installation of Reality is present
    //! in the path given.
    bool verifyRealityInstallation( const QString& realityPath );

    //! Camera updates
    void updateCameraData();

    void updateAnimationLimits();
  
    void renderFrame( const QString& sceneFileName = "", 
                      const unsigned int frameNo = 0,
                      const bool runRenderer = true );

    void monitorCamera( const DzCamera* cam );

    void monitorLight( const DzLight* light );
  
    /**
     * Renders the current frame by calling renderFrame(). This method might
     * seem redundant but it is defined as a way of doing all the prepartion
     * work outside the renderFrame() method since renderFrame() is used
     * by renderAnimation() to render each frame. There are operations that
     * need to executed only once per render cycle and which would be wasteful
     * if they were executed for each frame, when rendering animations.
     */
    void renderCurrentFrame( const QString& sceneFileName = "", 
                             const unsigned int frameNo = 0,
                             const bool runRenderer = true);

    void renderAnimation( const bool runRenderer, 
                          int selectedStartFrame = 0, 
                          int selectedEndFrame = 0 );


    //! Select a material in Studio. Generally as a response to the 
    //! user Alt-selecting the material in Reality
    void selectStudioMaterial( const QString& objID, const QString& materialName);

    //! Returns the current camera in Studio
    QString getCurrentStudioCamera() {
      // Select the active camera for the scene
      auto dsViewPort = dzApp->getInterface()->getViewportMgr()
                             ->getActiveViewport()->get3DViewport();
      auto currentCamera = dsViewPort->getCamera();
      QString camID;
      if (currentCamera) {
        camID = ReGUID::getGUID(currentCamera);        
      }
      else {
        camID = "";
      }
      return camID;
    }

    static Reality_DS* instance;

  public:
    //! Changes the state of the nodeAdditionActive flag. This method is used
    //! by the ReStorage class to communicate when to ignore addition of 
    //! nodes because they will be restored by the saved Reality data
    static void enableNodeAddition( const bool newState );

    static Reality_DS* getInstance();

    //! Constructor. It initializes the base class and sets
    //! the data structures for the DRM. It also initializes
    //! the skeleton-to-file-name table
    Reality_DS();
  
    ~Reality_DS();

    bool event( QEvent* );

    //! Search the list of objects that are getting loaded from a scene and
    //! find the object with id objID. If the object is found then the method
    //! determines if it needs a GUID. If so, the GUID is created, added to the
    //! node, returned in the newID parameter, the label of the object is
    //! set in the objLabel parameter and the method reruns true.
    bool calcIDForObject( const QString& objID, QString& newID, QString& objLabel );

    inline bool isSceneMerging() {
      return sceneIsMerging;
    }

  protected:

    void checkForUpdates();
    virtual void startup();
    virtual void shutdown();
    bool isRegistered() const;

    //! Adds a Reality object to the database
    void addRealityObject( const QString& objID, 
                           const DzNode* node );

    //! Starts adding a Studio node to the Reality database
    void addNode( DzNode* node );
  
    //! Event handler for when a node is added to the scene. This is launched
    //! by the event() method, as a result of processing the addition at the
    //! end of the event chain.
    void addNode( ReStudioEvent* e );

    //! Event handler for when the material is changed in Studio. This method
    //! is called by the event()method, as a result of processing the 
    //! modification of the material at the end of the event chain.
    void updateMaterial();

    void updateMaterialList();
   
  private:
    void getAnimationRange( int& startFrame, int& endFrame, int& fps );

  private slots:

    //! Background processing via a timer
    void processTimedEvents();

    //! Stores the definition of a camera
    void cameraAdded( DzCamera* camera );

    //! Removes a camera from the database
    void cameraRemoved( DzCamera* camera );
    //! Renames a camera already in the Reality database
    void cameraRenamed( const QString& newLabel );

    //! Adds a light tot he database
    void lightAdded( DzLight* light );

    //! Removes a light form the database
    void lightRemoved( DzLight* light );

    //! Renames a light
    void lightRenamed( const QString& newLabel );

    void nodeAdded( DzNode* node );
    void nodeRemoved( DzNode* node ) ;
    void nodeRenamed( const QString& newLabel );

/*
    //! Sends the needed information to the ObjectAdder thread
    //! for thr addition of a given object to the Reality database
    void queueObjectForAddition( const DzNode* skel );
*/

    void selectActiveCamera(DzCamera* dsCam);

    //! Executed when a Studio scene starts loading
    void sceneLoadingStarted();

    //! Executed when a scene is completely loaded
    void sceneLoaded();

    //! Sets the scene frame size in response to the change in the Studio
    //! render options
    void setFrameSize( const QSize& newSize );

    //! This method handles the change between visible/hidden state of a node
    void visibilityChanged();

    void watchMaterial();

    //! Monitors the selection of materials in Studio and synchronizes
    //! with Reality's UI.
    void materialSelected(DzMaterial *mat, bool onOff);

    //! Responds to the signal that a light transform has been changed by
    //! updating the light's position in the Reality database.
    void updateLightPos();

    //! Responds to the signal about a light property being changed and 
    //! updates the relevant data in Reality
    void updateLightProperty();

    //! Called in response to tranform change events of the IBL Preview Sphere
    void changeIBLRotation();

    //! When a material preset is applied several events can happen.
    //! Some times the property for a material is updated, other times
    //! a material list changes. Reality listens to the materialListChanged()
    //! signal of a DzShape and, if a new event happens, it updates the 
    //! material
    void materialListChanged();

    //! Checks the configuration for the ContentDir key. If found the value
    //! for that key is assumed to be the directory where the Reality addons
    //! have been installed. Reality the adds the directory to the Studio's
    //! configuration and removes the key from the comfiguration.    
    void installContentPath();
    
    void materialAdded( DzMaterial* dzMat );

    void materialRemoved( DzMaterial* dzMat );

    void shapeSwitchHandler();

  public slots:

    inline void writeToLog( const QString& msg ) {
      RE_LOG_INFO() << msg.toStdString();
    }

    //! Creating a new scene
    void setNewScene();
    
    //! Assigns an LDR to the IBL Preview Sphere
    void setIBLPreviewMap( const QString& mapFilename );
    
    //! Proxy method to forward it to the one in ReSceneData    
    void setIBLImage( const QString& mapFilename );

    //! Proxy method to forward it to the one in ReSceneData  
    void setLightIntensity( const QString& lightID, const float gain );

    //! Add signal monitoring for the events that Reality needs to
    //! track.
    void monitorNode(const DzNode* node);

    //! Updates the Library Paths used for the expansion of image map
    //! texture names.
    static void updateLibraryPaths();
    
};


#endif
