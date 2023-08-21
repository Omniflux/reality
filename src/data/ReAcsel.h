/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

/**
 * The ACSEL database contains the shader definitions for ACSEL.
 * Each shader in ACSEL is uniquely identified by a string, which is called a
 * UUID, or Universal Unique ID. The UUID is obtained by calculating a SHA1
 * digest using a string that includes the following elements:
 *
 * - The geometry file name of the figure. This identifies the figure uniquely,
 *   regardless of what lable it has in the scene. For example, V4 is uniquely
 *   identified as bMilWom_v4b
 * - The material name
 * - The series of image maps found in the original material. The image maps
 *   are obtained from the Diffuse and Alpha channnels. The bump map is not
 *   examined because the use of bump maps is not consistent between Poser
 *   and Studio versions of the same product/material
 *
 * The database includes several tables:
 *   - UUIDS: This table contains the UUIDs and the strings that generated them
 *   - ShaderSets: This table lists the properties of each set. 
 *   - Shaders: This table contains the shader definitions
 *
 * This is the structure of each table:
 *
 * ##UUIDS
 * 
 * - UUID: Text, Primary Key. The SHA1 digest obtained from the Schema, see below.
 * - Schema: Text. The combination of strings, separated by a "|" that 
 *           uniquely identifies a material.
 *
 * ##ShaderSets
 * This table is used as a catalog to show information about a set of shaders.
 * Often a material is part of a set. For example, a face material for V4
 * can be part of a set that defines how to render a specific "skin" for V4.
 * Shader sets group multiple materials into a coherent set that has a name,
 * a description and an icon.
 *
 * - SetID: INTEGER, Primary Key. This is automatically assigned as ROWID by
 *          SQLite
 * - SetName: TEXT
 * - Description: TEXT
 * - Author: TEXT
 * - CreationTime: Integer DEFAULT(CURRENT_TIMESTAMP)
 * - Thumbnail: NONE. This is a blob in PNG format
 *
 * ##Shaders
 * 
 * - UUID: TEXT, Primary Key
 * - ShaderCode: TEXT. The shder code is stored in JSON format.
 * - MaterialType: TEXT(3). This is a 3-character code with the following 
 *                 possible values: GLS (glass), GLO (glossy), MAT (matte), 
 *                 MTL (metal), MIR (mirror), MIX (mix), NUL (null), SKN (skin),
 *                 VLV (velvet), WTR (water), LGT (light), CLO (cloth),
 *                 HAR (hair)
 *                 N/A (not available)
 * - SetID: INTEGER. The SetID or 0 if there is not assigned ShaderSet
 * - IsOwner: BOOL. Indicates if the user owns this material
 * - Shared: BOOL. Indicates if the shader has been posted to the ACSEL Share
 *           service.
 */

#ifndef RE_ACSEL_H
#define RE_ACSEL_H

#include "ReMaterial.h"
#include "ReLogger.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include "ReLogger.h"

#include <QString>
#include <QStringList>

//! Major version number of the ACSEL database
#define RE_ACSEL_VERSION_MAJOR 5
//! Minor version number of the ACSEL database
#define RE_ACSEL_VERSION_MINOR 0

//! ACSEL version

#define RE_ACSEL_VERSION "5.0"

//! Version information abou thtis database. It helps in 
//! deciding if the database needs to be updated when a new version
//! of Reality is released
#define RE_ACSEL_TABLE_VERSION_INFO "VersionInfo"

//! Name of the table containing the shader definitions
#define RE_ACSEL_TABLE_SHADERS    "Shaders"
//! Name of the table holding the UUIDS and the strings that created them
#define RE_ACSEL_TABLE_UUIDS      "UUIDS"
//! Name of a temporary table of UUIDs. This table is emptied at the 
//! beginning of each session
#define RE_ACSEL_TABLE_TEMP_UUIDS "Temp_UUIDS"
//! Name of the table containign the shader sets descriptions
#define RE_ACSEL_TABLE_SETS       "Sets"

//! Name of the materials table. Materials are self-standing shaders that
//! can be applied to any pre-existing material in Reality. This is
//! deprecated and will need to be removed
#define RE_ACSEL_TABLE_MATERIALS  "Materials"

//! Name of the universal shaders table. UniShaders are self-standing 
//! shaders that can be applied to any pre-existing material in Reality. 
#define RE_ACSEL_TABLE_UNI_SHADERS  "UniversalShaders"

