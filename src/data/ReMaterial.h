/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MATERIAL_H
#define RE_MATERIAL_H

#include "reality_lib_export.h"
#include "ReTextureContainer.h"


namespace Reality {

/**
 Base class for all Reality materials.
 */
class REALITY_LIB_EXPORT ReMaterial : public ReTextureContainer
{

protected:
  /**

   All nodes for the material. 

   Nodes, aka Textures, can be linked together to form a shader tree. Several textures
   can have links to a same texture. This requires the need to avoid nesting nodes together
   as we would end up with duplicates of a node. For example, if multiple ColorMath textures
   require to use the same base color in one channel we would require to duplicate that color
   texture. To avoid this situation a pool of nodes is created. All nodes are simply added to 
   the pool and addressed by their label/name (in the host app) or by the memory address. 

   When a primary node, a node that is linked directly by one of the
   channels of the material (Diffuse, Specular and so on), links to another node we 
   simply take the pointer to the linked node from the pool and add it to the node that 
   links to it. In this way we convert a given node only once and then we re-use it. 
   This effectively implements the node system as seen in Poser but it allows us to use 
   it inside the more clear and intuitive "Outline-like" UI of Reality.
   */
  ReNodeDictionary nodeCatalog;

  //! Every material defines a certain number of channels, which are in turn textures
  //! A channel is defined by its role in the material. There can be a Diffuse channel,
  //! a Specular channel and so on.
  //! This dictionary keeps the list of the channels defined by a material. The
  //! actual definition is provided by the subclasses of ReMaterial.
  ReNodeDictionary channels;

  //! The name of this material. The name is unique within the same figure/object
  QString name;

  //! The type of this material
  ReMaterialType type;

  //! The original type of this material
  ReMaterialType originalType;
  
  //! The <ReGeometryObject> that owns this material
  const ReGeometryObject* parent;

  QString innerVolume;
  QString outerVolume;
  
  ReTexturePtr nullValue;

  //! Indicates if the material has been modified from the original conversion
  bool edited; 

  bool visibleInRender;

  //! This string uniquely identifies a material in the ACSEL database
  QString acselID;

  //! The ID of ACSEL shader set used for this material
  QString acselSetID;

  //! This string hold the name of the acsel set associated with the 
  //! material. This variable is serialized but not stored.
  QString acselSetName;

private:

  static QString typeNames[MatUndefined+1];

  /**
   * Companion method for <findDependencies>, it traverses the list
   * of textures and find all the dependencies recursively,
   * The result is a sorted list with the dependencies at the
   * top of the list.
   */
  void findTextureDependencies( ReTexturePtr tex, 
                                QHash<QString, bool>& visited, 
                                QStringList& texList ) const;

public:
    ReMaterial( const QString name, const ReGeometryObject* parent );

    // Destructor: ~ReMaterial()
    virtual ~ReMaterial();

    //! Configure this material with data from the material passed
    //! in the first parameter. This method is used to convert one 
    //! material to a different type
    virtual void fromMaterial( const ReMaterial* bm );

    inline QString getName() const {
      return name;
    };

    inline void setName( QString newVal ) {
      name = newVal;
    };

    const QString getUniqueName() const;

    inline const ReNodeDictionary getChannels() const {
      return channels;
    }

    inline ReTexturePtr getChannel( const QString channelName ) const {
      if (channels.contains(channelName)) {
        return channels[channelName];
      }
      return ReTexturePtr();
    }

    /**
    Each material has a catalog that stores all the nodes, textures, that are
    forming the "material network". This network is a nodal system that 
    defines a material as a connection of nodes, each one providing input to
    the node that is above itself in the hierarchy.

    The following method receives a node catalog from the material converter
    and stores that catalog in the material by making a copy of the items
    in the container.
    */
    void setNodeCatalog( const ReNodeDictionary newNodeCatalog );

    //! Parameters:
    //!   texList - Output, the resulting sorted list of textures to be exported
    //!   textures - Input, the list of textures linked to a given material.
    void findDependencies( QStringList& texList ) const;

    //! Called whenever an external entity wants to notify the material that
    //! one of its textures has been modified.
    virtual void textureUpdated( const QString& /* textureName */ ) {
      // nothing
    };

    virtual void serialize( QDataStream& dataStream ) const;

    /**
     The type is not extracted at this point because that piece of data is read by the 
     material deserializer before we call this method.
     */
    virtual void deserialize( QDataStream& dataStream );

    inline const ReGeometryObject* getParent() const {
      return parent;
    }

    inline ReMaterialType getType() const {
      return type;
    }

    inline void setEdited( bool e = true ) {
      edited = e;
    }

    inline bool isEdited() const {
      return edited;
    }
    
    /*
     Method: getVisibleInRender
     */
    inline bool isVisibleInRender() const {
      return visibleInRender;
    };

    /*
     Method: setVisibleInRender
     */
    inline void setVisibleInRender( bool newVal ) {
      visibleInRender = newVal;
    };


    ReTexturePtr& getTexture( const QString& textureID );

    inline ReNodeDictionary getTextures() const {
      return nodeCatalog;
    }

    //! Given a texture this method returns the name of the channel
    //! that references it. 
    //! \return Name of the channel found or and empty string if the
    //!         texture is not found.
    //!         The name is in the following format: 
    //!           - channelName(|subChannelName)*
    //! 
    //!         If the texture is found at the root of a channel then
    //!         the name returned will be something like "Kd", "Bm" and so on
    //!         If the texture is nested inside another texture then the name
    //!         returned will start with the material's channel and it will be
    //!         followed buy the path of texture channels needed to identify the
    //!         texture. 
    //!         For example, the second texture of a Math texture linked to the 
    //!         bump map channel of a material will be returned as "Bm|tex2"
    const QString identifyChannel( const ReTexturePtr tex ) const;

