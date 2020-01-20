#
# Copyright (c) Pret-a-3D/Paolo Ciccone. All rights reserved.
#

from Reality_services import *
from Reality import *

# To customize this script all you need to do is to 
# change the following variable
Re_sIBL_Map = ":Runtime:Textures:Reality:Chiricahua_Plaza:Chiricahua_Plaza.ibl"

# Set the IBL Map
Reality.Scene().setIBLImage(ReResolvePoserPath(Re_sIBL_Map).encode("utf8"))
