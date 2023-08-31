/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReAcsel.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QJson/Parser>
#include <QJson/Serializer>

#include "RealityBase.h"


#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER
namespace SQLite
{
    void assertion_failed(const char* apFile, const long apLine, const char* apFunc, const char* apExpr, const char* apMsg)
    {
        RE_LOG_WARN() << QString(
            "Assertion failed in SQLite: %1@%2 -- %3"
        )
            .arg(apFile)
            .arg(apLine)
            .arg(apMsg)
            .toStdString();
    }
}
#endif

using namespace Reality;

// Creates a properly formatted UUID
QString createUUID() {
  QString str = QUuid::createUuid().toString();
  // Remove the beginning and ending curlies
  str.remove(0,1);
  str.remove(str.length()-1,1);
  str.replace("-","");
  return str;
}


// Contructor
ReAcsel::ReAcsel() {
  dbOpen = false;
  db = NULL;
  transactionStarted = false;
  cachingLevels = 0;
  // Initialize the LUT
  initLut();
  // Initialize the database
  initDB();
  // Check if the database is up to date
  checkDatabaseVersion();
} 

// Initializers for static data
ReAcsel::LutMaterialTypes ReAcsel::lutMaterialTypes;
QString ReAcsel::lastErrorMsg;
ReAcsel* ReAcsel::instance = NULL;
ReAcsel::cleanup ReAcsel::_cleanup;

ReAcsel* ReAcsel::getInstance() {
  if (!ReAcsel::instance) {
    ReAcsel::instance = new ReAcsel();
  }
  return instance;
}

void ReAcsel::initLut() {
  lutMaterialTypes[MatGlass]  = "GLS";
  lutMaterialTypes[MatGlossy] = "GLO";
  lutMaterialTypes[MatMatte]  = "MAT";
  lutMaterialTypes[MatMetal]  = "MTL";
  lutMaterialTypes[MatMirror] = "MIR";
  lutMaterialTypes[MatMix]    = "MIX";
  lutMaterialTypes[MatNull]   = "NUL";
  lutMaterialTypes[MatSkin]   = "SKN";
  lutMaterialTypes[MatVelvet] = "VLV";
  lutMaterialTypes[MatWater]  = "WTR";
  lutMaterialTypes[MatLight]  = "LGT";
  lutMaterialTypes[MatCloth]  = "CLO";  
}

ReMaterialType ReAcsel::getMaterialType( const QString str ) {
  // No need to get smart. A single sequential scan will work fine. There
  // are just a handful of material types.
  QHashIterator<ReMaterialType, QString> i(lutMaterialTypes);
  while( i.hasNext() ) {
    i.next();
    if (i.value() == str) {
      return i.key();
    }
  }
  return MatGlossy;
}

QString ReAcsel::getAcselID( const QString geometryFileName, 
                             const QString matID, 
                             const QStringList& acselTextures ) {
  char separator = '|';
  QString objectID = geometryFileName;
  // Find the object alias, if present
  QString qry = QString("SELECT * from %1 WHERE ObjectID=?")
                  .arg(RE_ACSEL_TABLE_ALIASES);
  SQLite::Statement s( *db, qry.toUtf8() );
  s.bind(1, static_cast<const char*>(geometryFileName.toUtf8()));  
  if (s.executeStep()) {
    objectID = s.getColumn(1).getText();
  }

  QString acselSequence = objectID + separator + matID;
  int count = acselTextures.count();
  for (int i = 0; i < count; i++) {
    QFileInfo finfo(acselTextures.value(i));
    acselSequence += separator + finfo.fileName();
  }

  QString UUID = QString(
    QCryptographicHash::hash(
      acselSequence.toLower().toAscii(), 
      QCryptographicHash::Sha1
    ).toHex()
  );
  storeUUID(UUID, acselSequence);

  #ifndef NDEBUG 
  RE_LOG_INFO() << "Acsel ID: " << matID << " - " << acselSequence 
                << " - " << UUID;
  #endif

  return UUID;
}

bool ReAcsel::getOriginalShader( const QString objID, 
                                 const QString matID,
                                 QString& shaderCode ) 
{
  if (!dbOpen) {
    return false;
  }
  // Retrieve the shader code for the original state of the material,
  // before any change applied by the user of ACSEL
  QString q = QString(
                "SELECT ShaderCode FROM %1 WHERE App=? AND ObjectID=? AND"
                " MatID=?"
              )
              .arg(RE_ACSEL_TABLE_ORIGINAL_SHADERS);
  SQLite::Statement s( *db, q.toUtf8());
  s.bind(1, static_cast<const char*>(getAppCode().toUtf8()));
  s.bind(2, static_cast<const char*>(objID.toUtf8()));
  s.bind(3, static_cast<const char*>(matID.toUtf8()));
  
  if (s.executeStep()) {
    shaderCode = s.getColumn(0).getText();
  }
  return true;
}

bool ReAcsel::getOriginalShader( const QString objID, 
                                 const QString matID,
                                 QVariantMap& shaderData ) 
{
  QString shaderCode;
  if ( !getOriginalShader(objID, matID, shaderCode) ) {
    return false;
  }
  //! Convert the JSON text to a QVariantMap
  QJson::Parser jsonParser;
  bool ok = false;
  shaderData = jsonParser.parse(shaderCode.toUtf8(), &ok).toMap();
  if (!ok) {
    return false;
  }
  return true;
}


const QString ReAcsel::getDatabaseFileName() const {
  QString dbDirName = QString("%1/Pret-a-3D/Reality")
                        .arg(
                           QDesktopServices::storageLocation(
                             QDesktopServices::DocumentsLocation
                           )
                         );
  QDir dbDir(dbDirName);
  if (!dbDir.exists()) {
    QDir().mkpath(dbDirName);
  }

  return QString("%1/%2").arg(dbDir.absolutePath()).arg(RE_ACSEL_DB_FILE_NAME);
}