    ReTexturePtr findTextureByChannelPath( const QString channelPath );

    //! Returns a bool indicating whether this material has the channel
    //! channelName.
    inline bool hasChannel( const QString& channelName ) const {
      return channels.contains(channelName);
    }

    const QString getTypeAsString() const ;
  
    static const QString getTypeAsString( const ReMaterialType matType );

    //! Given a name of a material, like "glass" or "glossy", this method
    //! returns the corresponding ReMaterialType
    static ReMaterialType typeFromName( const QString& matName );

    //! Given the abbreviated name of a material, like "GLS" or "GLO", this method
    //! returns the corresponding ReMaterialType
    static ReMaterialType typeFromShortName( const QString& matTypeStr );
  
    virtual QString toString();

    //! Adds a texture to the catalog if it's not already there
    void addTextureToCatalog( ReTexturePtr tex );

    /**
     * Removes a texture from the catalog of textures used by this material.
     * Parameters:
     *   A pointer to the texture to remove
     */ 
    bool removeTextureFromCatalog( ReTexturePtr tex );

    //! Removes all the textures held by this material
    virtual void removeAllTextures();

    //! Removes a texture from the catalog and deletes it from the heap.
    bool deleteTexture( const QString& tex );

    //! Returns whether if a texture is used by this material or not
    //! It check if the texture is linked in the channels and if it's
    //! linked by other textures.
    //! The second, optional parameter, is used to name the owner of the
    //! texture, if known. If this parameter is different than a null 
    //! string then the owner will not be counted in the check. This 
    //! feature allows us to say 
    //! "check if the texture is used by any other texture besides the owner"
    bool textureIsUnused( const QString& texName, const QString ownerTexture = "" ) const;

    //! Creates a new texture and adds it to the <nodeCatalog>. The texture
    //! is created only if the channel name already exists and it does not
    //! contain a texture already
    //! \return true if the operation succeeds, false otherwise.
    bool makeNewTexture( const QString& channelName, 
                         const ReTextureType textureType,
                         const ReTexture::ReTextureDataType dataType = ReTexture::color );

    //! Replaces a texture with JSON data, which can usually come from the 
    //! clipboard. 
    //! \param channelID The channel identifying the texture inside the material
    //! \param jsonTextureData The texture data serialized using JSON
    //! \param masterTesture If the texture to be replaced is a subtexture, then
    //!                      this parameter will point to the master texture. 
    //!                      Otherwise it will be null
    void replaceTexture( const QString channelID, 
                         const QString jsonTextureData, 
                         ReTexturePtr masterTexture = ReTexturePtr() );

    void setChannel( const QString& channelName, const QString& textureName );

    //! Changes a texture from one type to another. 
    ReTexturePtr changeTextureType( const QString& name, const ReTextureType newType );

    virtual void setNamedValue( const QString& vname, const QVariant& value ) ;
    
    virtual const QVariant getNamedValue( const QString& vname ) const;

    /*
     Method: getInnerVolume
     */
    inline const QString getInnerVolume() const {
      return innerVolume;
    };
    /*
     Method: setInnerVolume
     */
    inline void setInnerVolume( QString newVal ) {
      innerVolume = newVal;
    };
    /*
     Method: getOuterVolume
     */
    inline const QString getOuterVolume() const {
      return outerVolume;
    };
    /*
     Method: setOuterVolume
     */
    inline void setOuterVolume( QString newVal ) {
      outerVolume = newVal;
    };

    //! Returns the absorption color of the linked volume, if any
    const QColor& getAbsorptionColor() const;

    //! Returns the scattering color of the linked volume, if any
    const QColor& getScatteringColor() const;

    float getAbsorptionScale() const;
    float getScatteringScale() const;

    float getClarityAtDepth() const;

    //! Communicates with the linked volumes. This method is called by a
    //! linked volume when the volume is renamed by the user. The material
    //! adjusts the inner or outer volume settings accordingly.
    void volumeRenamed( const QString oldName, const QString newName );

    //! Communicates with the linked volumes. This method is called by a
    //! linked volume when the volume is deleted by the user. The material
    //! adjusts the inner or outer volume settings accordingly.
    void unlinkVolume( const QString volumeName );
  
    //! Sets the gamma for a texture. If the texture is of type ImageMap
    //! then the search stops there. Otherwise the method traverses the 
    //! list of all dependencies looking for an <ImageMap> texture.
    void setTextureGamma( ReTexturePtr tex, const float newGamma );

    //! This method computes the ACSEL ID based on the unique properties
    //! of each material's subclass.
    virtual QString computeAcselID( const bool forDefaultShader = false );

    inline const QString& getAcselID() {
      if (acselID.isEmpty()) {
        acselID = computeAcselID();
      }
      return acselID;
    }

    //! Sets the acselID field
    inline void setAcselID( const QString& newID ) {
      acselID = newID;
    }

    inline void setAcselSetName( const QString& acselName ) {
      acselSetName = acselName;
    }

    inline QString& getAcselSetName() {
      return acselSetName;
    }


    //! Sets the AcselSet field
    inline void setAcselSetID( const QString& newVal ) {
      acselSetID = newVal;
    }

    inline const QString& getAcselSetID() const {
      return acselSetID;
    }
};

//! Shared pointer to obtain garbage collection when allocating <ReMaterials> in the heap 
typedef QSharedPointer<ReMaterial> ReMaterialPtr;


} // namespace

#endif