//! Name of the Categories table used to store the categories for 
//! the Universal shaders.
#define RE_ACSEL_TABLE_CATEGORIES  "Categories"

//! Name of the Volumes table
#define RE_ACSEL_TABLE_VOLUMES  "Volumes"

//! Name of the Object Alias table
#define RE_ACSEL_TABLE_ALIASES  "ObjectAliases"

//! Name of the Figures table. Used to identify what objects are actually
//! humanoid figures. Reality can use that information to set some defaults
//! in the automatic conversion of materials.
#define RE_ACSEL_TABLE_FIGURES  "Figures"

//! Temporary table used to store the original defintion of materials.
//! Such definitions are used for tasks like:
//! - Finding the original texture of materials that removed them, like
//!   when converting a material to glass and removing the diffuse texture.
//!   When we need to calculate the Acsel ID of such materials the basic
//!   data used for the calculation, the name of the image maps used for
//!   diffuse and alpha channel, are gone after the material has been 
//!   manipulated. Every material saves its original difition in this 
//!   table so that we can retrive the basic data without having to re-query
//!   the host application
//! - When the user needs to return a material to the original version, after
//!   having changed the material's type, we can restore the definition 
//!   just by querying this table. 
//!  
#define RE_ACSEL_TABLE_ORIGINAL_SHADERS "OriginalShaders"

//! Name of the ACSEL session
#define RE_ACSEL_CONNECTION_NAME  "ACSELDB4"
//! Name of the ACSEl database file on disk
#define RE_ACSEL_DB_FILE_NAME     "ACSEL.db"

//! Every bundle is recognized by this "signature"
#define RE_ACSEL_BUNDLE_SIGNATURE "Reality Shader Bundle"

//! A shader set is identified in the bundle by a signature of "A", as
//! in Automatic.
#define RE_ACSEL_SHADER_SET_SIGNATURE "A"

//! A Universal Shader is identified in the bundle by a signature of "U", as
//! in Universal.
#define RE_ACSEL_UNIVERSAL_SHADER_SIGNATURE "U"

//! A Default Shader Set is identified by this string in the SetName
#define RE_ACSEL_DEFAULT_SHADER_SET_SYMBOL "*"

// Keys for the ACSEL bundle
#define RE_ACSEL_BUNDLE_AUTHOR        "Author"
#define RE_ACSEL_BUNDLE_CATEGORY      "Category"
#define RE_ACSEL_BUNDLE_CREATION_TIME "CreationTime"
#define RE_ACSEL_BUNDLE_DATA          "Data"
#define RE_ACSEL_BUNDLE_DESCRIPTION   "Description"
#define RE_ACSEL_BUNDLE_ID            "ID"
#define RE_ACSEL_BUNDLE_IS_DEFAULT    "IsDefault"
#define RE_ACSEL_BUNDLE_IS_ENABLED    "IsEnabled"
#define RE_ACSEL_BUNDLE_IS_OWNER      "IsOwner"
#define RE_ACSEL_BUNDLE_MATERIAL_NAME "MaterialName"
#define RE_ACSEL_BUNDLE_MATERIAL_TYPE "MaterialType"
#define RE_ACSEL_BUNDLE_NAME          "Name"
#define RE_ACSEL_BUNDLE_OBJECT_ID     "ObjectID"
#define RE_ACSEL_BUNDLE_SET_ID        "SetID"
#define RE_ACSEL_BUNDLE_SET_NAME      "SetName"
#define RE_ACSEL_BUNDLE_SHADER_CODE   "ShaderCode"
#define RE_ACSEL_BUNDLE_SHADERS       "Shaders"
#define RE_ACSEL_BUNDLE_SHARED        "Shared"
#define RE_ACSEL_BUNDLE_SOURCE        "Source"
#define RE_ACSEL_BUNDLE_THUMBNAME     "Thumbnail"
#define RE_ACSEL_BUNDLE_TITLE         "Title"
#define RE_ACSEL_BUNDLE_TYPE          "Type"
#define RE_ACSEL_BUNDLE_UUID          "UUID"
#define RE_ACSEL_BUNDLE_UUIDS         "UUIDS"
#define RE_ACSEL_BUNDLE_VERSION       "Version"
#define RE_ACSEL_BUNDLE_VOLUME_ID     "VolumeID"
#define RE_ACSEL_BUNDLE_VOLUMES       "Volumes"

