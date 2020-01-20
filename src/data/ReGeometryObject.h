/**
  File: ReGeometryObject.h

  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/


#ifndef REGEOMETRY_OBJECT_H
#define REGEOMETRY_OBJECT_H
  
#include "ReDefs.h"
#include "ReMaterials.h"
#include "ReLightMaterial.h"
#include "ReGlossy.h"
#include "textures/ReColorMath.h"
#include "textures/ReMix.h"

//! Specular strength needs to be corrected when converting from Poser or 
//! Studio. The following constant specifies the factor of correction.
#define RE_SPECULAR_CORRECTION_FACTOR 0.7

namespace Reality {

//! A dictionary (QHash) of <ReMaterials> keyed by material GUID
//! This is used as the main material catalog for the scene.
typedef QHash<QString, ReMaterialPtr> ReMaterialDictionary;

//! A iterator for <ReMaterialDictionary>
typedef QHashIterator<QString, ReMaterialPtr> ReMaterialIterator;

/**
 This class is  a simple collection of all the textures and colors that we
 need to keep around during the conversion process. Most of the information
 stored here is then moved to the right slots in specific materials. The textures
 held in here are from the host application shaders and they need to be collected
 here before we know exactly what type of material they will lead to.
 */
class ReMaterialInfo {

public:

  //! Variable that holds the provenance of the shader that we are converting.
  QString shaderSource;

  //! If enabled this material is for skin
  bool isSkin;
  //! If enabled this material emits light
  bool isLight;

  //! Possible preset used to initialize the material. This could include
  //! things like "silk" for cloth, or "architectural" for glass.
  QString preset;

  //! The host can suggest what material type should be associated to 
  //! the material. This is not an override but a hint, in case all the other
  //! methods, including ACSEL, don't come up with a material type.
  ReMaterialType matTypeHint;

  //! Flag used to communicate if a default ACSEL shader has been found
  bool foundAcselDefaultShader;

  //! Glad used to communicate that we use the ACSEL shader only to set
  //! the type of the material. We don't actually read the shader values.
  bool setMaterialTypeOnly;

  //! Data for the default/generic ACSEL shader. The data is stored here so 
  //! that we avoid reading the shader data twice and therefore we save an
  //! access to the ACSEL database. This is a very simple form of caching
  QVariantMap acselShader;

  //! The ID of the ACSEL shader set associated to this material
  QString acselSetID;

  //! The Unique ID of this shader
  QString acselID;

  //! If enabled the light-emitting material should have a light alpha map
  bool lightAlpha;
  //! Gain of the light-emitting material
  float lightGain;
  //! Name of the materials
  QString materialName;

  bool translucenceEnabled; 

  QColor diffuseColor,
         diffuse2Color,
         specularColor,
         translucenceColor,
         ambientColor,
         coatColor;

  ReTexturePtr diffuseMap, 
               diffuse2Map,
               bumpMap, 
               displacementMap,
               alphaMap, 
               specularMap,
               glossinessMap, 
               translucenceMap,
               absorptionMap,
               scatterMap,
               ambientMap,
               coatMap;  

  //! Bump map strength, positive and negative values
  double bmStrength;
  double bmPositive, bmNegative;

  //! Disp map strength, positive and negative values
  double dmStrength;
  double dmPositive, dmNegative;

  float alphaStrength;
  //! Glossiness values
  qint32 uGlossiness, vGlossiness;

  ReMaterialInfo() {
    init();
  }

