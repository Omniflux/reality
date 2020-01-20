##
# \file
#  Reality plug-in
#  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
#

import poser

sc = poser.Scene()
sc.LoadLibraryProp(":Runtime:Libraries:light:Reality:Mesh Light.pp2")
l = sc.CurrentActor()
l.SetParameter("xRotate", -64)
l.SetParameter("yRotate", 0)
l.SetParameter("zRotate", 0)
l.SetParameter("xTran", 0.0)
l.SetParameter("yTran", 0.989)
l.SetParameter("zTran", 0.412)
l.SetParameter("xScale", 1.78)
l.SetParameter("yScale", 1.78)