namespace Reality {

/**
 * Implementation of the ACSEL services
 */
class RE_LIB_ACCESS ReAcsel {
public:
    //! Return codes for the importBundle() method
  enum ReturnCode {
    ShaderSetAlreadyExists,
    SyntaxError,
    NotAnACSELBundle,
    WrongACSELVersion,
    CannotAddShaderSet,
    Success,
    GeneralError,
    // Error used when trying to import a set which has a different
    // author from the version that is already in the database
    UnAuthorizedUpdate 
  };

  //! Map used for the check of existing shader sets in the database
  typedef QMap<QString,QString> ExistingShaderSets;

private:
  bool dbOpen;

  //! In certain situations methods that start a transaction need to
  //! call other method that use transactions as well. Since nested
  //! transactions are not allowed in SQLite, we use a variable here
  //! to signal that a transaction is already in execution
  bool transactionStarted;

  //! Useful typedef to define a smart pointer to a transaction
  typedef QSharedPointer<SQLite::Transaction> TransactionPtr;

  //! Last error message
  static QString lastErrorMsg;

  //! Singleton instance
  static ReAcsel* instance;

  //! A Pointer to the ACSEL database. Shared among all the 
  //! functions accessing the DB.
  SQLite::Database* db;

  typedef QHash<ReMaterialType, QString> LutMaterialTypes;
  static LutMaterialTypes lutMaterialTypes;

  const QString getDatabaseFileName() const;

  //! Utility method to simplify running non-SELECT queries
  bool runQuery( const QString& sql );

  //! Method used to intialize the LUT, just once
  void initLut();

  //! Constructor. Made private to enforce singleton protocol.
  explicit ReAcsel();

  //! This struct exists only to create a static instance of itself which
  //! will automatically delete the ReAcsel::instance instance at program
  //! end.
  struct cleanup {
    ~cleanup() {
      if (ReAcsel::instance) {
        delete ReAcsel::instance;
      }
    }
  };

  //! Static instance used to do the cleanup automatically
  static cleanup _cleanup;

  //! RAII struct to support SQLite transactions with nested calls.
  //! SQLite does not support nested transaction so a nested method
  //! that starts a transaction when the caller already started a 
  //! transaction, will fail with an exception.
  //! The following struct, and the startTransaction() method work
  //! around the problem. 
  //! 
  //! Methods can call startTransaction() to start a transaction whenever
  //! it's possible. A "root-level" call will start the transaction.
  //! A nested call will simply create an instance of AcselTransaction
  //! without doing anything. 
  //! This is achieved by using the ReAcsel::transactionStarted bool
  //! variable.
  //! The struct implements the RAII template
  //! so that it rools back the transaction is case the instance goes
  //! out of scope before the commit() method is called.
  struct AcselTransaction {
    SQLite::Transaction* t;

    bool& transFlag;
    AcselTransaction( SQLite::Database* db, bool& transFlag ) : 
      t(NULL),
      transFlag(transFlag) 
    { 
      // If we are not in the middle of a transaction then we start one
      if (!transFlag) {
        transFlag = true;
        RE_LOG_INFO() << "+ LOCK";
        t = new SQLite::Transaction(*db);
      }
    }

    ~AcselTransaction() {
      // If the t pointer is NULL then we didn't start the transaction
      // therefore it's not our task to change the flag in any way
      try {
        if (t != NULL) {
          transFlag = false;
          RE_LOG_INFO() << "~ ROLLBACK";
          delete t;
        }
      }
      catch( std::exception e ) {
        RE_LOG_WARN() << e.what();
      }
    }

    void commit() {
      if (t != NULL) {
        t->commit();
        RE_LOG_INFO() << "# COMMIT";
        delete t;
        t = NULL;
        transFlag = false;
      }
    }
  };

  typedef QSharedPointer<AcselTransaction> AcselTransactionPtr;

  //! When updating the database during the material conversion we need
  //! to start the transaction system or Reality will slow down considerably.
  //! To do this we need to create a transaction that persists across all
  //! calls to ReAcsel methods. The following variable does that.
  AcselTransactionPtr globalTransaction;

  AcselTransactionPtr startTransaction() {
    return AcselTransactionPtr( new AcselTransaction(db, transactionStarted) );
  };