  void init() {
    isSkin                  = false;
    isLight                 = false;
    preset                  = "";
    shaderSource            = "";
    setMaterialTypeOnly     = false;
    matTypeHint             = MatUndefined;
    foundAcselDefaultShader = false;
    lightAlpha              = false;
    lightGain               = 1.0;
    alphaStrength           = 1.0;
    uGlossiness             = vGlossiness = 0;
    bmStrength              = 0.1;
    bmPositive              = 0.001;
    bmNegative              = -0.001;
    translucenceEnabled     = false;
    materialName            = "";
    
    diffuseMap.clear();
    diffuse2Map.clear();
    specularMap.clear();
    bumpMap.clear();
    displacementMap.clear();
    alphaMap.clear();
    translucenceMap.clear();
    absorptionMap.clear();
    scatterMap.clear();
    glossinessMap.clear();
    ambientMap.clear();
    coatMap.clear();
    acselShader.clear();
    acselSetID = "";
    acselID = "";
  }

/*
 Incomplete
 
  void setup( QVariantMap& data ) {
    isSkin    = data["isSkin"].toBool();
    isLight   = false;
    QColor tmpClr = convertFloatColor(data["ambient"].toMap()["color"].toList());

    isLight   = !isColorBlack(tmpClr) && data["lightGain"].toFloat() > 0;
    lightGain = data["lightGain"].toFloat();
    foundAcselDefaultShader = false;
    lightAlpha = data["lightAlpha"].toBool();
    alphaStrength = data["alpha"].toMap()["strength"].toFloat();
    uGlossiness = (int)((1.0 - data["uRoughness"].toDouble()) * 10000);
    vGlossiness = (int)((1.0 - data["vRoughness"].toDouble()) * 10000);
    // adjust the specular value down to account for biased vs unbiased difference
    uGlossiness *= RE_SPECULAR_CORRECTION_FACTOR;
    vGlossiness *= RE_SPECULAR_CORRECTION_FACTOR;
  }
  */
};


//! A smart pointer to a ReGeometryObject. Used for garbage collection.
typedef QSharedPointer<ReGeometryObject> ReGeometryObjectPtr;

//! Smart pointer to a material dictionary
typedef QSharedPointer<ReMaterialDictionary> ReMaterialDictionaryPtr;

/**
 * A class to store a reference to an objects present in the scene.
 * The class keeps a few information about the object itself and then 
 * stores the list of materials discovered by the plugin and the material 
 * converter.
 */

class RE_LIB_ACCESS ReGeometryObject {

protected:
  //! The name of the the object in the scene. This is obtained from the 
  //! hosting application and must be unique in the scene.
  QString name;

  /**
   * An name that identifies the object uniquely. Not all object have this but 
   * if they do, the name helps Reality in determining the true nature of the 
   * object. If the object is handled by Poser this name reflects the geometry
   * file name, the .obj file. A V4 mode, for example, will always have this 
   * property set to the name of the obj file, without the extension, no matter
   * how the object is called in the scene.
   */
  QString internalName;

  /**
   * The name of the geometry file that holds the mesh for this object. This 
   * name is unique for each object but not unique for the scene. For example, 
   * Victoria 4 is stored in a obj file named blMilWom_v4b. That name works 
   * to uniquely identify the geometry for the purpose of converting its 
   * materials. On the other hand there can be multiple V4 figures in a scene.
   */
  QString geometryFile;

  //! This is a the dictionary of materials associated with this object
  ReMaterialDictionary materials;

  //! The list of materials converted to light
  ReMaterialDictionaryPtr lights;

  //! Flag used to signal if this object is actually a mesh light.
  //! Mesh light objects don't get listed in the material editor.
  bool isLightFlag;

  //! Visibility flag.
  bool visible;

  //! If this variable is on then the object refer to a human figure.
  //! This piece of data is used to set the gain for diffuse maps to 1.0
  //! regardless of what is in the original material. Several figures from
  //! Studio, especially the Genesis 2-base figures, have skin maps set to
  //! less than 1.0. That fact causes issues with Lux and with the system
  //! Reality confugures SSS.
  bool isHumanFigure;

  //! Identifier of the object for which this object is an instance. The 
  //! string is used to uniquely reference the original object.
  //! nstance objects are simple repetitions of source objects. They have 
  //! no geometry or materials of their own
  QString instanceSourceID;