bool ReAcsel::runQuery( const QString& sql ) {
  if (!dbOpen) {
    return false;
  }
  try {
    if (!db) {
      return false;
    }
    SQLite::Statement q(*db, sql.toUtf8());
    try {
      q.exec();
    }
    catch( SQLite::Exception e ) {
      lastErrorMsg = QString(
                       QObject::tr("Error in Reality ACSEL SQL: %1\n%2")
                     )
                     .arg(q.getErrorMsg())
                     .arg(sql);
      RE_LOG_INFO() << lastErrorMsg.toStdString();
      return false;
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << "SQLite exception in runQuery(): " << e.what();
  }
  return true;
}

bool ReAcsel::createTableIfMissing( const QString& tableName, 
                                    const QString& sql,
                                    const QString& postCommands ) {

  if (!db->tableExists(tableName.toAscii())) {
    RE_LOG_INFO() << QString("Table %1 does not exist").arg(tableName).toStdString();
    if ( runQuery( sql ) ) {
      RE_LOG_INFO() << QString("Table %1 created").arg(tableName).toStdString();
      if (!postCommands.isEmpty()) {
        // The \n is used to separate multiple commands in the string
        // because SQLite doesn't support multiple commands at the moment
        // of writing this. 
        // TransactionPtr t;
        AcselTransactionPtr t;
        if (!transactionStarted) {
          t = startTransaction();
        }
        auto cmds = postCommands.split("\n");
        for (int i = 0; i < cmds.count(); i++) {
          runQuery(cmds[i]);
        }
        if (!t.isNull()) {
          t->commit();
        }
      }
      return true;
    }
    else {
      throw std::runtime_error(
              QString("Error creating table %1")
                .arg(tableName).toAscii().data()
            );
    }
  }
  return false;
}

void ReAcsel::initDB() {
  if (!dbOpen) {
    // Open the connection
    try {
      db = new SQLite::Database(
                 getDatabaseFileName().toUtf8(),
                 SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
               );
    }
    catch( SQLite::Exception ) {
      lastErrorMsg = "Could not connect or open the ACSEL database";
      RE_LOG_INFO() << lastErrorMsg.toStdString();
      return;
    }
    dbOpen = true;
  }
  // Check if the tables exist. If not then create them
  RE_LOG_INFO() << "Testing if the ACSEL tables exist";

  try {
    /*
     * Version Info
     */
    QString str = QString(
                    "CREATE TABLE %1 (Major INTEGER, Minor INTEGER)"
                  )
                  .arg(RE_ACSEL_TABLE_VERSION_INFO);
    QString postCommands = QString("INSERT INTO %1 (Major, Minor) VALUES(%2,%3)")
                             .arg(RE_ACSEL_TABLE_VERSION_INFO)
                             .arg(RE_ACSEL_VERSION_MAJOR)
                             .arg(RE_ACSEL_VERSION_MINOR);

    createTableIfMissing(RE_ACSEL_TABLE_VERSION_INFO, str, postCommands);

    /*
     * Shaders sets. The Flags field will be a single-byte bitmask with the
     * the following structure:
     *   - bit 1: whether the shader set is a commercial preset. This prevents
     *            exporting it from the Preset Manager.
     *   - bits 2-8: reserverd for future use.
     */ 
    str = QString( "CREATE TABLE %1"
                   "(SetID TEXT, SetName TEXT, Description TEXT,"
                   " ObjectID TEXT, Author TEXT, CreationTime TEXT"
                   " DEFAULT(CURRENT_TIMESTAMP),"
                   " IsOwner BOOL, Shared BOOL, IsDefault BOOL, "
                   " IsEnabled BOOL DEFAULT 1, Thumbnail NONE,"
                   " Flags SMALLINT DEFAULT 0,"
                   " PRIMARY KEY(SetID))" )
          .arg(RE_ACSEL_TABLE_SETS);
    postCommands = QString(
                     "CREATE INDEX SetNames on %1 (SetName)\n"
                     "CREATE INDEX DefaultShaders on %1 (ObjectID,IsDefault)"
                   ).arg(RE_ACSEL_TABLE_SETS);

    createTableIfMissing(RE_ACSEL_TABLE_SETS, str, postCommands);
    /*
     * Shaders
     */ 
    str = QString(
            "CREATE TABLE %1"
            " (UUID TEXT, ShaderCode TEXT, MaterialType TEXT,"
            " SetID TEXT, MaterialName TEXT, VolumeID INTEGER DEFAULT 0,"
            " PRIMARY KEY(UUID))"
          )
          .arg(RE_ACSEL_TABLE_SHADERS);
    postCommands = QString(
                     "CREATE INDEX MaterialName ON %1 (MaterialName)"
                   ).arg(RE_ACSEL_TABLE_SHADERS);

    createTableIfMissing(RE_ACSEL_TABLE_SHADERS, str, postCommands);
    /*
     * Volumes
     */ 
    str = QString( 
            "CREATE TABLE %1"
            " (SetID TEXT, VolumeID INTEGER, VolumeCode TEXT,"
            " PRIMARY KEY(SetID, VolumeID))"
          )
          .arg(RE_ACSEL_TABLE_VOLUMES);
    createTableIfMissing(RE_ACSEL_TABLE_VOLUMES, str);

    /*
     * UUID
     */ 
    str = QString(
            "CREATE TABLE %1 (UUID TEXT, Schema TEXT, PRIMARY KEY(UUID))"
          )
          .arg(RE_ACSEL_TABLE_UUIDS);
    createTableIfMissing(RE_ACSEL_TABLE_UUIDS, str);

    /*
     * Figures
     */ 
    str = QString(
            "CREATE TABLE %1 (FigureName TEXT PRIMARY KEY,"
            " Description TEXT)"
          )
          .arg(RE_ACSEL_TABLE_FIGURES);
    createTableIfMissing(RE_ACSEL_TABLE_FIGURES, str);

    /*
     * Universal shaders categories
     */ 
    str = QString(
            "CREATE TABLE %1"
            " (CategoryID INTEGER, Name TEXT, PRIMARY KEY(CategoryID))"
          )
          .arg(RE_ACSEL_TABLE_CATEGORIES);
    postCommands = QString(
      "CREATE INDEX CategoryName ON %1 (Name)\n"
      "INSERT INTO %1 (Name) VALUES(\"%2\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%3\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%4\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%5\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%6\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%7\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%8\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%9\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%10\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%11\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%12\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%13\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%14\")\n"
      "INSERT INTO %1 (Name) VALUES(\"%15\")"
    )
    .arg(RE_ACSEL_TABLE_CATEGORIES)
    .arg(QObject::tr("Ceramic"))
    .arg(QObject::tr("Gemstone"))
    .arg(QObject::tr("Glass"))
    .arg(QObject::tr("Leather"))
    .arg(QObject::tr("Liquid"))
    .arg(QObject::tr("Metal"))
    .arg(QObject::tr("Plastic"))
    .arg(QObject::tr("Skin"))
    .arg(QObject::tr("Stone"))
    .arg(QObject::tr("Fabric"))
    .arg(QObject::tr("Vegetation"))
    .arg(QObject::tr("Wood"))
    .arg(QObject::tr("Other"))
    .arg(QObject::tr("Car paint"));

    createTableIfMissing(RE_ACSEL_TABLE_CATEGORIES, str, postCommands);

    /*
     * Universal shaders
     */ 
    str = QString(
            "CREATE TABLE %1"
            " (ID Text, Name TEXT, Description TEXT, ShaderCode TEXT,"
            "  Category INTEGER, CreationTime DEFAULT(CURRENT_TIMESTAMP),"
            "  Author TEXT, Thumbnail BLOB, PRIMARY KEY(ID))"
          )
          .arg(RE_ACSEL_TABLE_UNI_SHADERS);
    postCommands = QString(
                     "CREATE INDEX UniNames on %1 (Name)\n"
                     "CREATE INDEX UniDescriptions on %1 (Description)\n"
                     "CREATE INDEX UniCategories on %1 (Category)"
                   ).arg(RE_ACSEL_TABLE_UNI_SHADERS);

    createTableIfMissing(RE_ACSEL_TABLE_UNI_SHADERS, str, postCommands);

    /*
     * Temporary UUIDs
     */
    str = QString(
            "CREATE TABLE %1"
            " (APP TEXT, UUID TEXT, Schema TEXT, PRIMARY KEY(APP,UUID))"
          )
          .arg(RE_ACSEL_TABLE_TEMP_UUIDS);
    createTableIfMissing(RE_ACSEL_TABLE_TEMP_UUIDS, str);

    /*
     * Original material definitions
     */
    str = QString(
            "CREATE TABLE %1"
            " (App TEXT, ObjectID TEXT, MatID TEXT, ShaderCode TEXT,"
            " PRIMARY KEY(App,ObjectID,MatID))"
          )
          .arg(RE_ACSEL_TABLE_ORIGINAL_SHADERS);
    createTableIfMissing(RE_ACSEL_TABLE_ORIGINAL_SHADERS, str);

    /*
     * Object aliases
     */ 
    str = QString(
            "CREATE TABLE %1 (ObjectID TEXT PRIMARY KEY, Alias TEXT)"
          )
          .arg(RE_ACSEL_TABLE_ALIASES);
    postCommands = "";
    createTableIfMissing(RE_ACSEL_TABLE_ALIASES, str);

  }
  catch( std::runtime_error e ) {
    RE_LOG_WARN() << e.what();
  }
}

void ReAcsel::checkDatabaseVersion() {
  if (!dbOpen) {
    return;
  }

  SQLite::Statement q( *db, QString("SELECT * from %1")
                             .arg(RE_ACSEL_TABLE_VERSION_INFO).toUtf8());
  if (q.executeStep()) {
    float currentVer = RE_ACSEL_VERSION_MAJOR + RE_ACSEL_VERSION_MINOR/10.0;
    float dbVer = q.getColumn(0).getInt() + (q.getColumn(1).getInt() / 10.0);
    RE_LOG_INFO() << "ACSEL database is at version " << dbVer
                  << ", current db engine version is " << currentVer;
    if (dbVer < currentVer) {
      // Make sure that there are no pending SQL queries of the 
      // database file will be locked and we will not be able to update it
      q.reset();
      RE_LOG_INFO() << "ACSEL Database needs updating";
      updateDatabase(dbVer, currentVer);
      RE_LOG_INFO() << "  Database updated.";
    }
  }
}

void ReAcsel::updateDatabase( const float dbVer, const float libVer ) {
  // Begin transaction
  AcselTransactionPtr transaction = startTransaction();
  try {
    // Update the database to fix some issues that were found in
    // Reality 4.0.5
    if (dbVer < 4.5) {
      RE_LOG_INFO() << "Upgrade ACSEL database to v.4.5";
      // Remove two wrong shaders for V4's in the Charlie/Neve set
      runQuery("DELETE from Shaders where UUID='d083bf3ce59f4a67f1bd9c35c9e1abe10fc19bb0'"
               " and SetID='ce25530d8632433593ccdc19988e9cae'"
              );
      runQuery("DELETE from Shaders where UUID='e110c2df99f7fc5d95ca61463f268d91ec52fb82'"
               " and SetID='ce25530d8632433593ccdc19988e9cae'"
              );
      // Remove a wrong shader for V4's in the Rio's set
      runQuery("DELETE from Shaders where UUID='0175427163756a12dff4a9790fe13bac782807a0'"
               " and SetID='be78c53985794e4aa543c0ce87f14b41'"
              );
    } 
    // Remove a shader that prevents using the generic shader for the tear
    // material of Genesis 2
    if (dbVer < libVer) {
      RE_LOG_INFO() << "Upgrade ACSEL database to v." << libVer;
      runQuery("DELETE from Shaders where UUID='c3161a623f3c224e8dfc93b48c87e34f9c529c03'"
               " and SetID='c49d0e855f95458a9ca7a2c8182baccf'"
              );      
    }

    if (dbVer < 4.7) {
      // Check if the Car Paint category is missing. In the past this test 
      // was not done, which resulted in having two Car paint categories.
      SQLite::Statement s(*db, "SELECT count(CategoryID) from Categories where Name='Car paint'");
      if (s.executeStep()) {
        if (s.getColumn(0).getInt() == 0) {
          QString carPaintCat = QString("INSERT INTO %1 (Name) VALUES(\"%2\")")
                                  .arg(RE_ACSEL_TABLE_CATEGORIES)
                                  .arg(QObject::tr("Car paint"));
          runQuery(carPaintCat);
        }
      }
    }
    // Reality 4.3
    if (dbVer < 5.0) {
      // Check if there is a duplicate Car paint category.
      SQLite::Statement s(*db, "SELECT count(CategoryID) from Categories where Name='Car paint'");
      if (s.executeStep()) {
        if (s.getColumn(0).getInt() > 1) {
          runQuery("DELETE from Categories WHERE CategoryID=15");
        }
      }
      // Upgrade the Presets table to have the Flags colum
      s.reset();
      SQLite::Statement us(*db,QString("ALTER TABLE Sets ADD 'Flags' SMALLINT DEFAULT 0").toUtf8());
      if ( us.exec() ) {
        // Now search if the customer has bought the commercial sets that were
        // delivered before the flags field was implemented and if so, set 
        // the flag value for them.
        
        // Austrani Outfit and all the associated items
        runQuery("UPDATE Sets set Flags=1 WHERE SetName like 'Austrani%'");
        // V7 and genitalia
        runQuery("UPDATE Sets set Flags=1 WHERE SetID='24941b47151b43c8a3c8c9cafdde6623' "
                 "or SetID='c407cf6afb9d4c259394beb92400748d'");
      }
    }
    // Update the database version number
    if ( !runQuery( QString("UPDATE %1 set Major=%2, Minor=%3")
                      .arg(RE_ACSEL_TABLE_VERSION_INFO)
                      .arg(RE_ACSEL_VERSION_MAJOR)
                      .arg(RE_ACSEL_VERSION_MINOR) ) ) {
      return;
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
    // transactionStarted = false;
    return;
  }
  transaction->commit();
}


const QString ReAcsel::getAppCode() const {
  auto appId = RealityBase::getRealityBase()->getHostAppID();
  switch(appId) {
    case DAZStudio:
      return "ds";
    case Poser:
      return "ps";
    case Blender:
      return "bl";
    case ZBrush:
      return "zb";
    case Modo:
      return "md";
    case RealityPro:
      return "re";
    default:
      return "n/a";
  }
}

bool ReAcsel::saveShader( const QString& ID, 
                          const QString& shaderCode,
                          const ReMaterialType matType,
                          const QString& materialName,
                          const QString& setID,
                          const int volumeID ) 
{
  if (!dbOpen) {
    return false;
  }
  bool success = false;
  try {
    // Save the shader
    QString qry = QString(
                    "INSERT OR REPLACE into %1 (UUID, ShaderCode, MaterialType,"
                    " MaterialName,SetID, VolumeID)"
                    " VALUES(:id, :shaderCode, :matType, :matName, :setID,"
                    " :volID)"
                  )
                  .arg(RE_ACSEL_TABLE_SHADERS);

    SQLite::Statement q( *db, qry.toUtf8());
    q.bind(":id", static_cast<const char*>(ID.toAscii()));
    q.bind(":shaderCode", static_cast<const char*>(shaderCode.toUtf8()));
    q.bind(":matType", static_cast<const char*>(lutMaterialTypes[matType].toAscii()));
    q.bind(":matName", static_cast<const char*>(materialName.toAscii()));
    q.bind(":setID", static_cast<const char*>(setID.toAscii()));
    q.bind(":volID", volumeID);
    success = q.exec();
    // Move the UUID schema from the temp table to the permanent one
    if (success) {
      SQLite::Statement q( *db, 
                           QString(
                             "SELECT Schema from %1 WHERE APP='%2' AND UUID='%3'"
                           )
                           .arg(RE_ACSEL_TABLE_TEMP_UUIDS)
                           .arg(getAppCode())
                           .arg(ID)                         
                           .toUtf8()
                         );
      if (!q.executeStep()) {
        success = false;
      }
      else {
        // Save the UUID for this material
        QString schema = QString(q.getColumn(0));
        runQuery(
          QString(
            "INSERT OR IGNORE INTO %1 (UUID, Schema) VALUES('%2','%3')"
          )
          .arg(RE_ACSEL_TABLE_UUIDS)
          .arg(ID)
          .arg(schema)
        );
      }
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
    success = false;
  }
  return success;
}

bool ReAcsel::deleteShader( const QString& shaderID ) {
  if (!dbOpen) {
    return false;
  }
  bool success = false;
  try {
    QVariantMap data;
    findShader(shaderID, data);

    //SQLite::Transaction t(*db);
    AcselTransactionPtr t = startTransaction();
    QString qry = QString("DELETE FROM %1 WHERE UUID=:shaderID")
                    .arg(RE_ACSEL_TABLE_SHADERS);
    SQLite::Statement s(*db, qry.toUtf8());
    s.bind(":shaderID", shaderID.toUtf8().constData());
    success = s.exec();
    if (!success) {
      return success;
    }
    qry = QString("DELETE FROM %1 WHERE UUID=:shaderID")
            .arg(RE_ACSEL_TABLE_UUIDS);
    SQLite::Statement s2(*db, qry.toUtf8());
    s2.bind(":shaderID", shaderID.toUtf8().constData());
    success = s2.exec();
    if (!success) {
      return success;
    }
    t->commit();
    // Now check if there are any shaders left in the set. If not, then
    // we need to remove the set as well
    QString setID = data[RE_ACSEL_BUNDLE_SET_ID].toString();
    // List that will hold the information about the shader set
    QList<QStringList> shaders;
    getShaderSetShaderInfo( setID, shaders );
    if (!shaders.count()) {
      // No shaders are left in the set, so we delete the whole set
      deleteShaderSet(setID);
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
    success = false;
  }
  return success;
}

int ReAcsel::getVolumeID( const QString& setID, 
                          const QStringList& matNames ) 
{
  if (!dbOpen) {
    return 0;
  }
  try {
    QString matList;
    int numMats = matNames.count();
    if (numMats == 0) {
      return 0;
    }
    for (int i = 0; i < numMats; i++) {
      matList += QString("'%1'").arg(matNames[i]);
      if (i < numMats-1) {
        matList += ",";
      }
    }
    QString qry= QString( 
                   "SELECT max(VolumeID) from %1 WHERE SetID=:setID AND"
                   " MaterialName IN (%2) LIMIT 1"
                 )
                 .arg(RE_ACSEL_TABLE_SHADERS)
                 .arg(matList);
    SQLite::Statement q( *db, qry.toUtf8());
    q.bind(":setID", setID.toStdString());

    if (!q.executeStep()) {
      return 0;
    }
    return q.getColumn(0).getInt();
  }
  catch( SQLite::Exception e )  {
    RE_LOG_WARN() << e.what();
  }
  return 0;
}

int ReAcsel::saveVolume( const int volumeID, 
                         const QString& setID, 
                         const QString& volumeCode ) 
{
  try {
    // This is a new volume
    if (volumeID == 0) {
      QString qry = QString("SELECT MAX(VolumeID) FROM %1 WHERE SetID=:setID")
                      .arg(RE_ACSEL_TABLE_VOLUMES);
      SQLite::Statement q(*db, qry.toUtf8());
      q.bind(":setID", setID.toStdString());
      if (!q.executeStep()) {
        return 0;
      }
      int newID = q.getColumn(0).getInt()+1;
      qry = QString("INSERT INTO %1 (SetID, VolumeID, VolumeCode)"
                    " values(:setID, :volumeID, :volumeCode)")
              .arg(RE_ACSEL_TABLE_VOLUMES);

      SQLite::Statement iq(*db, qry.toUtf8());
      iq.bind(":setID", static_cast<const char*>(setID.toAscii()));
      iq.bind(":volumeID", newID);
      iq.bind(":volumeCode", static_cast<const char*>(volumeCode.toUtf8()));
      try {
        if (!iq.exec()) {
          return 0;
        }
      }
      catch( SQLite::Exception e ) {
        RE_LOG_WARN() << "SQL Error: " << e.what();
        return 0;
      }
      return newID;
    }
    // Save the new code of a pre-existing volume
    QString qry = QString( 
                    "INSERT OR REPLACE INTO %1 (SetID,VolumeID,VolumeCode)"
                    " VALUES(?,?,?)"
                  )
                  .arg(RE_ACSEL_TABLE_VOLUMES);
    SQLite::Statement us( *db , qry.toUtf8());
    us.bind(1, static_cast<const char*>(setID.toAscii()));
    us.bind(2, volumeID);
    us.bind(3, static_cast<const char *>(volumeCode.toUtf8()));
    if( !us.exec() ) {
      RE_LOG_WARN() << "SQL error: " << db->getErrorMsg();
      return 0;
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
  }
  return volumeID;
}

bool ReAcsel::findVolume( const QString& setID, const int volID, QString& volCode ) {
  if (!dbOpen) {
    return false;
  }
  // Just a precaution...
  volCode = "";
  SQLite::Statement q( 
    *db, 
    QString("SELECT VolumeCode FROM %1 WHERE SetId=? AND VolumeID=?")
      .arg(RE_ACSEL_TABLE_VOLUMES)
      .toUtf8()
  );
  q.bind(1, setID.toStdString());
  q.bind(2, volID);
  if ( !q.executeStep() ) {
    return false;
  }
  volCode = QString(q.getColumn(0).getText());
  return true;
}

QString& escapeText( QString& text ) {
  // Replaces a single quote with two single quotes, as required by SQL
  return text.replace("'", "''");
}

bool ReAcsel::saveShaderSet( QString& setID,
                             const QString& name, 
                             const QString& description,
                             const QString& author,
                             const QString& geometryID,
                             const bool isDefault,
                             const bool isEnabled ) 
{
  if (!dbOpen) {
    return 0;
  }
  QString qry;
  bool isNew;
  if (setID.isEmpty()) {
    setID = createUUID();
    isNew = true;
  }
  else {
    // This test covers the case when we import a set from a bundle
    qry = QString("SELECT SetID from %1 WHERE SetID=:id")
            .arg(RE_ACSEL_TABLE_SETS); 
    SQLite::Statement s( *db, qry.toUtf8());
    s.bind(":id", setID.toUtf8().constData());
    isNew = !s.executeStep();
  }

  bool result = true;
  try {
    if ( isNew ) {
      qry = QString("INSERT into %1 (SetID, SetName, Description, Author,"
                    " ObjectID, IsDefault, IsOwner, Shared, IsEnabled)"
                    " VALUES(:id, :setName, :desc, :author, :objID,"
                    " :isDefault, 1, 0, :isEnabled)")
              .arg(RE_ACSEL_TABLE_SETS);
      SQLite::Statement s( *db, qry.toUtf8());
      s.bind(":id", static_cast<const char*>(setID.toAscii()));
      s.bind(":setName", static_cast<const char*>(name.toUtf8()));
      s.bind(":desc", static_cast<const char*>(description.toUtf8()));
      s.bind(":author", static_cast<const char*>(author.toUtf8()));
      s.bind(":objID", static_cast<const char*>(geometryID.toAscii()));
      s.bind(":isDefault", isDefault ? 1 : 0);
      s.bind(":isEnabled", isEnabled ? 1 : 0);
      result = s.exec();
    }
    else {
      qry = QString("UPDATE %1 set SetName=:setName, Description=:desc"
                    " WHERE SetID=:setID")
              .arg(RE_ACSEL_TABLE_SETS);
      SQLite::Statement s( *db, qry.toUtf8());
      s.bind(":setID", static_cast<const char*>(setID.toAscii()));
      s.bind(":setName", static_cast<const char*>(name.toUtf8()));
      s.bind(":desc", static_cast<const char*>(description.toUtf8()));
      result = s.exec();    
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
    result = false;
  }
  return result;
}

void ReAcsel::updateSetProperties( const QString& setID, const QVariantMap& data) {
  if (!dbOpen) {
    return;
  }
  QStringList subQry;
  if (data.contains(RE_ACSEL_BUNDLE_SET_NAME)) {
    subQry << "SetName=:setName";
  }
  if (data.contains(RE_ACSEL_BUNDLE_DESCRIPTION)) {
    subQry << "Description=:desc";
  }
  if (data.contains(RE_ACSEL_BUNDLE_IS_DEFAULT)) {
    subQry << "IsDefault=:isDefault";
  }
  if (data.contains(RE_ACSEL_BUNDLE_IS_ENABLED)) {
    subQry << "IsEnabled=:isEnabled";
  }
  QString qry = QString("UPDATE %1 set %2 WHERE SetID=:setID")
                  .arg(RE_ACSEL_TABLE_SETS)
                  .arg(subQry.join(","))
                  .arg(setID);
  SQLite::Statement q(*db, qry.toUtf8());

  q.bind(":setID", static_cast<const char*>(setID.toAscii()));
  if (data.contains(RE_ACSEL_BUNDLE_SET_NAME)) {
    q.bind(":setName", data[RE_ACSEL_BUNDLE_SET_NAME].toString().toUtf8().constData());
  }
  if (data.contains(RE_ACSEL_BUNDLE_DESCRIPTION)) {
    q.bind(":desc", data[RE_ACSEL_BUNDLE_DESCRIPTION].toString().toUtf8().constData());
  }
  if (data.contains(RE_ACSEL_BUNDLE_IS_DEFAULT)) {
    q.bind(":isDefault", data[RE_ACSEL_BUNDLE_IS_DEFAULT].toBool() ? 1 : 0);
  }
  if (data.contains(RE_ACSEL_BUNDLE_IS_ENABLED)) {
    q.bind(":isEnabled", data[RE_ACSEL_BUNDLE_IS_ENABLED].toBool() ? 1 : 0);
  }
  if (!q.exec()) {
    RE_LOG_WARN() << "Error in SQL: " << db->getErrorMsg();
  }
}


bool ReAcsel::storeUUID( const QString& UUID, const QString& schema ) {
  if (!dbOpen) {
    return false;
  }
  return runQuery( 
           QString(
            "INSERT OR REPLACE INTO %1 (APP, UUID, Schema)"
            " VALUES('%2', '%3', '%4')"
           )
           .arg(RE_ACSEL_TABLE_TEMP_UUIDS)
           .arg(getAppCode())
           .arg(UUID)
           .arg(schema)
         );
}

bool ReAcsel::findShader( const QString& shaderID, QVariantMap& data ) {
  if (!dbOpen) {
    return false;
  }
  data["ShaderID"]     = "";
  data[RE_ACSEL_BUNDLE_SET_ID]        = "";
  data[RE_ACSEL_BUNDLE_SHADER_CODE]   = "";
  data[RE_ACSEL_BUNDLE_SET_NAME]      = "";
  data[RE_ACSEL_BUNDLE_MATERIAL_TYPE] = "";
  data[RE_ACSEL_BUNDLE_VOLUME_ID]     = 0;
  bool found = false;
  QString query = QString(
                    "SELECT %1.ShaderCode,%1.SetID,%2.SetName,%1.MaterialType,"
                    " %1.VolumeID from %1,%2"
                    " WHERE %1.UUID='%3' AND %1.SetID=%2.SetID"
                    " AND %2.IsEnabled=1"
                  )
                  .arg(RE_ACSEL_TABLE_SHADERS)
                  .arg(RE_ACSEL_TABLE_SETS)
                  .arg(shaderID);

  SQLite::Statement q(*db, query.toUtf8());
  if (q.executeStep()) {
    data["ShaderID"]     = shaderID;
    data[RE_ACSEL_BUNDLE_SET_ID]        = QString(q.getColumn(1));
    data[RE_ACSEL_BUNDLE_SHADER_CODE]   = QString(q.getColumn(0));
    data[RE_ACSEL_BUNDLE_SET_NAME]      = QString(q.getColumn(2));
    data[RE_ACSEL_BUNDLE_MATERIAL_TYPE] = getMaterialType(q.getColumn(3).getText());
    data[RE_ACSEL_BUNDLE_VOLUME_ID]     = QVariant(q.getColumn(4).getInt());
    found = true;
  }
  return found;
}

bool ReAcsel::findShader( const QString& shaderSetID, 
                          const QString& materialName,
                          QString& shaderCode,
                          ReMaterialType& matType ) {
  if (!dbOpen) {
    return false;
  }
  shaderCode = "";

  QString query = QString(
                    "SELECT ShaderCode,MaterialType from %1"
                    " WHERE SetID=:setID AND MaterialName=:matName"
                  )
                  .arg(RE_ACSEL_TABLE_SHADERS);

  SQLite::Statement q(*db, query.toUtf8());
  q.bind(":setID", shaderSetID.toUtf8().constData());
  q.bind(":matName", materialName.toUtf8().constData());
  if (q.executeStep()) {
    shaderCode = QString(q.getColumn(0));
    matType = getMaterialType(q.getColumn(1).getText());
    return true;
  }
  return false;
}


bool ReAcsel::findShaderSet( const QString& setID, 
                             QVariantMap& setData,
                             bool useName ) 
{
  if (!dbOpen) {
    return false;
  }
  QString qry;
  if (!useName) {
    qry = QString(
            "SELECT SetName,Description,ObjectID,Author,CreationTime,"
            " IsOwner,Shared,IsDefault from %1 WHERE SetID=:setID"
          )
          .arg(RE_ACSEL_TABLE_SETS);
  }
  else {
    // Use the set name
    qry = QString(
            "SELECT SetName,Description,ObjectID,Author,CreationTime,"
            " IsOwner,Shared,IsDefault from %1 "
            " WHERE lower(SetName)=:setName"
          )
          .arg(RE_ACSEL_TABLE_SETS);
  } 
  SQLite::Statement q(*db, qry.toUtf8());
  if (!useName) {
    q.bind(":setID", setID.toUtf8().constData());
  }
  else {
    q.bind(":setName", setID.toLower().toUtf8().constData());
  }

  if (q.executeStep()) {
    setData[RE_ACSEL_BUNDLE_NAME]          = QString(q.getColumn(0));
    setData[RE_ACSEL_BUNDLE_DESCRIPTION]   = QString(q.getColumn(1));
    setData[RE_ACSEL_BUNDLE_OBJECT_ID]     = QString(q.getColumn(2));
    setData[RE_ACSEL_BUNDLE_AUTHOR]        = QString(q.getColumn(3));
    setData[RE_ACSEL_BUNDLE_CREATION_TIME] = QString(q.getColumn(4));
    setData[RE_ACSEL_BUNDLE_IS_OWNER]      = QVariant(
                                               static_cast<bool>(
                                                 q.getColumn(5).getInt()
                                               )
                                             );
    setData[RE_ACSEL_BUNDLE_SHARED]        = QVariant(
                                               static_cast<bool>(
                                                 q.getColumn(6).getInt()
                                               )
                                             );
    setData[RE_ACSEL_BUNDLE_IS_DEFAULT]    = QVariant(
                                               static_cast<bool>(
                                                 q.getColumn(7).getInt()
                                               )
                                             );
    return true;
  }
  return false;
}

bool ReAcsel::findDefaultShaderSet( const QString& objectID, QVariantMap& data ) {
  if (!dbOpen) {
    return false;
  }
  QString qry = QString(
                  "SELECT SetID,SetName,Description,ObjectID,Author,"
                  " CreationTime,IsOwner,Shared,IsDefault from %1"
                  " WHERE lower(objectID)='%2' AND IsDefault=1"
                )
                .arg(RE_ACSEL_TABLE_SETS)
                .arg(objectID.toLower());
  SQLite::Statement q(*db, qry.toUtf8());

  if (q.executeStep()) {
    data[RE_ACSEL_BUNDLE_SET_ID]        = QString(q.getColumn(0));
    data[RE_ACSEL_BUNDLE_SET_NAME]      = QString(q.getColumn(1));
    data[RE_ACSEL_BUNDLE_DESCRIPTION]   = QString(q.getColumn(2));
    data[RE_ACSEL_BUNDLE_OBJECT_ID]     = QString(q.getColumn(3));
    data[RE_ACSEL_BUNDLE_AUTHOR]        = QString(q.getColumn(4));
    data[RE_ACSEL_BUNDLE_CREATION_TIME] = QString(q.getColumn(5));
    data[RE_ACSEL_BUNDLE_IS_OWNER]      = QVariant(
                                            static_cast<bool>(
                                              q.getColumn(6).getInt()
                                            )
                                          );
    data[RE_ACSEL_BUNDLE_SHARED]        = QVariant(
                                            static_cast<bool>(
                                              q.getColumn(7).getInt()
                                            )
                                          );
    data[RE_ACSEL_BUNDLE_IS_DEFAULT]    = QVariant(
                                            static_cast<bool>(
                                              q.getColumn(8).getInt()
                                            )
                                          );
    return true;
  }
  return false;
}


void ReAcsel::eraseTempData() {
  if (!dbOpen) {
    return;
  }
  QString appCode = getAppCode();
  SQLite::Statement q(*db, 
                      QString("SELECT count(UUID) from %1 WHERE APP='%2'")
                        .arg(RE_ACSEL_TABLE_TEMP_UUIDS)
                        .arg(getAppCode())
                        .toUtf8());
  try {
    if ( q.executeStep() ) {
      int numRows = q.getColumn(0);
      q.reset();
      if (numRows) {
        runQuery(
          QString(
            "DELETE FROM %1 WHERE APP='%2'"
          )
          .arg(RE_ACSEL_TABLE_TEMP_UUIDS)
          .arg(appCode)
        );
      }
    }
    // Erase the data in the OriginalMats table
    runQuery(
      QString("DELETE FROM %1 where App='%2'")
        .arg(RE_ACSEL_TABLE_ORIGINAL_SHADERS)
        .arg(appCode)
    );
  }
  catch( ... ) {
    RE_LOG_INFO() << "Exception in eraseTempUUIDs()";
  }
}

bool ReAcsel::deleteShaderSet( const QString& setID ) {
  if (!dbOpen) {
    return false;
  }
  try {
    // Begin transaction
    // SQLite::Transaction transaction(*db);
    AcselTransactionPtr transaction = startTransaction();

    // Delete the shader set information from the Sets table
    QString qry = QString("DELETE FROM %1 WHERE SetID=:setID")
                  .arg(RE_ACSEL_TABLE_SETS);
    SQLite::Statement q(*db, qry.toUtf8());
    q.bind(":setID", static_cast<const char*>(setID.toAscii()));
    if (!q.exec()) {
      return false;
    }
    // Collect all the UUIDs associated with the shader set
    qry = QString("SELECT UUID FROM %1 where SetID=:setID")
          .arg(RE_ACSEL_TABLE_SHADERS);
    SQLite::Statement q2(*db, qry.toUtf8());
    q2.bind(":setID", static_cast<const char*>(setID.toAscii()));

    QStringList uuids;
    while( q2.executeStep() ) {
      uuids << static_cast<const char*>(q2.getColumn(0));
    }
    // Delete all the shaders in the set
    qry = QString("DELETE FROM %1 where SetID=:setID")
          .arg(RE_ACSEL_TABLE_SHADERS);
    SQLite::Statement q3(*db, qry.toUtf8());
    q3.bind(":setID", static_cast<const char*>(setID.toAscii()));
    q3.exec();

    // Delete the connected volumes
    qry = QString("DELETE FROM %1 WHERE SetID=:setID")
            .arg(RE_ACSEL_TABLE_VOLUMES);
    SQLite::Statement vs(*db, qry.toUtf8());
    vs.bind(":setID", setID.toUtf8().constData());
    vs.exec();
    
    // Remove all the UUIDs in the set
    qry = QString("DELETE FROM %1 where UUID =:uuid")
          .arg(RE_ACSEL_TABLE_UUIDS);
    SQLite::Statement q4(*db, qry.toUtf8());
    for (int i = 0; i < uuids.count(); i++) {
      q4.bind(":uuid", uuids[i].toStdString());
      q4.exec();
      q4.reset();
    }
    transaction->commit();
  }
  catch( SQLite::Exception e ) {
    RE_LOG_INFO() << "SQLite error in deleteShaderSet(): " 
                  << e.what();
  }
  return true;
}

bool ReAcsel::deleteUniversalShader( const QString& shaderID ) {
  if (!dbOpen) {
    return false;
  }

  QString qry = QString("DELETE FROM %1 WHERE ID=:id")
                  .arg(RE_ACSEL_TABLE_UNI_SHADERS);
  SQLite::Statement s( *db, qry.toUtf8());
  s.bind(":id", shaderID.toStdString());
  return s.exec();
}

void ReAcsel::getShaderSetList( QList<QStringList>& records ) {
  if (!dbOpen) {
    return;
  }
  QString qry = QString(
                  "SELECT SetID,SetName,Description,CreationTime,IsDefault,"
                  "ObjectID,IsEnabled,Author from %1 ORDER BY SetName"
                )
                .arg(RE_ACSEL_TABLE_SETS);
  SQLite::Statement q(*db, qry.toUtf8());

  while (q.executeStep()) {
    QStringList* r = new QStringList();
    // Set ID
    r->append( static_cast<const char*>(q.getColumn(0)) ) ;
    // Set Name
    r->append( static_cast<const char*>(q.getColumn(1)) ) ;
    // Description
    r->append( static_cast<const char*>(q.getColumn(2)) ) ;
    // Creation time
    r->append( static_cast<const char*>(q.getColumn(3)) ) ;
    // Is default?
    r->append( q.getColumn(4).getInt() ? "y" : "n" ) ;
    // ObjectID
    r->append( static_cast<const char*>(q.getColumn(5)) );
    // Is enabled?
    r->append( q.getColumn(6).getInt() ? "y" : "n" ) ;
    // Author
    r->append( static_cast<const char*>(q.getColumn(7)) );
    records.append(*r);
  }
}

void ReAcsel::getShaderSetShaderInfo( const QString& shaderSetID, 
                                      QList<QStringList>& info ) {

  info.clear();
  if (!dbOpen) {
    return;
  }
  // Obtain the shaders for this set
  QString qry = QString(
                  "SELECT UUID,MaterialName,MaterialType FROM %1 "
                  " WHERE SetID=:setID"
                )
                .arg(RE_ACSEL_TABLE_SHADERS);

  SQLite::Statement q(*db, qry.toUtf8());
  q.bind(":setID", static_cast<const char*>(shaderSetID.toAscii()));
  while( q.executeStep() ) {
    QStringList* rec = new QStringList();
    // UUID
    rec->append( static_cast<const char*>(q.getColumn(0)) );
    // Material name
    rec->append( static_cast<const char*>(q.getColumn(1)) );
    // Material type
    rec->append( static_cast<const char*>(q.getColumn(2)) );
    info.append(*rec);
  }
}

QVariantMap ReAcsel::exportShaderSet( const QString& shaderSetID ) {
  QVariantMap shaderSet, 
              desc;
  if (!dbOpen) {
    return shaderSet;
  }

  // Get the shader set description
  QString qry = QString(
                  "SELECT SetName,Description,Author,ObjectID,CreationTime,"
                  " IsDefault,IsEnabled"
                  " FROM %1 WHERE SetID=:setID"
                )
                .arg(RE_ACSEL_TABLE_SETS);

  try {
    SQLite::Statement q(*db, qry.toUtf8());
    q.bind(":setID", static_cast<const char*>(shaderSetID.toAscii()));

    if (q.executeStep()) {
      desc[RE_ACSEL_BUNDLE_TYPE]          = RE_ACSEL_SHADER_SET_SIGNATURE;
      desc[RE_ACSEL_BUNDLE_SET_ID]        = shaderSetID;
      desc[RE_ACSEL_BUNDLE_SET_NAME]      = static_cast<const char*>(q.getColumn(0));
      desc[RE_ACSEL_BUNDLE_DESCRIPTION]   = static_cast<const char*>(q.getColumn(1));
      desc[RE_ACSEL_BUNDLE_AUTHOR]        = static_cast<const char*>(q.getColumn(2));
      desc[RE_ACSEL_BUNDLE_OBJECT_ID]     = static_cast<const char*>(q.getColumn(3));
      desc[RE_ACSEL_BUNDLE_CREATION_TIME] = static_cast<const char*>(q.getColumn(4));
      desc[RE_ACSEL_BUNDLE_IS_DEFAULT]    = q.getColumn(5).getInt();
      desc[RE_ACSEL_BUNDLE_IS_ENABLED]    = q.getColumn(6).getInt();
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_INFO() << "Error in ACSEL export: " << e.what();
    return shaderSet;
  }
  // Obtain the shaders for this set
  QVariantList shaders;
  QVariantMap uuids;
  qry = QString(
          "SELECT s.UUID,s.ShaderCode,s.MaterialType,s.MaterialName,"
          " s.VolumeID,u.Schema FROM %1 as s, %2 as U"
          " WHERE SetID=:setID and s.UUID = u.UUID"
        )
        .arg(RE_ACSEL_TABLE_SHADERS)
        .arg(RE_ACSEL_TABLE_UUIDS);

  SQLite::Statement q2(*db, qry.toUtf8());
  q2.bind(":setID", static_cast<const char*>(shaderSetID.toAscii()));

  while( q2.executeStep() ) {
    QVariantMap* rec = new QVariantMap();
    QString UUID = static_cast<const char*>(q2.getColumn(0)) ;
    rec->insert(RE_ACSEL_BUNDLE_UUID, UUID);
    rec->insert(RE_ACSEL_BUNDLE_SHADER_CODE, 
                static_cast<const char*>(q2.getColumn(1)) );
    rec->insert(RE_ACSEL_BUNDLE_MATERIAL_TYPE, 
                static_cast<const char*>(q2.getColumn(2)) );
    rec->insert(RE_ACSEL_BUNDLE_MATERIAL_NAME, 
                static_cast<const char*>(q2.getColumn(3)) );
    rec->insert(RE_ACSEL_BUNDLE_VOLUME_ID, 
                static_cast<const char*>(q2.getColumn(4)) );
    shaders.append(*rec);
    uuids[UUID] = static_cast<const char*>(q2.getColumn(5));
  }

  // Export the associated volumes
  QVariantMap volumes;
  qry = QString("SELECT VolumeID, VolumeCode from %1 WHERE SetID=:setID")
          .arg(RE_ACSEL_TABLE_VOLUMES);

  SQLite::Statement q3(*db, qry.toUtf8());
  q3.bind(":setID", static_cast<const char*>(shaderSetID.toAscii()));
  while( q3.executeStep() ) {
    volumes[q3.getColumn(0).getText()] = q3.getColumn(1).getText();
  }

  shaderSet[RE_ACSEL_BUNDLE_DESCRIPTION] = desc;
  shaderSet[RE_ACSEL_BUNDLE_SHADERS]     = shaders;
  shaderSet[RE_ACSEL_BUNDLE_VOLUMES]     = volumes;
  shaderSet[RE_ACSEL_BUNDLE_UUIDS]       = uuids;

  return shaderSet;
}

// Converts a blob into a base64 string
QString blobToText( const void* blob, int blobSize ) {
  QByteArray a = QByteArray::fromRawData((char*)blob, blobSize);
  return QString(a.toBase64());
}

QVariantMap ReAcsel::exportUniversalShader( const QString& shaderID ) {
  QVariantMap shader,
              desc;
  if (!dbOpen) {
    return shader;
  }

  // Get the shader data
  QString qry = QString("SELECT Name, Description, ShaderCode,"
                        " Category, CreationTime, Author, Thumbnail"
                        " FROM %1 WHERE ID=:ID")
                  .arg(RE_ACSEL_TABLE_UNI_SHADERS);

  try {
    SQLite::Statement q(*db, qry.toUtf8());
    q.bind(":ID", shaderID.toStdString());

    if (q.executeStep()) {
      desc[RE_ACSEL_BUNDLE_TYPE]          = RE_ACSEL_UNIVERSAL_SHADER_SIGNATURE;
      desc[RE_ACSEL_BUNDLE_ID]            = shaderID;
      desc[RE_ACSEL_BUNDLE_NAME]          = q.getColumn(0).getText();
      desc[RE_ACSEL_BUNDLE_DESCRIPTION]   = q.getColumn(1).getText();
      desc[RE_ACSEL_BUNDLE_CATEGORY]      = q.getColumn(3).getInt();
      desc[RE_ACSEL_BUNDLE_CREATION_TIME] = q.getColumn(4).getText();
      desc[RE_ACSEL_BUNDLE_AUTHOR]        = q.getColumn(5).getText();
      shader[RE_ACSEL_BUNDLE_DESCRIPTION] = desc;
      shader[RE_ACSEL_BUNDLE_SHADER_CODE] = q.getColumn(2).getText();
      auto blobData = q.getColumn(6);
      int blobSize = blobData.getBytes();
      shader[RE_ACSEL_BUNDLE_THUMBNAME]   =  blobToText(blobData.getBlob(), 
                                                        blobSize);
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_INFO() << "Error in Universal Shader export: " << e.what();
  }

  return shader;
}


bool ReAcsel::storeOriginalShader( const QString& objID, 
                                   const QString& matID,
                                   const QString& shaderCode ) 
{
  if (!dbOpen) {
    return false;
  }
  bool retVal; 
  try{
    QString q = QString(
                  "INSERT OR REPLACE INTO %1 (App, ObjectID, MatID, ShaderCode)"
                  " values(?,?,?,?)"
                ).arg(RE_ACSEL_TABLE_ORIGINAL_SHADERS);
    SQLite::Statement s( *db, q.toUtf8());
    s.bind(1, (const char*)getAppCode().toAscii());
    s.bind(2, (const char*)objID.toUtf8());
    s.bind(3, (const char*)matID.toUtf8());
    s.bind(4, (const char*)shaderCode.toUtf8());
    retVal = s.exec();
  }
  catch( SQLite::Exception e ) {
    RE_LOG_INFO() << "Exception in ReAcsel::storeOriginalShader(): "
                  << e.what();
  }
  return retVal;
}


QString ReAcsel::exportDbToBundle( const QString& bundleFileName ) {
  if (!dbOpen) {
    return "Database is not available";
  }

  // Make sure to export only non commercial shader sets
  QString qry = QString("SELECT SetID from %1 WHERE Flags == 0").arg(RE_ACSEL_TABLE_SETS);
  SQLite::Statement s(*db, qry.toUtf8());
  QStringList sets;
  while (s.executeStep()) {
    sets << s.getColumn(0).getText();
  }

  qry = QString("SELECT ID from %1").arg(RE_ACSEL_TABLE_UNI_SHADERS);
  SQLite::Statement su(*db, qry.toUtf8());
  QStringList universalShaders;
  while (su.executeStep()) {
    universalShaders << su.getColumn(0).getText();
  }

  return exportToBundle(
           sets, 
           universalShaders, 
           "ACSEL Base Bundle", 
           "Pret-a-3D", 
           bundleFileName
         );
}

QString ReAcsel::exportToBundle( const QStringList& sets,
                                 const QStringList& universalShaders,
                                 const QString& title,
                                 const QString& sourceName,
                                 const QString& fileName ) {
  if (!dbOpen) {
    return "Database is not available";
  }
  QFile bundleFile(fileName);
  if (!bundleFile.open(QIODevice::WriteOnly)) {
    return QString(
             QObject::tr(
               "Could not open the file %1 for writing the ACSEL bundle \"%2\""
             )
           )
           .arg(fileName);
  }
  QVariantMap bundle;
  bundle[RE_ACSEL_BUNDLE_ID]      = RE_ACSEL_BUNDLE_SIGNATURE;
  bundle[RE_ACSEL_BUNDLE_TITLE]   = title;
  bundle[RE_ACSEL_BUNDLE_VERSION] = RE_ACSEL_VERSION;
  bundle[RE_ACSEL_BUNDLE_SOURCE]  = sourceName;

  QVariantList shaderData;
  int numSets = sets.count();
  if (numSets) {
    for (int i = 0; i < numSets; i++) {
      shaderData.append( exportShaderSet(sets[i]) );
    }
  }
  // Export the universal shaders
  int numUniShaders = universalShaders.count();
  if (numUniShaders) {
    for (int i = 0; i < numUniShaders; i++) {
      shaderData.append( exportUniversalShader(universalShaders[i]) );
    }
  }
  bundle[RE_ACSEL_BUNDLE_DATA] = shaderData;
  // JSONize and write to disk
  QJson::Serializer json;
  json.setIndentMode(QJson::IndentFull);
  if ( bundleFile.write(json.serialize(bundle)) == -1 ) {
    return QString(
             QObject::tr("Error in writing the ACSEL bundle to file %1")
           )
           .arg(fileName);
  }

  bundleFile.close();
  return "";
}

//! Check the version passed in the first parameter if it's less then or
//! equal to the current ACSEL version.
//! \return True if the condition matches, False if the version is higher
//!         than the current ACSEL version.
bool ACSELVersionCheck( const QString bv ) {
  // bv = Bundle Version. cv = Current Version
  return bv.toFloat() <= QString(RE_ACSEL_VERSION).toFloat();
}

ReAcsel::ReturnCode ReAcsel::checkIfShaderSetsExist( 
                      const QVariantMap& bundle, 
                      ExistingShaderSets& existingShaderSets 
                    )
{
  if (!dbOpen) {
    return ReAcsel::GeneralError;
  }
  // Just to stay on the safe side...
  existingShaderSets.clear();

  QVariantList shaderSets = bundle.value(RE_ACSEL_BUNDLE_DATA).toList();
  int shaderCount = shaderSets.count();

  for (int i = 0; i < shaderCount; i++) {
    auto oneEntry = shaderSets[i].toMap();
    if ( oneEntry.value(RE_ACSEL_BUNDLE_TYPE) == RE_ACSEL_UNIVERSAL_SHADER_SIGNATURE ) {
      // Universal shaders are structured differently
      QString id = oneEntry[RE_ACSEL_BUNDLE_ID].toString();
      QVariantMap data;
      if (findUniversalShader(id, data)) {
        existingShaderSets[oneEntry[RE_ACSEL_BUNDLE_NAME].toString()] = 
          oneEntry[RE_ACSEL_BUNDLE_DESCRIPTION].toString();
      }
      continue;
    }
    // shaderSets[i] gives us the whole object that defines the shader set,
    // which means the three sections of descripion, shaders and uuids.
    QVariantMap ss = shaderSets[i].toMap().value(RE_ACSEL_BUNDLE_DESCRIPTION).toMap();
    if ( !(ss.contains(RE_ACSEL_BUNDLE_SET_ID) && ss.contains(RE_ACSEL_BUNDLE_DESCRIPTION)) ) {
      return NotAnACSELBundle;
    }
    QString setID = ss[RE_ACSEL_BUNDLE_SET_ID].toString();
    QString setName = ss[RE_ACSEL_BUNDLE_SET_NAME].toString();
    QVariantMap data;
    if (findShaderSet(setID, data)) {
      existingShaderSets[setName] = data.value(RE_ACSEL_BUNDLE_DESCRIPTION).toString();
    }
  }
  if (existingShaderSets.count() > 0) {
    return ShaderSetAlreadyExists;
  }
  return Success;
}

ReAcsel::ReturnCode ReAcsel::importShaderSet( QVariantMap& shaderData ) {
  QVariantMap desc        = shaderData.value(RE_ACSEL_BUNDLE_DESCRIPTION).toMap();
  QVariantList shaderList = shaderData.value(RE_ACSEL_BUNDLE_SHADERS).toList();
  QVariantMap volumes     = shaderData.value(RE_ACSEL_BUNDLE_VOLUMES).toMap();
  QVariantMap uuids       = shaderData.value(RE_ACSEL_BUNDLE_UUIDS).toMap();

  QString author = desc.value(RE_ACSEL_BUNDLE_AUTHOR).toString().toLower();
  QString setID = desc.value(RE_ACSEL_BUNDLE_SET_ID).toString();
  // Check if the set is already in the database and if the author is 
  // the same. We don't allow importing a new set definition if it's
  // from a different author. This is particularly important for 
  // Generic presets
  QVariantMap tmpVals;
  bool setFound = false;
  // A generic set is searched only based on the objectID. If found then 
  // it can be updated only if the author is the same
  if (desc.value(RE_ACSEL_BUNDLE_IS_DEFAULT).toBool()) {
    if (findDefaultShaderSet(desc.value(RE_ACSEL_BUNDLE_OBJECT_ID).toString(),
                             tmpVals)) 
    {
      setFound = true;
      if (tmpVals[RE_ACSEL_BUNDLE_AUTHOR].toString().toLower() != author) {
        RE_LOG_INFO() << QString("Import bundle. Bundle author: %1, "
                                 "originally: %2")
                           .arg(tmpVals[RE_ACSEL_BUNDLE_AUTHOR].toString().toLower())
                           .arg(author)
                           .toStdString();
        return UnAuthorizedUpdate;
      }
    }  
  }
  if (!setFound) {
    if (findShaderSet(setID, tmpVals)) {
      setFound = true;
      if (tmpVals[RE_ACSEL_BUNDLE_AUTHOR].toString().toLower() != author) {
        return UnAuthorizedUpdate;
      }
    }
  }
  // A set that replaces an existing one could coneivably have fewer
  // shaders than the previous version. So, we need to delete the existing
  // shader set before we install the new version, otherwise we will have
  // shaders from the old version mixed with the new ones.
  // RE_LOG_INFO() << "Previous shader set found";
  if (setFound) {
    // RE_LOG_INFO() << "Delete set " << setID.toStdString();
    deleteShaderSet(setID);
  }
  // Save the set
  if ( !saveShaderSet(setID,
                      desc.value(RE_ACSEL_BUNDLE_SET_NAME).toString(), 
                      desc.value(RE_ACSEL_BUNDLE_DESCRIPTION).toString(), 
                      desc.value(RE_ACSEL_BUNDLE_AUTHOR).toString(),
                      desc.value(RE_ACSEL_BUNDLE_OBJECT_ID).toString(),
                      desc.value(RE_ACSEL_BUNDLE_IS_DEFAULT).toBool(),
                      desc.value(RE_ACSEL_BUNDLE_IS_ENABLED).toBool()) ) 
  {
    return CannotAddShaderSet;
  }
  // Add the UUIDs
  QMapIterator<QString, QVariant> ui(uuids);
  while( ui.hasNext() ) {
    ui.next();
    storeUUID(ui.key(), ui.value().toString());
  }

  // Install the shaders
  int numShaders = shaderList.count();
  for (int l = 0; l < numShaders; l++) {
    QVariantMap shader = shaderList[l].toMap();
    saveShader(shader.value(RE_ACSEL_BUNDLE_UUID).toString(), 
               shader.value(RE_ACSEL_BUNDLE_SHADER_CODE).toString(),
               getMaterialType(shader.value(RE_ACSEL_BUNDLE_MATERIAL_TYPE).toString()),
               shader.value(RE_ACSEL_BUNDLE_MATERIAL_NAME).toString(),
               setID,
               shader.value(RE_ACSEL_BUNDLE_VOLUME_ID).toInt());
  }
  // Install the volumes
  QMapIterator<QString, QVariant> vi(volumes);
  while( vi.hasNext() ) {
    vi.next();
    saveVolume(vi.key().toInt(), setID, vi.value().toString());
  }
  return Success;
}

ReAcsel::ReturnCode ReAcsel::importUniversalShader( QVariantMap& shaderData ) {
  QVariantMap desc = shaderData.value(RE_ACSEL_BUNDLE_DESCRIPTION).toMap();
  QString ID = desc.value("ID").toString();
  auto blob = QByteArray::fromBase64(shaderData["Thumbnail"].toByteArray());
  bool retVal = saveUniversalShader(
                  ID, 
                  desc.value(RE_ACSEL_BUNDLE_NAME).toString(), 
                  desc.value(RE_ACSEL_BUNDLE_DESCRIPTION).toString(), 
                  shaderData.value(RE_ACSEL_BUNDLE_SHADER_CODE).toString(), 
                  desc.value(RE_ACSEL_BUNDLE_CATEGORY).toInt(), 
                  desc.value(RE_ACSEL_BUNDLE_AUTHOR).toString(),
                  blob
                );
  if (retVal) {
    return Success;
  }
  return GeneralError;
}

ReAcsel::ReturnCode ReAcsel::importBundle( const QString bundleFileName, 
                                           const bool overwrite,
                                           ExistingShaderSets& existingShaderSets ) 
{
  if (!dbOpen) {
    return GeneralError;
  }

  QJson::Parser jsonParser;
  bool ok = false;
  QVariantMap bundle;

  QFile bundleFile(bundleFileName);
  if (bundleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    bundle = jsonParser.parse(&bundleFile, &ok).toMap();
    bundleFile.close();
  }

  if (!ok) {
    return SyntaxError;
  }
  // SQLite::Transaction tran(*db);
  AcselTransactionPtr tran = startTransaction();
  // Integrity check
  if (bundle.value(RE_ACSEL_BUNDLE_ID).toString() != RE_ACSEL_BUNDLE_SIGNATURE) {
    return NotAnACSELBundle;
  }
  QString version = bundle.value(RE_ACSEL_BUNDLE_VERSION).toString();
  if ( !ACSELVersionCheck(version) ) {
    return WrongACSELVersion;
  }

  // Check if the shaders in the bundle is already in the ACSEL database
  if (!overwrite) {
    if ( checkIfShaderSetsExist( bundle, existingShaderSets ) == ShaderSetAlreadyExists ) {
      return ShaderSetAlreadyExists;
    }
  }
  /**************************
   * Install the shaders
   **************************/

  QVariantList shaderSets = bundle.value(RE_ACSEL_BUNDLE_DATA).toList();
  int shaderCount = shaderSets.count();
  ReturnCode retCode;
  for (int i = 0; i < shaderCount; i++) {
    QVariantMap shaderData  = shaderSets[i].toMap();
    QString shaderType = shaderData.value(RE_ACSEL_BUNDLE_DESCRIPTION).toMap()
                           .value(RE_ACSEL_BUNDLE_TYPE).toString();
    if (shaderType == RE_ACSEL_SHADER_SET_SIGNATURE) {
      if ( (retCode = importShaderSet(shaderData)) != Success ) {
        return retCode;
      }
    }
    else if (shaderType == RE_ACSEL_UNIVERSAL_SHADER_SIGNATURE) {
      importUniversalShader(shaderData);
    }
  }
  tran->commit();
  return Success;
}

void ReAcsel::importObjectAliases( const QVariantMap& aliases ) {
  QString qry = QString("INSERT OR REPLACE INTO %1 (ObjectID, Alias) VALUES(:objID,:alias)").arg(RE_ACSEL_TABLE_ALIASES);
  SQLite::Statement s( *db, qry.toUtf8());

  QMapIterator<QString, QVariant> i(aliases);
  try {
    while( i.hasNext() ) {
      i.next();
      RE_LOG_INFO() << "Importing alias " << i.key() << " => " << i.value().toString();
      s.bind(":objID", static_cast<const char*>(i.key().toUtf8()));
      s.bind(":alias", static_cast<const char*>(i.value().toString().toUtf8()));
      s.exec();
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
  }
}

void ReAcsel::importObjectAliases( const QString& aliasFileName ) {
  if (!dbOpen) {
    return;
  }

  RE_LOG_INFO() << "Importing alias file " << aliasFileName;
  QJson::Parser jsonParser;
  bool ok = false;
  QVariantMap aliases;
  QFile aliasFile(aliasFileName);
  if (aliasFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    aliases = jsonParser.parse(&aliasFile, &ok).toMap();
    aliasFile.close();
  }

  if (!ok) {
    RE_LOG_INFO() << "  conversion to JSON failed";
    return;
  }
  importObjectAliases(aliases);
}

void ReAcsel::setShaderSetEnabled( const QString& setID, bool isEnabled ) {
  if (!dbOpen) {
    return;
  }
  runQuery( 
    QString("UPDATE %1 set IsEnabled=%2 WHERE SetID='%3'")
      .arg(RE_ACSEL_TABLE_SETS)
      .arg(isEnabled ? 1 : 0)
      .arg(setID)
  );
}

bool ReAcsel::isFigure( const QString figName ) {

  if (!dbOpen) {
    return false;
  }
  try {
    SQLite::Statement q( 
      *db, 
      QString("SELECT FigureName from %1 WHERE FigureName='%2'")
        .arg(RE_ACSEL_TABLE_FIGURES)
        .arg(figName)
        .toUtf8()
    );
    return q.executeStep();
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
  }
  return false;
}

bool ReAcsel::getFigures( QVariantMap& figures ) {
  if (!dbOpen) {
    return false;
  }
  QString q = QString("SELECT * from %1").arg(RE_ACSEL_TABLE_FIGURES);
  try {
    SQLite::Statement s( *db, q.toUtf8());
    while( s.executeStep() ) {
      figures[s.getColumn(0).getText()] = s.getColumn(1).getText();
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
    return false;
  }
  return true;
}

bool ReAcsel::saveUniversalShader( QString& shaderID,
                                   const QString& shaderName, 
                                   const QString& desc, 
                                   const QString& shaderCode, 
                                   const QString& category,
                                   const QString& author,
                                   const QString& previewFileName ) 
{
  if (!dbOpen) {
    return false;
  }
  try {
    SQLite::Statement q(*db, 
                        QString(
                          "SELECT CategoryID from %1 WHERE Name='%2'"
                        )
                        .arg(RE_ACSEL_TABLE_CATEGORIES)
                        .arg(category)
                        .toUtf8());
    if (q.executeStep()) {
      int cat = q.getColumn(0).getInt();
      return saveUniversalShader(
        shaderID, shaderName, desc, shaderCode, cat, author, previewFileName
      );
    }
  } 
  catch(...) {
    // nothing
  }
  return false;
}

bool ReAcsel::saveUniversalShader( QString& shaderID,
                                   const QString& shaderName, 
                                   const QString& desc, 
                                   const QString& shaderCode, 
                                   const int category,
                                   const QString& author,
                                   const QString& previewFileName ) 
{
  QByteArray blob;

  if (!previewFileName.isEmpty()) {
    QFile previewFile(previewFileName);
    if (previewFile.open(QIODevice::ReadOnly)) {
      blob = previewFile.readAll();
      previewFile.close();
    }
  }
  return saveUniversalShader(
           shaderID, shaderName, desc, shaderCode, category, author, blob
         );
}

bool ReAcsel::saveUniversalShader( QString& shaderID,
                                   const QString& shaderName, 
                                   const QString& desc, 
                                   const QString& shaderCode, 
                                   const int category,
                                   const QString& author,
                                   const QByteArray& blob ) 
{
  if (!dbOpen) {
    return false;
  }
  try {
    bool isNew;
    if ( shaderID.isEmpty() ) {
      shaderID = createUUID();
      isNew = true;
    }
    else {
      // This test covers the case when we import from a bundle
      QString qry = QString("SELECT ID from %1 WHERE ID=:id")
                      .arg(RE_ACSEL_TABLE_UNI_SHADERS);
      SQLite::Statement s( *db, qry.toUtf8());
      s.bind(":id", shaderID.toStdString());
      isNew = !s.executeStep();
    }
    // Save the shader
    QString qry;
    if (isNew) {
      qry = QString(
               "INSERT into %1 (ID, Name, Description, ShaderCode,"
               " Category, Author, Thumbnail)"
               " VALUES(:id,:name,:description,:shaderCode,:cat,:author,"
               " :thumb)"
             )
             .arg(RE_ACSEL_TABLE_UNI_SHADERS);
    }
    else {
      if (blob.size() > 0) {
        qry = QString(
                 "UPDATE %1 set Name=:name, Description=:description,"
                 " ShaderCode=:shaderCode, Author=:author, Category=:cat,"
                 " Thumbnail=:thumb WHERE ID=:id"
               )
               .arg(RE_ACSEL_TABLE_UNI_SHADERS);
      }
      else {
        qry = QString(
                 "UPDATE %1 set Name=:name, Description=:description,"
                 " ShaderCode=:shaderCode, Author=:author, Category=:cat WHERE ID=:id"
               )
               .arg(RE_ACSEL_TABLE_UNI_SHADERS);
      }
    }
    SQLite::Statement q( *db, qry.toUtf8());
    q.bind(":id", static_cast<const char*>(shaderID.toAscii()));
    q.bind(":name", static_cast<const char*>(shaderName.toUtf8()));
    q.bind(":description", static_cast<const char*>(desc.toUtf8()));
    q.bind(":shaderCode", static_cast<const char*>(shaderCode.toUtf8()));
    q.bind(":author", static_cast<const char*>(author.toUtf8()));
    q.bind(":cat", category);
    if (blob.size() > 0) {
      q.bind(":thumb", (void*)blob.data(), blob.size());
    }
    bool result = q.exec();
    if (!result) {
      lastErrorMsg = db->getErrorMsg();
      RE_LOG_WARN() << "Error while saving a universal shader: " 
                    << lastErrorMsg.toStdString();
    }
    return result;
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
  }
  return false;
}

bool ReAcsel::findUniversalShader( const QString& shaderID, 
                                   QVariantMap& data ) {
  if (!dbOpen) {
    return false;
  }
  data[RE_ACSEL_BUNDLE_NAME]        = "";
  data[RE_ACSEL_BUNDLE_DESCRIPTION] = "";
  data[RE_ACSEL_BUNDLE_SHADER_CODE] = "";

  bool found = false;
  QString query = QString( 
                    "SELECT us.Name,us.Description,us.Category,"
                    " us.ShaderCode,cat.Name as CategoryName from %1 as us,"
                    " %2 as cat WHERE ID=:id and us.Category=cat.CategoryID" )
                    .arg(RE_ACSEL_TABLE_UNI_SHADERS)
                    .arg(RE_ACSEL_TABLE_CATEGORIES);

  SQLite::Statement q(*db, query.toUtf8());
  q.bind(":id", static_cast<const char*>(shaderID.toAscii()));
  if (q.executeStep()) {
    data[RE_ACSEL_BUNDLE_NAME]         = QString(q.getColumn(0));
    data[RE_ACSEL_BUNDLE_DESCRIPTION]  = QString(q.getColumn(1));
    data[RE_ACSEL_BUNDLE_CATEGORY]     = QString(q.getColumn(2));
    data[RE_ACSEL_BUNDLE_SHADER_CODE]  = QString(q.getColumn(3));
    data["CategoryName"] = QString(q.getColumn(4));
    found                = true;
  }
  return found;
}

void ReAcsel::getUniversalShaderList( QList<QStringList>& records, 
                                      const QString& category ) {
  if (!dbOpen) {
    return;
  }
  QString qry;
  if (category.isEmpty()) {
    qry = QString("SELECT ID,Name,Description,CreationTime from %1"
                  " ORDER BY Name")
            .arg(RE_ACSEL_TABLE_UNI_SHADERS);
  }
  else {
    qry = QString("SELECT ID,us.Name,Description,CreationTime from %1 as us,"
                  " %2 as cat WHERE us.Category=cat.CategoryID AND"
                  " cat.Name='%3' ORDER BY us.Name")
            .arg(RE_ACSEL_TABLE_UNI_SHADERS)
            .arg(RE_ACSEL_TABLE_CATEGORIES)
            .arg(category);
  }
  SQLite::Statement q(*db, qry.toUtf8());

  while (q.executeStep()) {
    QStringList* r = new QStringList();
    // ID
    r->append( static_cast<const char*>(q.getColumn(0)) ) ;
    // Name
    r->append( static_cast<const char*>(q.getColumn(1)) ) ;
    // Description
    r->append( static_cast<const char*>(q.getColumn(2)) ) ;
    // Creation time
    r->append( static_cast<const char*>(q.getColumn(3)) ) ;
    records.append(*r);
  }
}

void ReAcsel::getCategoryList( QStringList& list ) {
  if (!dbOpen) {
    return;
  }

  QString qry = QString("SELECT Name from %1 order by Name")
                  .arg(RE_ACSEL_TABLE_CATEGORIES);
  SQLite::Statement q(*db, qry.toUtf8());
  while( q.executeStep() ) {
    list << static_cast<const char*>(q.getColumn(0));
  }
}

void ReAcsel::getUniversalShaderPreview( const QString& shaderID,
                                         unsigned char*& bitmapData,
                                         int& blobSize ) {
  bitmapData = NULL;
  blobSize = 0;
  if (!dbOpen) {
    return;
  }
  try {
    QString qry = QString("SELECT Thumbnail from %1 WHERE ID='%2'")
                    .arg(RE_ACSEL_TABLE_UNI_SHADERS)
                    .arg(shaderID);
    SQLite::Statement s( *db, qry.toUtf8());
    if (s.executeStep()) {
      blobSize = s.getColumn(0).size();
      if (blobSize > 0) {
        bitmapData = new unsigned char[blobSize];
        memcpy(bitmapData, s.getColumn(0).getBlob(), blobSize);
      }
    }
  }
  catch( SQLite::Exception e ) {
    RE_LOG_WARN() << e.what();
  }
}