  //! Returns an string with the app code  to be used in the Temp_UUIDS
  //! table
  const QString getAppCode() const;

  //! Method used by importBundle() to verify if the shader sets included
  //! in the bundle already exist in the ACSEL database.
  ReturnCode checkIfShaderSetsExist( const QVariantMap& bundle, 
                                     ExistingShaderSets& existingShaderSets );

  //! Checks the version of the database against the version handled by
  //! the program. If the database is older it converts it to the new 
  //! version.
  void checkDatabaseVersion();

  //! Updates the database to the new version, if necessary
  void updateDatabase( const float dbVer, const float libVer );

  //! Tests if a table exists and if it doesn't it executes the 
  //! SQL command in the sql parameter, which is expected to be
  //! a "CREATE TABLE" statement
  //! \return true if the table has been created, false otherwise
  //! This method throws an exception in case of error while executing
  //! the SQL statement
  bool createTableIfMissing( const QString& tableName, 
                             const QString& sql,
                             const QString& postCommands = "");

  //! Helper method for importBundle() that handles the importing of
  //! automatic shaders.
  ReturnCode importShaderSet( QVariantMap& shaderInfo );
  
  //! Helper method for importBundle() that handles the importing of
  //! universla shaders.
  ReturnCode importUniversalShader( QVariantMap& shaderInfo );

  //! Used to handle nested calls to the caching system. When this
  //! variable reaches zero the caching is disabled
  int cachingLevels;

public:

  //! Access method to retrieve the instance. It creates an instance if it
  //! does not exist already. This is the singleton implementation.
  static ReAcsel* getInstance();

  inline QString getErrorMessage() {
    return lastErrorMsg;
  }
  
  //! Destructor: ReAcsel
  ~ReAcsel() {
    // Avoid deleting the instance more than once
    ReAcsel::instance = NULL;
    if (db) {
      delete db;
    }
  };

  //! Starts the transactioning system so that all writes are cached
  inline void startCaching() {
    if (globalTransaction.isNull()) {
      // RE_LOG_INFO() << "(+) Started global ACSEL transaction";
      globalTransaction = startTransaction();
    }
    cachingLevels++;
  }

  //! Close the global transaction and commit the writes to the database
  inline void stopCaching() {
    // RE_LOG_INFO() << "(-) Committed global ACSEL transaction";
    cachingLevels--;
    if (!globalTransaction.isNull() && (cachingLevels == 0)) {
      globalTransaction->commit();
      globalTransaction.clear();
    }
  }

  //! Rolls-back all the transactions. This is generally not used but
  //! it's been implemented for possible future uses.
  inline void deleteGlobalTransaction() {
    globalTransaction.clear();
  }

  //! Returns the unique identifier to connect an ACSEL shader with a 
  //! given material
  QString getAcselID( const QString geometryFileName, 
                      const QString matID, 
                      const QStringList& acselTextures );

  //! Starts the database connection and creates the tables, if necessary
  void initDB();

  //! Given a string it returns the corresponding material type
  ReMaterialType getMaterialType( const QString str );

  //! Saves a shader set to the database.
  //! \param setID Input/Output. If the method is called to save an 
  //!                 existing shader set then this parameter is set to 
  //!                 the ID of that set. If this is a new shader set
  //!                 then this parameter must be an empty string.
  //!                 On function return the parameter will be set to the
  //!                 ID of the shader set or to an empty string in case
  //!                 of error.
  //!
  //! \param name The name of the shader set.
  //! \param description The description for the shader set.
  //! \param author The name of the author.
  //! \param geometryID The ID of the geometry, usually the file name, 
  //!                   that holds the materials of this set.
  //! \param isDefault True if the shader set is a default/generic set.
  //! \param isEnabled True if the shader set is a enabled.
  //!
  //! \return true in case of success, false otherwise.
  bool saveShaderSet( QString& setID,
                      const QString& name, 
                      const QString& description,
                      const QString& author,
                      const QString& geometryID,
                      const bool isDefault = false,
                      const bool isEnabled = true );

  //! Updates a shader set with the data from a QVariantMap.
  //! Expected data:
  //! - Description
  //! - IsDefault
  //! - IsEnabled
  void updateSetProperties( const QString& setID, const QVariantMap& data);

  //! Flag a shader set as enabled or disabled.
  void setShaderSetEnabled( const QString& setID, bool isEnabled );

