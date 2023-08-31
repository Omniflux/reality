/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */


#ifndef RESTORAGE_H
#define RESTORAGE_H

#include <QStack>
#include <QVariantMap>
#include <dzassetextraobjectio.h>
#include <dzcustomdata.h>


namespace Reality {

/**
 * \file
 * Classes used to handle the loading and saving of Reality data from the Studio
 * Scene.
 *
 * See the description for ReSceneBlock for a description of the whole process.
 *
 * History:
 *  Reality 1.0 used a DzSimpleSceneData object attached to the scene. This created problems because that
 *  class, at that time, had a limit of 64K data. If the stored data was bigger then there would be data loss.
 *  In Reality 1.2, 2.0 and 2.2 we used a custom-made class called ReSceneConfigData that simply extended the
 *  limit beyond 64K. It didn't do anything else special, the data was still stored using the Settings class
 *  converted to XMl string. Unlike what was done in Reality 1.0, the storage object is attached not to the
 *  Studio Scene but to an invisible node called "Reality_Scene_Data". This makes it possible to merge data
 *  when two Studio scenes are merged by the user.
 *
 *  With the introduction of the new DUF (DSON) file format in DS4.5 the ReSceneConfigData doesn't work anymore
 *  because it was tied to the .daz file format. At the same time the 64K limit of DzSimpleSceneData and 
 *  DzSimpleElementData has been removed so it is now usable, which simplifies things a lot. With Reality 2.5
 *  we went back to the DzSimpleElementData class, attached to the Reality_Scene_Data node.
 *
 * With Reality 4 we have a full C++ implementation. The major differences are:
 *  - After we read the data from the Reality_Scene_Data node we delete it
 *    from the scene. There is no need to keep it around. It will be re-created
 *    when the scene gets saved.
 *  - The data is saved in JSON format, exactly in the same format used for
 *    Reality for Poser.
 *
 * Parameters:
 *  appVersion - The current version of the application. Useful to verify compatibility in the future.
 */

#define RE_DS_DATANODE_NAME       "Reality_Scene_Data"
// Reality 2 special node that we need to ignore
#define RE_DS_METADATA            "Reality MetaData"
// ID for the Reality 3+ scene data
#define RE_DS_SCENEDATA_ID        "RealityEngineVersionedData"

// #define RE_DS_SCENEDATA_VERSION   "RealityEngineVersion"
// #define RE_DS_SCENEDATA_CONTAINER "RealityInformation"

/**
 *
 * ##Logic of the data loading in DAZ Studio.
 *
 * Studio stores data in a variant of the JSON format, called DSON.
 * Writing the data is straightforward, reading it is much more complex.
 * 
 * Studio uses a system of objects that feature several callbacks. The system
 * revolves around the idea of a "data context." The context keeps track of
 * what container is receiving the data read from the scene file.
 *
 * The intended design of the classes of Studio is to have a subclass of 
 * DzSceneData to hold the custom data. Since we store the Reality data
 * in a stand-alone structure that is not dependent from a specific host, we
 * don't use that class. It's defined here only because it's required by Studio.
 *
 * The reading process goes like this:
 * 
 * - A subclass of DzSceneData is registered together with a subclass
 *   of DzExtraSceneDataIO. They must be registered as plugins as usual and 
 *   then registered again with the DZ_PLUGIN_REGISTER_SCENEDATA_EXTRA_OBJECT_IO
 *   macro. A string identifies the data in the Studio scene to belong to that
 *   DzSceneData subclass.
 * - When Studio reads a scene file it finds the string identifiers and so it
 *   creates an instance of the scene subclasses. Our classes are 
 *   ReSceneBlock and ReStorage
 * - Studio then calls the ReStorage::startInstanceRead(DzAssetJsonItem* parentItem) 
 *   method expecting a subclass of DzAssetJsonObject in return. Our implementation
 *   is called ReSceneReader
 * - For every block of JSON data found, Studio calls the methods in 
 *   ReSceneReader. If a sub-object or an array are found, Studio calls
 *   the ReSceneReader::startMemberObject() or ReSceneReader::startMemberArray()
 *   and a new instance of ReSceneReader is created. Studio takes ownership of
 *   the new instances so they must not be deleted by the plugin. The only
 *   exception is the initial instance, the one created by ReStorage
 * - Every instance of ReSceneReader that deals with an object or array must 
 *   return true when the method ReSceneReader::isObject() or 
 *   ReSceneReader::isArray() are called by Studio while processing 
 *   objects or arrays.
 * - To Adjust where Reality writes the values read by Studio there is a
 *   class, \ref ReSceneContext, that keeps track of the current context. 
 *   The initial context is the root QVariantMap object that holds the 
 *   scene data. With any call to ReSceneReader::startMemberObject(), for 
 *   example, the ReSceneContext::startObject() is called. This switches
 *   the context to the new sub-object. Something similar happens for arrays.
 *
 */
 class ReSceneBlock : public DzSceneData {

