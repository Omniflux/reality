#-----------------------------------------------------------------------------
# This was a draft of a strand-based hair exporter for Poser. It has been 
# menrged into ReGeometryExporter.py but I'm keeping this around for a little
# longer. It's an adaptatoion of Hairs.py by Eugene Cherepanov, which is also
# included in the same directory.
#
# Poser to Lux hair strand exporter. The export is based on the Cem Yuksel
# format. See http://www.cemyuksel.com/research/hairmodels/
#  Reality plug-in
#  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved. 
# 
# From contribution by Eugene Cherepanov bpdozer@mail.ru 
#  
#-----------------------------------------------------------------------------

import poser
import os
import struct
import random

def writeFileHeader( fileHandle, numVerts, numPolys, numStrands ):
  headerCustomDataLength = 88

  # Writing the header for the hair file

  # ------------- Start of header -------------  
  # The start of the file must be 'HAIR'
  f.write(b'\x48\x41\x49\x52')

  # Number of hair strands, uint
  f.write(struct.pack('I', numPolys)

  # Total number of points
  f.write(struct.pack('I', numVerts)

  # Flags, from byt 12 to 15. As follows:
  # - bit 0: whether the file has a segments array
  # - bit 1: wheter the file has a points array. This is mandatory and so must be 1
  # - bit 2: wheter the file has a thickness array
  # - bit 3: wheter the file has a transparency array
  # - bit 4: wheter the file has a color array
  # - bits 5 to 31: reserved and must be 0
  # 
  # Our configuration: 01110 => 14 decimal
  f.write(struct.pack('I', 14))

  # Default number of strands
  f.write(struct.pack('I', numStrands)

  # Default thickness
  f.write(struct.pack('f', 0.001))

  # Default transparency
  f.write(struct.pack('f', 0.01))

  # Default RGB value
  f.write(struct.pack('f', 0.05)) # r
  f.write(struct.pack('f', 0.05)) # g
  f.write(struct.pack('f', 0.05)) # b

  # Add the name in the custom data area as a way of documenting the origin,
  # padded to fill 88 characters, as per cyHair spec
  f.write("RealityHair".ljust(headerCustomDataLength))
  # ------------- End of header -------------  

def exportHairGroup(hairGroup, root):
  hairProp = hairGroup.HairProp()

  # Conversion from Poser Native Units to metric 1PNU = 262.128cm
  scale = 2.621280116332
  vertex = hairProp.Geometry().Vertices()

  numVertices = hairProp.Geometry().NumVertices())
  numPolys    = hairProp.Geometry().NumPolygons())
  numStrands  = hairProp.Geometry().Polygon(0).NumVertices()-1)

  f = open("%s/%s.hair" % (root, hairGroup.Name()),'wb')

  writeFileHeader(f, numVertices, numPolys, numStrands)

  hairData = {}
  # Points array
  hairData["p"] = []
  # Thickness array
  hairData["th"] = []
  # Transparency array
  hairData["tr"] = []

  # Points array
  verts = hairProp.Geometry().WorldVertices()
  for aVert in verts:
    # f.write(struct.pack('fff', aVert.X()*scale, -aVert.Z()*scale, aVert.Y()*scale)) # coords
    f.write(struct.pack('fff', aVert.X()*scale, -aVert.Z()*scale, aVert.Y()*scale)) # coords

  # Thickness array
  knots     = hairGroup.NumbVertsPerHair()
  rootWidth = hairGroup.RootWidth()/2000
  tipWidth  = hairGroup.TipWidth()/2000
  polys     = hairProp.Geometry().Polygons()

  for exPoly in polys:
    x=0
    for exVert in exPoly.Vertices():
  		curTh=rootWidth-(rootWidth-tipWidth)/knots*x
  		x=x+1
  		f.write(struct.pack('f', curTh))

  # Transparency array
  for exPoly in polys:
    x=0.0
    for exVert in exPoly.Vertices():
      f.write(struct.pack('f', x))
      x = x+(0.3/knots)		

  f.close()

def exportHairProp(actor):
  for numHairGroup in range(0, actor.NumHairGroups()):
    exportHairGroup(actor.HairGroup(numHairGroup),"/Users/paolo/Desktop/Hair")


# def main():
#   for exAct in poser.Scene().Actors():
#     if exAct.NumHairGroups() > 0 :
#       exportHairProp(exAct)

# main()