  //! Saves a shader to the database
  //! \param ID The UUID of the shader
  //! \param shaderCode The JSON-formatted string that defines the shader
  //! \param matType The type of the material. The type is converted to a 
  //!                three-character string
  //! \param materialName The name of the material as reported by the object
  //!                     that owns it
  //! \param setID The ID of the shader set that is associated with the shader
  //! \param volumeID The ID of a volume or 0 if the material does not have 
  //!                 an associated volume
  //! \return Trye if the operation is successful, false otherwise
  bool saveShader( const QString& ID, 
                   const QString& shaderCode,
                   const ReMaterialType matType,
                   const QString& materialName,
                   const QString& setID,
                   const int volumeID = 0 );

  //! Deletes a shader from the database 
  bool deleteShader( const QString& shaderID );

  //! Saves a volume and returns the ID of the volume.
  //! \param volumeID The ID of the volume. If it's 0 then the volume is
  //!                 saved as a new one. If greater the zero then the 
  //!                 volume code is updated with the code passed to this
  //!                 function.
  //! \param setID The ID of the shader set associated with the volume
  //! \param volumeCode The code, in JSON format that defines the volume
  //! \return An integer identifying the volume within the shader set
  //!         the ID starts from 1 for each shader set
  int saveVolume( const int volumeID, 
                  const QString& setID, 
                  const QString& volumeCode );

  //! Returns the volume ID used by a set of materials
  //! \param setID The ID of the shader set that contains the materials
  //! \param matNames A list of material names that are queried. All 
  //!                 materials are expected top be using the same volume
  //! \return The ID of the volume or zero if no volume is used by the
  //!         materials listed.
  int getVolumeID( const QString& setID, const QStringList& matNames );

  //! Search the database for a matching volume. If found then the code
  //! for the volume is copied in the volCode parameter
  //! \param setID The shader set ID to search
  //! \param volID The ID of the volume to find
  //! \param volCode This is an output parameter. If the volume is found
  //!                then the code for it is copied here.
  //! \return True if the volume is found, false otherwise.
  bool findVolume( const QString& setID, const int volID, QString& volCode );

  /**
   * Searches the database for a shader. If the shader is found it is 
   * returned as a map. If it's not found the string will be empty
   * 
   * \param shaderID The UUID of the shader to be found
   * \param shaderData Output variable. On return it will contain the
   *                   shader data if the shader was found. Otherwise 
   *                   if will be an empty map
   * \return A boolean indicating if the shader was found or not
   */
  bool findShader( const QString& shaderID, QVariantMap& data );

  /**
   * Finds a shader based on the shader set id and the material name.
   * \param shaderSet The numeric ID of the shader set
   * \param materialName The name of the material to find
   * \param shaderCode If the shader is found this variable will contain
   *                   the code of the shader. Otherwise it will be null.
   * \return true if the shader is found, false otherwise.
   */
  bool findShader( const QString& shaderSetID, 
                   const QString& materialName,
                   QString& shaderCode,
                   ReMaterialType& matType );

  //! Finds a shader set based on the set ID or Set name
  //! If the shader is found the method returns true and sets the
  //! QVariantMap to contain a copy of the shader set's data.
  //! \param setID The setID or Set name to use for the search
  //! \param setData Output. The data for the shader set, if it is found
  //! \param useName If true then the search will use the set name instead
  //!                of the setID
  bool findShaderSet( const QString& setID, 
                      QVariantMap& setData, 
                      bool useName = false );

  //! Finds if there is a default/generic shader set for the given
  //! object.
  //! \param objectID The ID of the object for which we are searching
  //! \param data This output parameter gets filled with the data from
  //!             the shader, if found.
  //! \return true if the shader set is found, false otherwise
  bool findDefaultShaderSet( const QString& objectID, QVariantMap& data );

  //! Returns the full list of shader sets
  //! \param records A QVariantList provided by the caller that will be
  //!        filled with records describing the shader sets. Each record
  //!        is a QVariantList itself, with the following order:
  //!            - Set name
  //!            - Set description
  //!            - Set creation time
  void getShaderSetList( QList<QStringList>& records );

  //! Given a shader set ID this methid returns a brief list of information
  //! about the shaders containers in the set.
  //! \return A list of QStringList. Each list entry describes a shader in 
  //!         the set. Each subentry, a QStringList, lists the shader's UUID
  //!         ,the material name and the material's type
  void getShaderSetShaderInfo( const QString& shaderSetID, 
                               QList<QStringList>& info );