  Q_OBJECT

public:

  //! Constructor: ReSceneBlock
  ReSceneBlock() : DzSceneData(RE_DS_SCENEDATA_ID, true) {
  };

  //! Destructor: ReSceneBlock
 ~ReSceneBlock() {
  };
};

/**
 * This class keeps track of the context for the loading of the JSON data
 * read from the Studio scene. The data is stored, as usual, inside a
 * QVariantMap. The initial context is the \ref sceneData variable.
 * As Studio reads sub-objects or arrays the context shifts to point to
 * the sub-object or array. The setValue() method is used by the user of 
 * this class to store values in the right context. 
 *
 * The ContextWrapper class is the actual implementation of the context
 * handler. 
 */
class ReSceneContext {

private:
  //! The container for all the scene data
  QVariantMap sceneData;

  /**
   * This class is how we take a snapshot of the context. Instances of this
   * class are pushed and popped to and from the stack to save and restore
   * the context for the data being read.
   * The class is really just a container for the pointer to the current 
   * element that receives the data from the stream, the optional identifier
   * for the current object, and the type of the object handled, although
   * the type is actually never used because the context provides the 
   * information. For now it's kept "just in case"
   */

  class ContextWrapper {

  public:
    enum ContextWrapperType {
      map, list
    };

    ContextWrapperType ptrType;
    QString id;

    union ContextPointers {
      QVariantMap* map;
      QVariantList* list;
    } ptr; 

    ContextWrapper() {
      ptr.map = NULL;
      ptrType = map;      
    }

    ContextWrapper( const ContextWrapper* ctx ) {
      ptr     = ctx->ptr;
      ptrType = ctx->ptrType;
      id      = ctx->id;
    }

    inline ContextWrapper& operator=( const ContextWrapper& ctx ) {
      ptr     = ctx.ptr;
      ptrType = ctx.ptrType;
      id      = ctx.id;
      return *this;
    }
  };  


  //! Pointer to the current receiver of data, when reading
  ContextWrapper currentContext;

  //! Stack of contextpointers for when we read nested objects
  QStack<ContextWrapper*> contextStack;

public:
  DzAssetFile& assetFile;
  
  //! Constructor: ReSceneBuffer  
  ReSceneContext(DzAssetFile& assetFile) : assetFile(assetFile)
  {
    currentContext.ptr.map = &sceneData;
  };

  //! Destructor: ReSceneBuffer
 ~ReSceneContext() {
  };
  
  inline void setValue( const QString& key, const QVariant& value ) {
    (*currentContext.ptr.map)[key] = value;
  }

  //! Array version
  inline void setValue( const QString& value ) {
    currentContext.ptr.list->append(value);
  }

  inline void setValue( const double value ) {
    currentContext.ptr.list->append(value);
  }

  inline void setValue( const bool value ) {
    currentContext.ptr.list->append(value);
  }

  inline QVariantMap& getData() {
    return sceneData;
  }

  //! This method needs to be called whenever a sub-object is found in 
  //! the stream. It does the following:
  //! - It creates a new sub-object
  //! - It saves the current context on the stack
  //! - It set the current context to point to the newly created object. 
  void startObject( const QString& id = "" ) {
    QVariantMap* qvmp = new QVariantMap();

    ContextWrapper* ctx = new ContextWrapper(currentContext);
    contextStack.push(ctx);

    currentContext.ptr.map = qvmp;
    currentContext.ptrType = ContextWrapper::map;
    currentContext.id = id;
  }

  void startList( const QString& id = "" ) {
    QVariantList* qvlp = new QVariantList();

    ContextWrapper* ctx = new ContextWrapper(currentContext);
    contextStack.push(ctx);

    currentContext.ptr.list = qvlp;
    currentContext.ptrType = ContextWrapper::list;
    currentContext.id = id;
  }

  void endObject() {
    ContextWrapper* ctx = contextStack.pop();
    // Connect the object just created to the parent
    if (currentContext.id == "") {
      ctx->ptr.list->append(QVariant(*currentContext.ptr.map));
    }
    else {
      (*ctx->ptr.map)[currentContext.id] = QVariant(*currentContext.ptr.map);
    }
    currentContext = ctx;
    delete ctx;
  }