  /**
   * Converts a material from the JSON representation to a Reality material of the 
   * proper kind. 
   *
   * \param  matGUID The GUID of the material to be converted
   * \param  srcMaterial The JSON string that describes the material from the 
   *                     hosting application. The data structure is a 
   *                     dictionary defined as follows:
   *
   *  type: "short type" (one of the three letter short types like GLO, GLS)
   *  preset: "string"   (a string, possibly empty, that indicates the preset for 
   *                      the material. For example, "frosted" for glass or
   *                      "silk" for cloth)
   *  source: one of: "ff" (FireFly), "3dl" (3Delight), "ir" (iRay)
   * 
   *  # The Diffuse channel
   *  diffuse:
   *      map: "<texture name>"
   *      color: [0.0, 0.0, 0.0]
   *      strength: 1.0
   *      
   *  # Alternate Diffuse channel
   *  diffuse2:
   *      map: "<texture name>"
   *      color: [0.0, 0.0, 0.0]
   *  
   *  # Specular channel
   *  specular:
   *      map: "<texture name>"
   *      color: [0.0, 0.0, 0.0]
   *  
   *  # Second Specular channel
   *  "specular 2":
   *      map: "<texture name>"
   *      color: [0.0, 0.0, 0.0]

   *  # Glossy coat
   *  coat:
   *      map: "<texture name>"
   *      color: [0.0, 0.0, 0.0]
   *      
   *  bump:
   *      map: "<texture name>"
   *      strength: 0.5
   *      neg: -0.001
   *      pos: 0.001
   *  
   *  displacement:
   *      map: "<texture name>"
   *      strength: 0.5
   *      neg: -0.001
   *      pos: 0.001
   *      
   *  translucence:
   *      color: [0.0, 0.0, 0.0]
   *      map: "<texture name>"
   *      strength: 1.0
   *  
   *  alpha:
   *      map: <texture name>
   *      strength: 1.0
   *  
   *  # Specular roughness. Horizontal and vertical
   *  uRoughness: 1.0
   *  vRoughness: 1.0
   *  "roughness map": "<texture name>"
   *  
   *  isSkin: False
   *  lightGain: 0.0
   *  lightAlpha: False
   *  
   *  ambient: 
   *      map: "<texture name>"
   *      color: "<color>"
   *  
   *  # Light intensity. Not used
   *  intensity: 1.0
   *  
   *  # Spotlight angle. Not used
   *  angle: 50.0
   *  
   *  nodes:
   *      <node id>:
   *          <node data>
   *  
   *  # Nodes
   *  
   *  # Image map
   *  type: 170
   *  fileName: <path>
   *  "u tile": 1.0
   *  "v tile": 1.0
   *  "u offset": 1.0
   *  "v offset": 1.0
   *  "gain': 1.0
   *  
   *  # Mix Texture 
   *  "type": 190
   *  "tex1 color": [<float>, <float>, <float>]
   *  "tex1 map": <node id>
   *  "tex2 color": [<float>, <float>, <float>]
   *  "tex2 map": <node id>
   *  "mix": <float>
   *  "mix map": <node id>
   *
   *  # Math Texture 
   *  "type": 235
   *  "value 1": Map{ "map": <node id>, "value": float }
   *  "value 2": Map{ "map": <node id>, "value": float }
   *  "function": one of "a", "s", "m"
   *
   */
  ReMaterialPtr convertMaterial( const QString matGUID, 
                                 const QString srcMaterial,
                                 const ReMaterialType materialType = MatUndefined );

  //! Overloaded version that uses a QVAriantMap for input instead of the
  //! JSON string
  ReMaterialPtr convertMaterial( const QString matID, 
                                 const QVariantMap& srcMat,
                                 ReMaterialType materialType = MatUndefined );
  ReMaterialInfo matInfo;