  //! Stores a UUID into a temporary table. UUIDS are saved to the
  //! permanent table only if the shader is saved. The temporary
  //! UUIDs are deleted at the beginning of the Reality session.
  bool storeUUID( const QString& UUID, const QString& schema );

  //! Empty the temp tables. This is meant to be used at the end of 
  //! each session
  void eraseTempData();

  //! Deletes a shader set. This action affects the Sets, Shaders and
  //! UUIDs tables. The shader set is completely erased from the database
  //! 
  //! \param setID The ID of the shader set to be deleted
  //! \returns True if the operation was successful, false otherwise.
  bool deleteShaderSet( const QString& setID );

  //! Deletes a universal shader. 
  //! 
  //! \param shaderID The ID of the shader to be deleted
  //! \returns True if the operation was successful, false otherwise.
  bool deleteUniversalShader( const QString& shaderID );


  //! Stores a shader in the temporary table RE_ACSEL_TABLE_ORIGINAL_MATS
  //! \returns true if the operation was successful, false otherwise.
  bool storeOriginalShader( const QString& objID, 
                            const QString& matID,
                            const QString& shaderCode );

  //! Retrieves the data of an original shader previously stored.
  //! \param objID The unique identifier of the object
  //! \param matID The unique identifier of the material
  //! \param shaderData A map that will be filled with the data from the 
  //!                   shader
  //! \return true if the operation was successful, false otherwise
  bool getOriginalShader( const QString objID, 
                          const QString matID,
                          QVariantMap& shaderData );

  //! Overloaded version that returns the raw text data from the database
  bool getOriginalShader( const QString objID, 
                          const QString matID,
                          QString& shaderData );

  /**
   * Exports a series of shader sets and universal shader as a bundle. 
   * The bundle is the format used to distribute Reality shaders so that 
   * they can be installed into the database.
   *
   * \param sets. A list of shader sets that need to be exported. It can
   *              be empty
   * \param universalShaders. A list of universal shader that need to be 
   *                          exported. It can be empty.
   * \param fileName. The name of the JSON file that will contain the
   *        exported bundle
   *
   * Format of the ACSEL bundle, the way to distribute ACSEL shaders to be 
   * installed by Reality via command line.
   *
   * - The storage format is JSON
   * - The tables exported are: 
   *     - Sets
   *     - Shaders
   *     - UUIDS
   * - Each bundle is prefixed by some identifying information followed
   *   by an array or elements. Each element is the collection of records
   *   from the ACSEL database that form the shader set exported.
   *
   * This is how a bundle looks:
   *
   *  {
   *    "id" : "ACSEL Bundle",
   *    "title": "Title of this bundle",
   *    "version": "4.0",
   *    "source": "Pret-a-3D",
   *    "data": [
   *      {
   *        "description": {
   *           "SetName": "...",
   *           "Description": "...",
   *           "Author": "...",
   *           "CreationTime": "YYYY/MM/DD HH:MM:SS"
   *        },
   *        "shaders": [
   *           {
   *             "UUID": "...",
   *             "ShaderCode": "...",
   *             "MaterialType": "...",
   *             "MaterialName": "...",
   *           },
   *           ...
   *        ],
   *        "volumes": {
   *            "<id[0]>": "<VolumeCode>" 
   *           ...
   *           "<id[n]>": "<VolumeCode>" 
   *        },
   *        "uuids": {
   *             <UUID[0]>: <Schema 0>,
   *             ...
   *             <UUID[n]>: <Schema n>,
   *        }
   *      },
   *      ...
   *      {
   *        < last shader set>
   *      }
   *    ]
   *  }
   *
   * \return A string with the result of the operation. A blank string
   *         means no error, otherwise an error message is provided.
   */
  QString exportToBundle( const QStringList& sets, 
                          const QStringList& universalShaders,
                          const QString& title, 
                          const QString& sourceName,
                          const QString& fileName );

  //! Exports to whole database to a bundle.
  //! \param bundleFileName The name of the file that will contain the bundle.
  //! \return A string with the result of the operation. A blank string
  //!         means no error, otherwise an error message is provided.
  QString exportDbToBundle( const QString& bundleFileName );

