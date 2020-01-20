##
# \file
# Reality plug-in. This file defines the UUID for Reality objects
# 
# Legal:
# Copyright (c) Pret-a-3D/Paolo Ciccone 2012-2013. All rights reserved.
#

import uuid, poser, os, binascii
import ReTools

# CRC-CCITT implemetnation that returns a 5-digit crc value
POLYNOMIAL = 0x1021
PRESET = 0

# CRC table initialization
def _initial(c):
    crc = 0
    c = c << 8
    for j in range(8):
        if (crc ^ c) & 0x8000:
            crc = (crc << 1) ^ POLYNOMIAL
        else:
            crc = crc << 1
        c = c << 1
    return crc

_tab = [ _initial(i) for i in range(256) ]

def _update_crc(crc, c):
    cc = 0xff & c

    tmp = (crc >> 8) ^ cc
    crc = (crc << 8) ^ _tab[tmp & 0xff]
    crc = crc & 0xffff

    return crc

# CRC calculation
def crc(str):
    crc = PRESET
    for c in str:
        crc = _update_crc(crc, ord(c))
    return crc

def makeObjectUID( obj ):
  """
  Given a Poser object, this function returns a unique ID for the object
  calculated by using the object's name and by appending the CRC value
  of a GUID calculated at the moment the function is called. The use of the
  CRC value is to condense the otherwise very long GUID string.
  """
  return "%s-%d" % (ReTools.getObjectGeometryID(obj), crc(uuid.uuid1().hex))

def getUIDFromObject( obj ):
  pass