  //! Returns the ACSEL ID for a material based on the textures present
  //! \param matID The ID of the material for which we need the ACSEL ID
  //! \param forDefaultShader If this parameter is set to true the the 
  //!                         ID returned is for the default shader
  QString computeAcselID( const QString& matID, 
                          const bool forDefaultShader = false );

private:

  void createMaterial( const QString matID, 
                       ReMaterialPtr& matPtr, 
                       const ReMaterialType materialType = MatUndefined);

  /**
   * Creates a material from an ACSEL shader,if present. Otherwise it 
   * simply exist without any change.
   * \param matID The ID of the material. This is used to find 
   *              if a shader is present in the ACSEL database
   * \param matPtr  If a shader is found then a materila will be created
   *                and this pointer will be set to that object.
   * \return A ReMaterialType set to a valid material type if an ACSEL 
   *         shader was found. Otherwise the value is set to MatUndefined
   */
  ReMaterialType createMaterialFromAcselShader( const QString& matID, 
                                                ReMaterialPtr& matPtr );

  //! Removes the light materials from the scene
  void removeOwnLights();

  //! Convert the specular data from the host into a single texture.
  //! We have two posisble channels of specular in both Poser and Studio
  //! In Poser they are called "specular" and "alternate specular".
  //! In Studio they are called "specular" and "specular 2".
  //! In either case, if both are available we convert them both and then
  //! combine them via multiplication.
  //! This method is used to perform the same conversion on both "specular"
  //! and "specular 2" dictionaries. 
  //! \param specData A QVariantMap that holds the material data from the
  //!                 host.
  //! \param matID The ID of the material being processed.
  //! \param suffix A string to make the ID of the textures created unique.
  //! \return A texture corresponding to the data in input.
  ReTexturePtr convertSpecular( const QVariantMap& specData, 
                                const QString& matID,
                                const QString& suffix );

  //! When we receive an iRay shader that contains translucency we convert 
  //! it in the following way:
  //! - The Diffuse texture is replaced with a Mix texture where texture 1
  //!   is the original Diffuse texture
  //! - Texture2 is an ReConstant texture set to the translucency color
  //! - The translucency map, if present, is set to be the mix texture
  void convertIRayTranslucency( Glossy* mat, ReTexturePtr transTex );

public:

  ReGeometryObject( const QString name, 
                    const QString internalName, 
                    const QString geometryFile );

  /**
    The destructor disconnects all the materials used by this object from their
    volumes.
   */
  ~ReGeometryObject();

  inline const QString& getName() const {
    return name;
  }

  inline const QString& getInternalName() const {
    return internalName;
  }

  inline void setInternalName( const QString newName ) {
    internalName = newName;
  }

  inline QString getGeometryFileName() const {
    return geometryFile;
  }

  void rename( const QString newName );

  //! Returns whether or not this object is an instance of another object
  inline bool isInstance() {
    return !instanceSourceID.isEmpty();
  }

  inline QString getInstanceSourceID() const {
    return instanceSourceID;
  }

  //! Set the ID of the instance source. If the ID is not blank then
  //! this object is flagged as being an instance.
  inline void setInstanceSourceID( const QString sourceID ) {
    instanceSourceID = sourceID;
  }

  /**
   *   Add a material to this object by converting its representation 
   *   obtained by the material converter running inside the host app.
   * 
   *   \param matID The name of the material
   *   \param scrMat A string in JSON format that describes the material
   *                 and its textures.
   */
  void addMaterial( const QString matID, const QString srcMat);
  //! Overloaded version
  void addMaterial( const QString matID, const QVariantMap& srcMat);
  
  inline void addMaterial( const QString matGUID, ReMaterialPtr newMat) {
    materials[matGUID] = newMat;
  }

  void updateMaterial( const QString matName, const QString srcMat);

  //! Overloaded version that uses a QVariant Map instead of a JSON string
  void updateMaterial( const QString matName, const QVariantMap srcMat);

  void changeMaterialType( const QString materialID, 
                           const QString jsonData, 
                           const ReMaterialType newType);

