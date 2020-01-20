#
# Copyright (c) 2013 Pret-a-3D/Paolo Ciccone. All rights reserved.
#
#
# This script changes the YON parameter for the current camera so that it can
# "see" as far as the Reality IBL Preview Sphere is. Otherwise the spher will be
# too large to be seen by the default camera settings and the user will not be
# able to adjust the background for IBL lighting.
#
import poser

newYON = 1000

try:
  cc = poser.Scene().CurrentCamera()
  if cc:
    currentYON = cc.ParameterByCode(poser.kParmCodeYON)
    if currentYON and currentYON.Value() < newYON:
      cc.SetParameter(currentYON.Name(), newYON)
except Exception, e:
  print "Error happened while setting the YON parameter for camera %s: %s " %(cc.Name(), e.message)
