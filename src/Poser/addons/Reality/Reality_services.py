#
# Reality API
# Copyright (c) 2012 Pret-a-3D/Paolo Ciccone. All rights reserved
#

# ------------------------------------------
#             The Reality API
# ------------------------------------------

import sys,os, poser, json, wx

Reality_IBL_Sphere_ID = "RealityIBLSphere"
Reality_IBL_ID = "IBL"
Reality_IBL_Sphere_Material = "RealityIBLSphere"

RealityPath = ""

def ReSetIBLPreviewMap( previewMap ):
  """
  Set the IBL preview map on the Reality IBL Preview Sphere
  If the sphere is not in the Poser scene then it's added.
  If it is already present then the map for the sphere is updated to
  use the new map passed in the paramter for this function
  """
  scene = poser.Scene()

  # Check if the preview sphere is already in the scene
  try:
    previewSphere = scene.ActorByInternalName(Reality_IBL_Sphere_ID);
  except:
    scene.LoadLibraryProp(":Runtime:Libraries:Props:Reality:Reality IBL Sphere.pp2")

  # Get a handle on the preview sphere
  try:
    previewSphere = scene.ActorByInternalName(Reality_IBL_Sphere_ID)
  except:
    # If, for any reason, the sphere is not in the scene let the user know and quit
    wx.MessageBox("Information", "It was not possible to set the IBL preview sphere. Operation stopped.")
    return

  mat = previewSphere.Material(Reality_IBL_Sphere_Material)
  shaderTree = mat.ShaderTree()
  # Check if the map node already exists
  poserSurface = shaderTree.Node(0)
  diffuse = poserSurface.InputByInternalName("Diffuse_Color")
  mapNode = diffuse.InNode()  

  # If there is no image map node then create one
  if not mapNode:
    mapNode = shaderTree.CreateNode(poser.kNodeTypeCodeIMAGEMAP)
    mapNode.ConnectToInput(diffuse)

  # Set the map
  mapNode.InputByInternalName("Image_Source").SetString(previewMap)

# def ReSetIBLMap( iblMap ):
#   """
#   Set the IBL map used by Reality. This automatically turn on the IBL light
#   if it was off before.
#   """
#   lightData = {}
#   lightData["lightID"]      = Reality_IBL_ID
#   lightData["name"]         = Reality_IBL_ID
#   lightData["on"]           = True
#   lightData["IBLMapFormat"] = "latlong";
#   lightData["color"]        = {"color": (1.0, 1.0, 1.0), "map": ""}
#   lightData["color"]["map"] = iblMap
#   lightData["angle"]        = 0
#   lightData["intensity"]    = 1.0

#   Reality.Scene().updateLight(lightData["lightID"].encode("utf8"), 
#                               lightData)

def ReResolvePoserPath( poserFileName ):
  pathSep = "/"
  if sys.platform == 'win32':
    pathSep = '\\'

  poserFileName = poserFileName.replace(":", pathSep)
  for libName in poser.Libraries():
    fullFileName = pathSep.join([libName, poserFileName])
    if os.path.exists(fullFileName):
      return fullFileName
  return ""


def initRealityPath():
  global RealityPath

  RealityPath = os.path.dirname(__file__)
  # So that other Python scripts can import the Reality module
  sys.path.append(RealityPath)

initRealityPath()