  void changeMaterialType( const QString materialID, 
                           const QVariantMap& srcData, 
                           const ReMaterialType newType);

  //! When a material is converted from the host type to the Reality version
  //! the original definition is stored in the ACSEL database. At any time
  //! the user can revert the shader to the original version. Thsi method
  //! performs that opertation.
  void revertToOriginalShader( const QString matID );

  //! Changes the type of a material by comverting one material to the new
  //! type without re-reading the base material data from the host.
  void changeMaterialType( const QString materialID, 
                           const ReMaterialType newType);

  inline int getNumMaterials() const {
    return materials.count();
  }

  inline QStringList getMaterialNames() {
    return materials.keys();
  }

  inline ReMaterialDictionary getMaterials() {
    return materials; 
  }

  inline ReMaterialPtr getMaterialAt(const qint16 numMat) {
    return materials[materials.keys()[numMat]];
  }

  inline bool hasMaterial( const QString& matName ) {
    return materials.contains(matName);
  }

  inline ReMaterialPtr getMaterial(const QString& matName) {
    if (materials.contains(matName)) {
      return materials[matName];
    }
    else {
      RE_LOG_WARN() << "Requested inexistent material " << matName.toStdString()
                    << " for object " << internalName.toStdString();
    }
    return ReMaterialPtr();
  }

  //! Returns the list of materials-turned-lights controlled by this objects
  inline ReMaterialDictionaryPtr getLights() {
    return lights; 
  }

  //! Returns a light material or a null pointer if the light does not exist in te list
  inline ReLightMaterialPtr getLight( const QString& lightName ) {
    if (lights->contains(lightName)) {
      return lights->value(lightName).staticCast<ReLightMaterial>();
    }
    return ReLightMaterialPtr();
  }

  //! Deletes a light material
  //! \param lightName, the name of the light material
  inline void deleteLight( const QString& lightName ) {
    if (lights->contains(lightName)) {
      lights->remove(lightName);
    }
  }

  //! Given a light ID, this method finds which material links to that light
  //! and deletes it, effectively breaking the link between the object and the
  //! light. This is necessary in order to delete lights and linked objects
  //! separately, so to avoid a deadlock.
  void unlinkLight( const QString& lightID );

  //! Returns wheter this object is actually a light, like in the case of
  //! a MeshLight
  inline bool isLight() const {
    return isLightFlag;
  }

  inline void markAsLight( const bool onOff ) {
    isLightFlag = onOff;
  }

  //! Returns whether this object contains materials that are
  //! light emitters. 
  bool isLightEmitter();

  //! Sets the visibiliy in render of the object
  inline void setVisible( const bool yesNo ) {
    visible = yesNo;
  }

  inline bool isVisible() {
    return visible;
  }

  /**
   * Adds a material to the object by reading its definition from the
   * parameter data. The data in that parameter is stored in the host app's
   * scene data in JSON format and converted to equivalent QVariantMap by the
   * caller of this method. This method is called when reading the material data 
   * from the scene file. 
   */
  void restoreMaterial( const QString matID, const QVariantMap& data );

/* OBSOLETE
  void restoreObject( const QVariantMap data );
*/

  //! This method is called when a material is changed in the host app.
  //! The new version of the material is serialized at the host app level
  //! and it's sent to the GUI. This method does the de-serialization and
  //! replacement of the old version.
  void deserializeMaterial( const QString& materialName, QDataStream& dataStream );

  //! Serializes the material lights for this object. This portion has been
  //! turned into a method because it is needed during the conversion of a material
  //! from one type to another.
  void serializeMaterialLights( QDataStream& dataStream );

  //! This method takes care of deserializing the pointers to material lights
  //! that are stored in the lights QMap in the object
  void deserializeMaterialLights( QDataStream& dataStream );

  void serialize( QDataStream& dataStream );  
  static ReGeometryObject* deserialize( QDataStream& dataStream);
};

}

#endif
