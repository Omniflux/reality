#
# Reality plug-in
# Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.
#

import logging, os.path, poser, re, sys,traceback;

"""
General Reality utility functions
"""

# Poser Light type indentifiers
SpotLightType     = 1
IBLLightType      = 3
InfiniteLightType = 0
AreaLightType     = 4
RealityIBLType    = 5

# Useful constants for recognizing the Poser environment
(_POSER_MAJOR_S, _POSER_MINOR_S, _POSER_PATCH_NUMBER_S, _POSER_BUILD_NUMBER_S) = poser.AppVersion().split(".")
POSER_MAJOR = int(_POSER_MAJOR_S)
POSER_MINOR = int(_POSER_MINOR_S)
POSER_PATCH_NUMBER = int(_POSER_PATCH_NUMBER_S)
POSER_BUILD_NUMBER = int(_POSER_BUILD_NUMBER_S)

"""
Creates a Global Unique ID for a material based on the owner, the object
that holds the material, and the material name
"""
def createMatGUID(ownerName,matName):
  return "%s::%s" % (ownerName,matName)

DEBUG_ON = False
def debug(msg):
    if DEBUG_ON:
      logging.basicConfig(filename=os.path.expanduser('~/Reality_debug.txt'),level=logging.DEBUG)
      logging.debug(msg)
      # print msg


def getWeldingList(figure):
  weldingList = {}
  for actor in figure.Actors():
    if not actor.Geometry() or actor.IsDeformer() or actor.IsBase() or actor.IsZone():
      continue
    aName = actor.Name()    
    if aName == "GoalCenterOfMass" or aName == "CenterOfMass":
      continue

    weldingList.setdefault(aName, [])
    for goalActor in actor.WeldGoalActors():
      weldingList.setdefault(goalActor.Name(), []).append(aName)
  
  return weldingList

def getNumVertices(obj):
  # This is some internal object used by Poser when replacing an object with 
  # another one
  if obj.Name() == "__refRepose__":
    return 0

  try:
    if not isinstance(obj, poser.FigureType):
      return obj.Geometry().NumVertices()

    # Every DSON-imported figure has an actor named __SUBD 
    try:
      subd = obj.Actor("__SUBD")
    except:
      subd = False

    # If we found __SUBD then we need to look at the number of vertices
    # of the __SUBD actor without the subdivision. To do so we need
    # to set the Subdivision to 0 and read the numbe rof vertices.
    # After thaty is done we set the subdivision back to what it was.
    # We cannot simply obtain the number of vertices using a division
    # because DSON doesn't work with a simply multiple of 4 when 
    # changing the subdivision level.
    if subd:
      # We find the level of subdivision in the Body actor
      body = obj.ParentActor()
      subdParam = None
      # Just being very cautious
      try:
        subdParam = body.Parameter("SubDivision Level")
      except:
        pass

      if subdParam:
        subdLevel = subdParam.Value()
        subdParam.SetValue(0)
        numVerts = subd.Geometry().NumVertices()
        subdParam.SetValue(subdLevel)

        return numVerts

    # Find the to total number of vertices in the figure, including,
    # for Poser 9, the vertices that are counted twice at the welding 
    # points.
    try:      
      unimeshInfo = obj.UnimeshInfo()
      numVerts = unimeshInfo[0].NumVertices()
    except:
      # We are probably in the Setup room, there is no geometry
      # at this point.
      numVerts = 0

    # For Poser 10 and higher there can be multiple skinning methods for
    # figures. If a figure has been designed for the weight-mapping
    # rigging then the number of vertices returned is all that is needed
    # because there are no individual bones that need to be welded. The
    # weight-mapped skin is just a single mesh.
    # If a figure uses the old bones-based rigging then the function needs
    # to subtract the welded vertices, operation that is done after getting
    # the overall number of vertices in the figure.
    if POSER_MAJOR > 9:
      if obj.SkinType() != poser.kSkinTypePOSER:
        return numVerts

    # Get the list of actors in the figure that have welding edges
    weldingDict = getWeldingList(obj)

    # Traverse the list of weld actors that are welded to the each actor
    # and subtract the number of welded vertices from the total. This
    # gives us the exact number of vertices in the figure. The dictionary
    # is organzed as:
    # actorName => [weldActName1, weldActName2...weldActNameN]
    for weldActors in weldingDict.itervalues():
      for wActorName in weldActors:
        wActor = obj.Actor(wActorName)
        # WeldGoals is a list of lists
        for weldGoal in wActor.WeldGoals():
          for weldVertIdx in weldGoal:
            if weldVertIdx != -1:
              numVerts -= 1


    return numVerts;    
  except Exception, e:
    print "Error in getNumVertices(\"%s\"): %s" % (obj.Name(),e.message)
    traceback.print_stack()

  return 0

##
# Given a Poser object it returns a unique id for its geometry based on the
# geometry file name plus the number of vertices, separated by
# an underscore
#
def getObjectGeometryID(obj):
    gfName = obj.GeomFileName()
    if not gfName:
        fName = "noFile_%s" % obj.Name()
    else:
        fName = os.path.splitext(os.path.basename(gfName))[0]
    # Fixing the geometry name for the DSON-imported models.
    # Such objects have a UUID added to the name of the file.
    # We strip that UUID because it does not match how the same
    # name is found in DAZ Studio.
    # 
    # Regexp: a group of hexadecimal digits followed by an 
    #         optional "-". The group of digits is between 4 and
    #         12 characters and can be repeated exactly 5 times.
    #         This sequence is enclosed between curly braces
    # Example: {d327347a-b64f-fe84-ca60-38c7d7f9dad3}
    fName = re.sub(r"\{([0-9a-f]{4,12}-*){5}\}$", "", fName)
    return ("%s_%d" % (fName, getNumVertices(obj)))

