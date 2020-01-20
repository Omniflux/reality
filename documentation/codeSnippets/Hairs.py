#-----------------------------------------------------------------------------
# This sample file will copy  keyframes (and values) from one parameter to another
#  across different objects on different figures.
#
#  Several named parameters are copied from the Head of Figure 2 to the
#  Head of Figure 1
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# These are the parm names to copy from Head of Figure 2 to Head of Figure 1
#-----------------------------------------------------------------------------
copyParms = ["Blink Right", "Blink Left", "Side-Side"]


import poser
import os
import struct
import random



def ExpHair(Hair):
  HairProp = Hair.HairProp()
  hNodes=HairProp.Materials()
  HairShader=hNodes[0].ShaderTree()
  for exShader in HairShader.Nodes():
    if exShader.Type()=='hair':
      HairColors=exShader

  cRoot=HairColors.InputByInternalName('Root_Color').Value()
  cTip=HairColors.InputByInternalName('Tip_Color').Value()

                             
  worldVert = HairProp.Geometry().Vertices()
  Polygons = HairProp.Geometry().Polygons()
  rootW = Hair.RootWidth()/2000
  TipW = Hair.TipWidth()/2000
  Knots = Hair.NumbVertsPerHair()
  rR=cRoot[0]
  rG=cRoot[1]
  rB=cRoot[2]
  tR=cTip[0]
  tG=cTip[1]
  tB=cTip[2]

  scale=2.621280116332
  vertex = HairProp.Geometry().Vertices()
  f = open('f:/bpdozer/' + Hair.Name() + '.hair','wb')
  f.write(b'\x48\x41\x49\x52')
  f.write(struct.pack('I',HairProp.Geometry().NumPolygons()))
  f.write(struct.pack('I',HairProp.Geometry().NumVertices()))
  f.write(struct.pack('I',30)) #bits
  f.write(struct.pack('I',HairProp.Geometry().Polygon(1).NumVertices()-1)) # strands number
  f.write(struct.pack('f',0.001)) # thikness
  f.write(struct.pack('f',0.0)) # opacity
  f.write(struct.pack('f',1.0)) # r
  f.write(struct.pack('f',1.0)) # g
  f.write(struct.pack('f',1.0)) # b
  print Hair.Name(), '-', HairProp.Geometry().NumPolygons()
  f.write(Hair.Name())
  for x in range (1,89-len(Hair.Name())):
	f.write(b'\x20')


  for exVert in HairProp.Geometry().WorldVertices():
    f.write(struct.pack('fff', exVert.X()*scale, -exVert.Z()*scale, exVert.Y()*scale)) # coords

  for exPoly in HairProp.Geometry().Polygons():
	x=0
	for exVert in exPoly.Vertices():
		curTh=rootW-(rootW-TipW)/Knots*x
		x=x+1
		f.write(struct.pack('f', curTh))         #thiknes

  for exPoly in HairProp.Geometry().Polygons():
	x=0.0
	for exVert in exPoly.Vertices():
		f.write(struct.pack('f', x))         #opacity
		x=x+(0.3/Knots)

  for exPoly in HairProp.Geometry().Polygons(): #color
	x=0
	for exVert in exPoly.Vertices():
		cR=rR-(rR-tR)/Knots*x
		cG=rG-(rG-tG)/Knots*x
		cB=rB-(rB-tB)/Knots*x
		x=x+1
		f.write(struct.pack('fff', random.uniform(cR-0.05,cR+0.05), random.uniform(cG-0.05,cG+0.05), random.uniform(cB-0.05,cB+0.05)))  #rgb
		



  f.close()

def listHairs(actor):
  for exAct in range(0,actor.NumHairGroups()):
    ExpHair(actor.HairGroup(exAct))


def main():
  scene = poser.Scene()
  actor = scene.CurrentActor()
  for exAct in range(0,actor.NumHairGroups()):
    ExpHair(actor.HairGroup(exAct))
  print 'Done'

  actor = scene.CurrentActor()
  for exAct in scene.Actors():
    if exAct.NumHairGroups()>0 :
      listHairs(exAct)


main()


