## \file
# This module exports the Poser geometry to Reality
#
# Reality plug-in
# Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.
# \author Paolo Ciccone
#
# 
# \brief Interface between Poser and the Reality geometry exporter. The
# class in this source exports the Poser geometry into the Reality's
# intermediate storage. From there the Reality standard exporter outputs
# the geometry in the LuxRender format.
#
import poser, traceback, sys, array, wx, os
import collections
import math
import ReTools
import struct

# import time
RealityActive = False
try:
    import Reality
    RealityActive = True
except Exception, e:
    wx.MessageBox(
        """
        Could not load the Reality plugin!
        You are running Poser at %s bits. 
        Make sure that you have installed Reality at %s bits as well. 
        """ % (poser.Bitness(), poser.Bitness())
    )

RealitySceneData = None
if RealityActive:
    RealitySceneData = Reality.Scene()

##
# Lightweight storage class for Actor data passed around during the
# conversion. 
class ReActorData:
  name = ""
  geometry = None
  subdivided = False
  isAreaLight = False

UTF8="utf-8"
REALITY_ID="RealityID"
##
# The Geometry exporter
#
class ReGeometryExporter:

  def __init__(self, Globals):
    self.Globals = Globals
    self.materials = {}
    # Establishes if this version of Poser supports subdivision
    self.hasSubdivision = ReTools.POSER_MAJOR > 9
    self.RealitySceneData = Reality.Scene()

  def exportGeometry(self):
    try:
      numObjects = self.RealitySceneData.getNumObjects()

      progDlg = wx.ProgressDialog(title="Reality", message="Exporting scene to LuxRender", maximum=numObjects)
      poser.Scene().ProcessSomeEvents(1)
      progressValue = 0

      Reality.exportStarted(numObjects)
      allActors = []
      # Find all the actors that are Reality objects to be exported
      for act in poser.Scene().Actors() + poser.Scene().Figures():
        # Reality.writeToLog("Scanning object %s" % act.Name())
        rid = act.CustomData(REALITY_ID)
        if rid:
          allActors.append(act)
        # else:
        #   Reality.writeToLog("  object has no Reality ID")


      for actor in allActors:
        # Reality.writeToLog("Scanning object %s" % actor.Name())
        objectName = actor.CustomData(REALITY_ID)
        if not actor.IsLight() and not actor.IsCamera() and not objectName:
          Reality.writeToLog("No reality ID")
          continue

        progressValue += 1
        msg = "Exporting %s ..." % self.RealitySceneData.getObjectName(
          objectName
        )
        Reality.exportingObject(msg)
        Reality.writeToLog(msg)
        progDlg.Update(progressValue, msg)
        poser.Scene().ProcessSomeEvents(1)
        self.exportObject(actor)

      progDlg.Destroy()
      Reality.exportFinished()

    except Exception as e:
      traceback.print_exc(file=sys.stdout)  
      if progDlg:
        progDlg.Destroy()
        Reality.exportFinished()

  # The matrix is a tuple that contains four tuples, one for each row
  # of the 4x4 matrix. Each sub-tuple contains 4 floats.
  # The format is, as expected:
  #  (
  #    (x1, x2, x3, 0)
  #    (y1, y2, y3, 0) 
  #    (z1, z2, z3, 0)
  #    (xt, yt, zt, 1)
  #  )
  def multiplyVertex(self, vertex, matrix):
    newVertex = [0, 0, 0]
    
    vx = vertex[0]
    vy = vertex[1]
    vz = vertex[2]

    m1 = matrix[0]
    m2 = matrix[1]
    m3 = matrix[2]
    m4 = matrix[3]

    newVertex[0] = vx*m1[0]+vy*m2[0]+vz*m3[0]+m4[0]
    newVertex[1] = vx*m1[1]+vy*m2[1]+vz*m3[1]+m4[1]
    newVertex[2] = vx*m1[2]+vy*m2[2]+vz*m3[2]+m4[2]
    return newVertex

  ##
  # Creates a simple quad to act as a mesh light. This is to implement support
  # for Poser's own Area light since the geometry provided by the Actor itself
  # is not usable for our purposes.
  def createHostAreaLight(self, obj):
    m = obj.WorldMatrix()
    # According to SMI the original size of the are alight is 0.1 PNU
    sideSize = 0.1
    halfSide = sideSize/2
    # Vertices of the Area light. We define a square standing up
    # The square measure a meter per side and it's centered at the origin.
    # The vertices are given in a counter-clockwise sequence starting from
    # the lower-left corner.
    verts = [ [-halfSide, -halfSide, 0], [ halfSide, -halfSide, 0], 
              [ halfSide,  halfSide, 0], [-halfSide,  halfSide, 0]]

    # List of vertices after applying the transform matrix. The geometry export
    # services expect the lists to be flat. 
    newVerts = []
    for x in range(len(verts)):
      newVerts.extend(self.multiplyVertex(verts[x], m))
    matData = self.materials["Preview"]
    matData["v"] = newVerts
    matData["i"] = [0,1,2,  0,2,3]
    matData["n"] = [0,0,-1,  0,0,-1,  0,0,-1,  0,0,-1]
    matData["uv"] = [0, 0.0, 0.0,  1, 1.0, 0.0, 
                     2, 1.0, 1.0,  3, 0.0, 1.0]

  ##
  #  Pre-scan the geometry to obtain the number of vertices, triangles and UVs needed to store the
  #  data about each material
  def gatherGeometryInfo(self, obj, matNames):
    # Create the material map. Test if the object passed is a single actor
    # or a figure. If it is the latter then we fill the actors list with
    # all the geometry-based actors of the figure
    actors = []

    # List of object/materials for which we encountered problems
    # If this event happens we want to report the problem in the log but
    # we don't want to output the same message multiple times
    exportExceptions = {}
    if isinstance(obj, poser.FigureType):
      # Skip figures that are flagged as invisible
      if not obj.Visible():
        return False
      done = False
      try:
        dsonSubd = obj.Actor("__SUBD")
      except:
        dsonSubd = False

      # Check if this is Poser 10 or higher
      if self.hasSubdivision:
        oneActor = ReActorData()
        oneActor.name = obj.CustomData(REALITY_ID)
        # If the skin is not of type Poser (old Poser) then it's subdivided
        if obj.SkinType() != poser.kSkinTypePOSER:
          # Geometry
          oneActor.geometry = obj.SubdivGeometry()
          # Subdivided flag
          oneActor.subdivided = True
          actors.append(oneActor)
          done = True
        elif dsonSubd:
            # Geometry
            oneActor.geometry = dsonSubd.Geometry()
            # Subdivided flag
            oneActor.subdivided = False
            actors.append(oneActor)
            done = True

      if not done:
        # This is an old-style figure so we add all the actors that it contains
        for actor in obj.Actors():
          if (not (actor.Visible() and actor.VisibleInCamera())): 
            # Parented props are not necessary because they are processed independently
            continue
          if actor.IsProp():
            # If the actor doesn't have any geometry then it's of no interest to us
            continue
          if actor.Geometry():
            oneActor = ReActorData()
            oneActor.name = actor.InternalName()
            objID = actor.CustomData(REALITY_ID)
            if objID:
              oneActor.name = objID
            oneActor.geometry = actor.Geometry()
            oneActor.subdivided = False
            actors.append(oneActor)
    else:
      # This is a prop
      if (not (obj.Visible() and obj.VisibleInCamera())):
        return False
      oneActor = ReActorData()
      oneActor.name = obj.CustomData(REALITY_ID)
      if self.hasSubdivision and obj.NumbSubdivLevels() > 0:
        oneActor.geometry = obj.SubdivGeometry()
        oneActor.subdivided = True
      else:
        oneActor.geometry = obj.Geometry()
        oneActor.subdivided = False
      oneActor.isAreaLight = (obj.IsLight() and obj.LightType() == ReTools.AreaLightType)

      actors.append(oneActor)


    Reality.writeToLog("Gathering geometry data for %s" % oneActor.name)
    self.materials = {}
    for matName in matNames:
      self.materials[matName] = { 
        "v" : [], # The list of vertices (x, y, z)
        "i": [],  # The list of triangles (v1, v2, v3)
        "n" : [], # The list of normals (x, y, z)
        "uv": [], # The list of uv points (pointer to uv vertex, index of corresponding vertex)
        "t": {},  # Temporary vertex re-numbering table
        "vi": -1  # Temporary counter of newt available vertex index
      }
    ##
    # Organization of the geometry in Poser. 
    # In Poser the Geometry() method of the Actor returns a reference to
    # the geometric information.
    #
    # The Geometry.WorldVertices() method returns the list of vertices that 
    # form a mesh transformed with world coordinates.
    # The Geometry.TexVertices() returns the corresponding list of UV points
    # for the vertices.
    #
    # The Geometry.Polygons() returns the list of defined polys. Each poly has
    # properties that describe how many vertices it has and the first index in 
    # the array Geometry.Sets() that defines the polygon. For the sake of 
    # simplicity we are going to assume that each poly is a quad
    #
    # A "set" is simply an ordered array that contains the list of indices inside
    # the array of vertices. Once we know the start of the sequence inside the
    # set we just need to read a number of indices equal to the number of vertices
    # that the polygon has
    #
    # The following diagram represents the data organization:
    #
    # \dot
    # digraph poserGeom {
    #   node [shape=record, fontname=Trebuchet, fontsize=10];
    #   Poly [label="<f0> start index | <f1> num verts (4)"];
    #   subgraph "cluster_PolyLabel" {label="Polygon"; Poly};
    #
    #   Set [label="<f5> ...| <f0> index 1| <f1> index 2 | <f2> index 3 | <f3> index 4 | <f6> ..."];
    #   subgraph "cluster_SetLabel" {label="Set"; Set}
    #
    #   Verts [label="<f5> ...|<f0> 0.5, 1.3, 4.7| <f1> 1.56, 0.67, 3.1 | <f2> 12.34, 1.8, 45.0 | <f3> 5.21, 67.2, 9.8 | <f6> ..."];
    #   subgraph "cluster_VertsLabel" {label="Vertices"; Verts}
    #   Poly:f0 -> Set:f0;
    #
    #   Set:f0 -> Verts:f1;
    #   Set:f1 -> Verts:f0;
    #   Set:f2 -> Verts:f2;
    #   Set:f3 -> Verts:f3;
    # }
    # \enddot
    #

    for actor in actors:
      actorName = actor.name

      if actor.isAreaLight:
        Reality.writeToLog("Found host's area light %s" % actorName)
        self.createHostAreaLight(obj)        
        continue

      geom = actor.geometry
          
      if not geom:
        Reality.writeToLog("Error in exporting: no geometry available for %s" % (actorName))
        continue

      vertices = geom.WorldVertices()
      if (vertices == None):
        vertices = geom.Vertices()

      normals  = geom.WorldNormals()
      if (normals == None):
        normals = geom.Normals()

      texVerts = geom.TexVertices() 

      sets    = geom.Sets()
      polys   = geom.Polygons()
      uvSets  = geom.TexSets()
      uvPolys = geom.TexPolygons()

      hasUVs = True
      if not uvSets or len(uvSets) == 0:
        hasUVs = False

      if not polys:
        Reality.writeToLog("No polygons for actor %s" % actorName)
        continue
      # Scan the list of polygons and index them
      for (pn,poly) in enumerate(polys):
        matName = poly.MaterialName()
        if not matName in self.materials:
          Reality.writeToLog("Skipping material %s in %s" % (matName, actorName))
          continue
        # Avoid Python's expensive lookup
        matRef = self.materials[matName]
        matRef_t = matRef["t"]

        if not actorName in matRef_t:
          matRef_t[actorName] = {}

        numVertices = poly.NumVertices()
        # Get the first entry in the list of indices to vertices belonging
        # to this poly
        indexStart = poly.Start()

        # If the Actor has UVs then we test if the polygon has UVs
        if hasUVs:
          # It can happen that one or more polys are not included in the UV map
          try:
            polyHasUVs = uvPolys[pn].NumTexVertices() > 0
            if polyHasUVs:
              uvIndexStart = uvPolys[pn].Start()
          except:
            polyHasUVs = False
            exceptionKey = (actorName,matName)
            if not exceptionKey in exportExceptions :
              Reality.writeToLog("Exception is ReGeometryExport.py: object %s, mat: %s has wrong UVs" % (actorName, matName))
              exportExceptions[exceptionKey] = True
        else:
          polyHasUVs = False

        # Temporary list of vertex indices used for triangulation
        indexList = []
        # Here we build the lists of vertices and faces based on what LuxRender
        # needs. The geometry is grouped by material, so we rebuild our list
        # of vertices based on the new order. The polys are then set to use
        # the new list of vertices.
        #
        # The version of the algo that deals with UVs needs to create a list of
        # vertices that is equal to the number of UV entries. In the case of closed
        # geometry where multiple UV point are assigned to the same vertex, because
        # of the seam, the vertex needs to be listed multiple time in the Lux 
        # output. The vertexKey variable is used to keep track of the unique 
        # combinations of vertex and UV.
        #
        # Duplicated algo to avoid millions of tests on the UV
        if polyHasUVs:
          for v in range(numVertices):
            vertexIndex = sets[indexStart+v]

            uvIndex = uvSets[uvIndexStart+v]
            # The vertex key is a unique combination of a vertex index and the 
            # UV index. When dealing with seams the same vertex index will be
            # in two keys because the UV index is different. In this way we are
            # guaranteed to output the vertex twice, as required by Lux
            vertexKey = (vertexIndex, uvIndex)

            # If the key exists already it means that we are dealing with a 
            # shared vertex, in that case we retrieve the vertex index to 
            # add it to the indexes used by the current polygone
            if vertexKey in matRef_t[actorName]:
              reVertexIndex = matRef_t[actorName][vertexKey]
            else:
              # If the vertex key does not exist then we add the vertex to the
              # vertex tab and add the new index to the indexes used by the 
              # polygon
              matRef["vi"] += 1
              matRef_t[actorName][vertexKey] = matRef["vi"]
              reVertexIndex = matRef["vi"]
              vert = vertices[vertexIndex]
              norm = normals[vertexIndex]
              matRef["v"].extend( [vert.X(), vert.Y(), vert.Z()] )
              matRef["n"].extend( [norm.X(), norm.Y(), norm.Z()] )

            # Add the vertex index to the list of vertices used to define this poly
            # This is triangulated later
            indexList.append(reVertexIndex)

            # Store the UV point coordinates together with the index of 
            # the vertex they corresponds to.
            uvp = texVerts[uvIndex]
            matRef["uv"].extend( [reVertexIndex, uvp.U(), uvp.V() ] )
        else:
          for v in range(numVertices):
            vertexIndex = sets[indexStart+v]

            if vertexIndex in matRef_t[actorName]:
              reVertexIndex = matRef_t[actorName][vertexIndex]
            else:
              matRef["vi"] += 1
              matRef_t[actorName][vertexIndex] = matRef["vi"]
              reVertexIndex = matRef["vi"]
              vert = vertices[vertexIndex]
              norm = normals[vertexIndex]
              matRef["v"].extend( [vert.X(), vert.Y(), vert.Z()] )
              matRef["n"].extend( [norm.X(), norm.Y(), norm.Z()] )

            # Add the vertex to the list of vertices used to define this poly
            # This is triangulated below
            indexList.append(reVertexIndex)

        """
        Triangulation
        """
        # Most common case is to read quads. With this initialization we avoid a test and
        # assignment for the most frequent case.
        numTris  = 2
        if numVertices == 3:
          numTris = 1
        else:
          numTris = numVertices-2
        for i in range(numTris):
          matRef["i"].extend( [indexList[0],indexList[i+1],indexList[i+2]] )

    return True

  def exportObject(self, obj):      
    objName = obj.CustomData(REALITY_ID)

    # Skip the components of a hair prop because otherwise we will render the
    # blocky geometry of those sub-props. Instead we have a separate routine
    # to handle the strand hair. See below
    if obj.IsHairProp():
      return;

    # Get the list of material names associated with the object/figure
    try:
      matNames = self.RealitySceneData.getMaterialNames(objName)
    except Exception as e:
      print "No materials for object %s. Reason: %s" % (objName, e.message)
      return

    # Is this a dynamic hair prop?
    numHairGroups = 0 

    if isinstance(obj, poser.ActorType):
      numHairGroups = obj.NumHairGroups()

    if numHairGroups > 0:
      Reality.writeToLog("%s is a dynamic hair prop with %d groups" % (objName, numHairGroups))
      for i in range(numHairGroups):
        self.exportHairGroup(obj.HairGroup(i))
    else:
      # Collect all the vital information about this actor/object
      if not self.gatherGeometryInfo(obj, matNames):
        return

      self.RealitySceneData.renderSceneObjectBegin( objName )

      # Export all the polygons related to each material. One material at the time
      for matName in self.materials.keys():
        # Skip materials that have been hidden by the user
        if not self.RealitySceneData.isMaterialVisible(objName, matName):
          continue

        mat = self.materials[matName]
        hasUVs = len(mat["uv"]) > 0
        #!!! What happens if there are no UVs?
        self.RealitySceneData.newGeometryBuffer( 
          matName,
          len(self.materials[matName]["v"])/3,
          len(self.materials[matName]["i"])/3,
          hasUVs
        )
        # We have UV maps for this material
        if hasUVs:
          self.RealitySceneData.copyUVData(mat["uv"], mat["v"], mat["n"])
        else:
          self.RealitySceneData.copyVertexData(mat["v"], mat["n"])

        # Export the polygon definition
        self.RealitySceneData.copyPolygonData(mat["i"])

        shapeName = "%s::%s" % (objName, matName)
         
        self.RealitySceneData.renderSceneExportMaterial(
          matName, objName, shapeName 
        ) 
        # Free memory
        del self.materials[matName]

      self.RealitySceneData.renderSceneObjectEnd( objName )

  def writeHairFileHeader(self, fileHandle, numVerts, numStrands, numSegments ):
    headerCustomDataLength = 88

    # Writing the header for the hair file

    # ------------- Start of header -------------  
    # The start of the file must be 'HAIR'
    fileHandle.write(b'\x48\x41\x49\x52')

    # Number of hair strands, uint
    fileHandle.write(struct.pack('I', numStrands))

    # Total number of points
    fileHandle.write(struct.pack('I', numVerts))

    # Flags, from byte 12 to 15. As follows:
    # - bit 0: whether the file has a segments array
    # - bit 1: whether the file has a points array. This is mandatory and so must be 1
    # - bit 2: whether the file has a thickness array
    # - bit 3: whether the file has a transparency array
    # - bit 4: whether the file has a color array
    # - bits 5 to 31: reserved and must be 0
    # 
    # Our configuration: 00110 => 6 decimal
    fileHandle.write(struct.pack('I', 6))

    # Default number of segments
    fileHandle.write(struct.pack('I', numSegments))

    # Default thickness
    fileHandle.write(struct.pack('f', 0.001))

    # Default transparency
    fileHandle.write(struct.pack('f', 0.01))

    # Default RGB value
    fileHandle.write(struct.pack('f', 1.0)) # r
    fileHandle.write(struct.pack('f', 1.0)) # g
    fileHandle.write(struct.pack('f', 1.0)) # b

    # Add the name in the custom data area as a way of documenting the origin,
    # padded to fill 88 characters, as per cyHair spec
    fileHandle.write("RealityHair".ljust(headerCustomDataLength))
    # ------------- End of header -------------  

  ##----------------------------------------------------------------------------
  # Poser to Lux hair strand exporter. The hair is exported using the Cem Yuksel
  # format. See http://www.cemyuksel.com/research/hairmodels/
  # 
  # From contribution by Eugene Cherepanov bpdozer@mail.ru 
  #  
  #-----------------------------------------------------------------------------
  def exportHairGroup(self, hairGroup):
    # Save the state of the hair group
    populatedState = hairGroup.ShowPopulated();
    # Enable the population of the hair group so that the whole geometry
    # will be available for export. Otherwise only the guide strands will be
    # returned
    hairGroup.SetShowPopulated(1)
    hairProp = hairGroup.HairProp()
    poser.Scene().Draw()
    poser.Scene().ProcessSomeEvents(20)

    objID = hairProp.CustomData(REALITY_ID)
    objectPath = RealitySceneData.getSceneResourceObjectPath()   

    # Conversion from Poser Native Units to metric 1PNU = 262.128cm
    scale = 2.621280116332
    vertex = hairProp.Geometry().Vertices()

    numVertices = hairProp.Geometry().NumVertices()
    numStrands  = hairProp.Geometry().NumPolygons()
    # Number of segments per strand
    numSegments = hairGroup.NumbVertsPerHair()-1 #hairProp.Geometry().Polygon(0).NumVertices()-1

    Reality.writeToLog("Dynhair: %s. Strands: %d. Segments: %d" % (objID, numStrands, numSegments))

    fileName = os.path.join(objectPath,objID) +  "-Hair.hair"
    try:
      f = open(fileName,'wb')
    except Exception, e:
      print("Error: cannot open hair file %s for writing" % fileName)
      hairGroup.SetShowPopulated(populatedState)
      return

    self.writeHairFileHeader(f, numVertices, numStrands, numSegments)

    polys = hairProp.Geometry().Polygons()
    # Points array
    for aPoly in polys:
      for aVert in aPoly.Vertices():
        f.write(struct.pack('fff', 
                            aVert.X()*scale, 
                            -aVert.Z()*scale, 
                            aVert.Y()*scale))

    # Thickness array
    thickNessCoeff = 2000
    knots     = hairGroup.NumbVertsPerHair()
    rootWidth = hairGroup.RootWidth() / thickNessCoeff
    tipWidth  = hairGroup.TipWidth() / thickNessCoeff

    for exPoly in polys:
      x=0
      for exVert in exPoly.Vertices():
        curTh = rootWidth-(rootWidth-tipWidth)/knots*x
        x=x+1
        f.write(struct.pack('f', curTh))

    # # Transparency array
    # for exPoly in polys:
    #   x=0.0
    #   for exVert in exPoly.Vertices():
    #     f.write(struct.pack('f', x))
    #     x = x+(0.3/knots)   

    f.close()
    # Write the reference to the file in the scene
     # Turn the path to a relative one based on the scene file location
    fileRelName = RealitySceneData.getSceneResourceRelativePath(fileName)    

    RealitySceneData.renderSceneCustomData("AttributeBegin\n")    
    hairMatName = Reality.sanitizeMaterialName("%s:Hair" % objID)
    RealitySceneData.renderSceneCustomData("NamedMaterial \"%s\"\n" % hairMatName)
    RealitySceneData.renderSceneCustomData(
      "Shape \"hairfile\" \"string filename\" [\"%s\"] " % fileRelName
    )
    RealitySceneData.renderSceneCustomData(
      "\"string tesseltype\" [\"ribbonadaptive\"] \"float adaptive_error\" [0.01] "
    )
    RealitySceneData.renderSceneCustomData(
      "\"point camerapos\" [%f %f %f]\n" % tuple(RealitySceneData.getRenderCameraPosition())
    )
    RealitySceneData.renderSceneCustomData("AttributeEnd\n")
    hairGroup.SetShowPopulated(populatedState)