  /**
   * Imports a bundle into the database
   * \param bundleFileName The name of the file that contrains the bundle
   * \param overwrite Flag that signals if Reality should overwrite a 
   *                  shader set if it already exists in the ACSEL database
   * \param existingShaderSets If the \ref overwrite flag is false and one
   *                           or more shader sets included in the bundle are
   *                           found to already exist it the ACSEL database, 
   *                           then this list will be returned with an entry
   *                           for each shader set found in the database.
   *                           The map is keyed by shader set name and the
   *                           associated value is the description for the set
   *                           as it was found in the database.
   * \return A code that signals the result of the operation. For example, if
   *         a bundle includes pre-existing shader sets and the overwrite flag
   *         is false then the function will return ShaderSetAlreadyExists
   */
  ReturnCode importBundle( 
    const QString bundleFileName,
    const bool overwrite,
    ExistingShaderSets& existingShaderSets
  );

  //! Exports a shader set as a QVariantMap string
  //! \param shaderSet The ID of the shader set to export
  //! \return The QVariantMap object describing the shader set in the 
  //!         format described in exportToBundle()
  QVariantMap exportShaderSet( const QString& shaderSet );

  //! Exports a universal shader as a JSON string
  //! \param shaderID The ID of the universal shader set to export
  //! \return The QVariantMap object describing the shader set in the 
  //!         format described in exportToBundle()
  QVariantMap exportUniversalShader( const QString& shaderID );

  //! Imports a series of object aliases into the ObjectAliases table.
  //! \param aliases A map keyed by object ID where each element is an alias
  void importObjectAliases( const QVariantMap& aliases );
  
  //! Overridden version that accepts a file name. The file is expected to
  //! contain a json object listing the aliases.
  void importObjectAliases( const QString& aliasFileName );

  //! Checks if a string is the name of a known figure and returns a 
  //! boolean indicating the finding.
  bool isFigure( const QString figName );

  //! Exports all the record from the Figures table to the QVariantMap passed
  //! in the parameter for this function.
  //! \return true if successful, false in case of error.
  bool getFigures( QVariantMap& figures );

  //! Saves a universal shader into the database. The shader is saved by
  //! creating a UUID for it by this method. The UUID is then written
  //! to the shaderID parameter so that it can be referenced by the 
  //! caller as needed.
  //!
  //! \param shaderID Input/Output parameter that is set to the unique ID of the
  //!                 shader. If the parameter is blank then it is assumed
  //!                 that the shader is new and the ID will be created
  //!                 automatically by this method. If it's not blank then
  //!                 it contains the ID of an existing shader that will be
  //!                 updated with the data passed in the other parameters.
  //! \param shaderName The descriptive name of the shader
  //! \param desc The description of the shader
  //! \param shaderCode The code of the shader
  //! \param author The name of the author of this shader
  //! \param blob Alternative to the previewFileName parameter, it provides
  //!             the content, in PNG format, of the thumbnail.
  //! \param previewFileName Optional. If passed it contains the name of
  //!                        a png file that holds the preview for the 
  //!                        shader
  //! \return true if the operation is successful, false otherwise.
  bool saveUniversalShader( QString& shaderID,
                            const QString& shaderName, 
                            const QString& desc,
                            const QString& shaderCode,
                            const int category,
                            const QString& author,
                            const QByteArray& blob );

  bool saveUniversalShader( QString& shaderID,
                            const QString& shaderName, 
                            const QString& desc,
                            const QString& shaderCode,
                            const int category,
                            const QString& author,
                            const QString& previewFileName = "" );

  //! Overloaded version that allows to use a string for the category
  bool saveUniversalShader( QString& shaderID,
                            const QString& shaderName, 
                            const QString& desc,
                            const QString& shaderCode,
                            const QString& category,
                            const QString& author,
                            const QString& previewFileName = "" );

  bool findUniversalShader( const QString& shaderID, QVariantMap& data );

  //! Retrieves the list of all Universal Shaders
  //! If a category is specified then the list will be restricted to the
  //! shaders of that category.
  void getUniversalShaderList( QList<QStringList>& records, 
                               const QString& category = 0 );

  void getUniversalShaderPreview( const QString& shaderID,
                                  unsigned char*& bitmapData,
                                  int& blobSize );

  //! Returns the list of all the categories available for 
  void getCategoryList( QStringList& list );
};

typedef QSharedPointer<ReAcsel> ReAcselPtr;
} // namespace


#endif