  void endList() {
    ContextWrapper* ctx = contextStack.pop();
    // Connect the object just created to the parent
    if (currentContext.id == "") {
      ctx->ptr.list->append(QVariant(*currentContext.ptr.list));
    }
    else {
      (*ctx->ptr.map)[currentContext.id] = QVariant(*currentContext.ptr.list);
    }

    currentContext = ctx;
    delete ctx;
  }

  bool isInMap() {
    return currentContext.ptrType == ContextWrapper::map;
  }

  bool isInList() {
    return currentContext.ptrType == ContextWrapper::list;
  }
};

/**
 * This class implements the scene reading interface of the Studio API.
 * It specializes the JSON addMember and other methods to filter and store
 * the Reality data into the right place in a QVariantMap.
 * This object gets automatically deleted at the end of the reading of 
 * values from the file. Studio takes ownership of it.
 */
class ReSceneReader : public DzAssetJsonObject {

public:
  ReSceneContext* sceneContext;

  ReSceneReader( ReSceneContext* sceneContext )  
    : DzAssetJsonObject(sceneContext->assetFile)
    , sceneContext(sceneContext)
  {
    // Nothing
  }
  ~ReSceneReader() {
  }

  DzAssetJsonItem*  startObject();
  DzAssetJsonItem*  startArray();

  /*
   * Virtual methods re-implementation
   */
  bool addMember( const QString &name, const QString &val );
  bool addMember( const QString &name, double val );
  bool addMember( const QString &name, bool val );
  bool addItem( const QString &val );
  bool addItem( double val );
  bool addItem( bool val );

  DzAssetJsonItem* startMemberObject( const QString &name );
  DzAssetJsonItem* startMemberArray( const QString &name );

  void  finishObject( DzAssetJsonItem *item );
  void  finishArray( DzAssetJsonItem *item );

  bool isObject() const {
    return sceneContext->isInMap();
  }

  bool isArray() const {
    return sceneContext->isInList();
  }
};

class ReStorage : public DzExtraSceneDataIO {

  Q_OBJECT

private:
  //! The version of the data stored in scene created with this version of
  //! Reality.
  const float RE_DS_SCENE_DATA_VERSION;

  ReSceneContext* sceneContext;

public:
  //! Constructor: ReStorage
  ReStorage() : RE_DS_SCENE_DATA_VERSION(3.0), 
                   sceneContext(NULL) 
  {
  };

  //! Destructor: ReStorage
 ~ReStorage() {
    if (sceneContext) {
      delete sceneContext;
    }
  };

  // virtual methods from the base class
  DzSceneData* createDataItem( const DzFileIOSettings *opts ) const;

  //! Returns whether the scene needs to be written to disk
  bool shouldWrite( QObject* object, const DzFileIOSettings* opts ) const;

  //! This method is called by Studio when it needs to write the custom 
  //! data to the scene.
  //! \param object A pointer to the custom scene data item. This needs
  //!               to be cast to \ref ReSceneBlock
  //! \param io The IO object that we can use to write the data to the scene
  //! \param opt Undocumented 
  DzError writeExtraInstance( QObject* object, 
                              IDzJsonIO* io, 
                              const DzFileIOSettings* opts ) const;
  /**
   * The way Studio reload the data from the DSON file and provides it to 
   * custom classes is as follows:
   *   - Studio called the \ref startInstanceRead() method of this class
   *   - The parentItem parameters needs to be used to retrieve a reference
   *     to a DzAssetFile via the parentItem->getFile() method
   *   - startInstanceRead needs to return a DzAssetJsonObject that holds
   *     the reference to the file
   *   - Studio then calls the addMember() method of the returned DzAssetJsonObject
   *     once for each piece of custom data in the file. This is the loading
   *     phase and it can be used to store the data in a safe location
   *   - Studio then calls the applyInstanceToObject() method. This is where
   *     we can apply the data loaded, if needed
   *   - As a last step Studio then calls the resolveInstance() method. I'm 
   *     not sure why. It has to do with the way Studio organizes its own
   *     data structures. It's not needed for Reality.90
   */
  DzAssetJsonObject* startInstanceRead( DzAssetJsonItem* parentItem );
  DzError applyInstanceToObject( QObject* object, const DzFileIOSettings* opts ) const;
  DzError resolveInstance( QObject* object, const DzFileIOSettings* opts ) const;
};

} // namespace

#endif
