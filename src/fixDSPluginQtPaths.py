# 
# This script changes the install names of the Qt libraries embedded in the Reality for DS 
# library. It is executed asd a post action for the Mac OS version. See CMakeList.txt
# 
import re, sys
from subprocess import *

# This is expected to be the full path of Reality_3_DS64.dylib
libraryFile = sys.argv[1]

print "Setting relative paths for Qt library for " + libraryFile
# Get all the names of the Qt libraries linked by our library
libraryNames =  check_output("otool -L %s | grep Qt" % libraryFile, shell=True,stderr=STDOUT)

matcher = re.compile(r'^\s+(\S+)\s', re.MULTILINE)

libList = matcher.findall(libraryNames)

# Extract the name of the library without the path that precedes it
libNameShortner = re.compile(r"/lib/(.+)")
for libName in libList:
  m = libNameShortner.search(libName)
  if m:
    shortLibName = m.group(1)
    print "%s => %s" % (libName, shortLibName)
    call("install_name_tool -change \"%s\" \"@rpath/%s\" %s" % (libName, shortLibName, libraryFile), shell=True)
