/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RENODECONVERTER_H
#define RENODECONVERTER_H

#include <QVariantMap>

#include "ReTexture.h"


namespace Reality {

/**
 * Converts a node from the the host importer to a ReTexture
 */
class ReNodeConverter {
public:
  // Enum: NodeHint
  //   Used to suggest what kind of node is expected. This is an optional
  //   parameters for the <convertNode> method.
  enum NodeHint {
    NumericNode,
    ColorNode,
    NoHint
  };

  enum nodeTypes {
    Band           = 100,
    Bricks         = 110,
    Checkers       = 120,
    Clouds         = 130,
    Constant       = 140,
    Dots           = 150,
    FBM            = 160,
    FresnelColor   = 165,
    ImageMap       = 170,
    Marble         = 180,
    Mix            = 190,
    MultiMix       = 200,
    DistortedNoise = 225,
    ColorMath      = 230,
    Math           = 235,
    Wrinkled       = 240,
    Component      = 250,
    Wood           = 111
  };

private:

  static QVariantMap nodes;

  /**
   All nodes for the material. 

   Nodes, aka Textures, can be linked together to form a shader tree. Several nodes
   can be linked to the same node. This prevents the possibility to nest nodes together
   as we would end up with duplicates of the shared nodes. To avoid this situation a
   pool of nodes is created. All nodes are simply added to the pool and addressed by
   their label/name. When a primary node, a node that is linked directly by one of the
   channels of the material, links to another node we simply take the pointer to the
   linked node from the pool and add it to the node that links to it. In this way we
   convert a given node only once and we then re-use it. This effectively implements
   the node system as seen in Poser but it allows us to use it inside the more clear 
   and intuitive "Outline-like" UI of Reality.

   For ReGeometryObject the nodeCatalog is only transient. Each material holds its own
   node catalog. The one employed in this class is only used to store the node network
   while the material is getting converted. The node network is then stored in the 
   material and this class' copy is erased at the beginning of each conversion.

   */
  static ReNodeDictionary nodeCatalog;

  static ReTexturePtr convertBand(const QString& nodeName, const QVariantMap& node);
  static ReTexturePtr convertClouds(const QString& nodeName, const QVariantMap& node, const NodeHint hint = NumericNode);
  static ReTexturePtr convertFBM(const QString& nodeName, const QVariantMap& node, const NodeHint hint = NumericNode);
  static ReTexturePtr convertWood(const QString& nodeName, const QVariantMap& node, const NodeHint hint = NumericNode);
  static ReTexturePtr convertColorMath( const QString& nodeName, 
                                        const QVariantMap& node,
                                        const NodeHint hint = NoHint,
                                        const bool makeUnique = false );
  static ReTexturePtr convertComponent( const QString& nodeName, 
                                        const QVariantMap& node,
                                        const NodeHint hint = NoHint );
  static ReTexturePtr convertConstant(const QString& nodeName, const QVariantMap& node);
  static ReTexturePtr convertDistortedNoise(const QString& nodeName, const QVariantMap& node, const NodeHint hint = NumericNode);
  static ReTexturePtr convertFresnelColor(const QString& nodeName, const QVariantMap& node);
  static ReTexturePtr convertImageMap(const QString& nodeName, 
                                      const QVariantMap& node, 
                                      const NodeHint hint = NoHint);

  //! A math node should not need any hint but the reality is that Poser treats grayscale and 
  //! color nodes in the same way so we can have a source Poser node of type Math to be
  //! the input for a color channel. In that case the method simply scans if it has a texture
  //! in one of its channels and if it's so it returns an ImageMap texture set to grayscale because
  //! that was the reason why the Poser Math node was used. Math nodes are often used as a simple
  //! way of turning an image map into a grayscale version.
  static ReTexturePtr convertMath( const QString& nodeName, 
                                   const QVariantMap& node, 
                                   const NodeHint hint = NoHint,
                                   const bool makeUnique = false );
  static ReTexturePtr convertMix( const QString& nodeName, 
                                  const QVariantMap& node, 
                                  const NodeHint hint = NoHint,
                                  const bool makeUnique = false );

public:
  // Constructor: ReNodeConverter
  ReNodeConverter() {
  };
  // Destructor: ReNodeConverter
 ~ReNodeConverter() {
  };

  /**
   Reset the state of the converter. This method must be called at the beginning of
   a material conversion.  
   */
  static void init( const QVariantMap& nodes );

  /**
   Converts the node network received from the hosting application to 
   a network of ReTexture. The method works in recursive mode so that
   it can traverse the whole node network (tree) starting form the first
   node and return the whole network in one recursive iteration.

   Parameters:
     nodeName - The QString that contains the name of the node that we are converting
     hint  - A flag that indicates if the converted node must be of type color or numeric.
     makeUnique - A flag that indicates if the node returned must be unique. In case of textures
                  that are shared among several channels this flag can force the method
                  to create a copy of the node so that it is not affected by changes done
                  when the same texture is edited as the input for another channel.
   */            
  static ReTexturePtr convertNode( const QString nodeName, const NodeHint hint = NoHint, const bool makeUnique = false );

  static void addNode( ReTexturePtr node );

  static ReNodeDictionary getNodeCatalog();
};


} // namespace

#endif
