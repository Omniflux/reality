#!/usr/bin/python
#! This script generates a build number for Reality and it updates the 
#! header file passed as a parameter.
#!
#! The build number is generated using the followin formula:
#!   TWO_DIGIT_YEAR concat DAY_OF_THE_YEAR concat TIME_IN_15TH_OF_DAY
#!
#! The first number is the year of the century expressed as two digits.
#! 2015 becomes 15 and so on.
#!
#! The DAY_OF_THE_YEAR component is the cardinality of the current day. 
#! For example, February 1st is day 32. The number is zero-padded to three
#! digits.
#!
#! Lastly, we concatemnate the hour of the day but we keep that to a single
#! digit expressed in hexadeciaml. To do so we represent the hour as a 
#! 15th of the day instead of the common 24th of a day. 
#!
#! The time is calcalated in UTC, so it's not locale dependent. Any build
#! machine, regardless, where it is, will return the same exact build number
#! at any given time.
#!
import time,argparse,re

argParser = argparse.ArgumentParser()
argParser.add_argument("infile", help="The name of the C++ header file to update")
argParser.add_argument("-r", "--read", 
                       help="Read the build number from a file, instead of generating a new one.")
argParser.add_argument("-w", "--write", 
                       help="generates a new build number then it writes to the file specified.")
argParser.add_argument("--notime", action="store_true",
                       help="Don't add the hour to the build number")
args = argParser.parse_args()

#! Generate the build number
def makeBuildNumber():
  global args
  t = time.gmtime()
  if args.notime:
    return "%d%03d" % (t.tm_year-2000, t.tm_yday)    
  return "%d%03d%X" % (t.tm_year-2000, t.tm_yday, int(t.tm_hour*15/24))

if args.read:
  try:
    bnFile = open(args.read,"r")
    buildNo = bnFile.read()
    bnFile.close()
  except:
    print("Error: could not read file %s" % args.read)
    buildNo = makeBuildNumber()
else:
  buildNo = makeBuildNumber()

isOK = True

#! Open the header file and read the content
try:
  headerFile = open(args.infile,"r")
  headerData = headerFile.read()
  print("----------------------------------")
  print("Build number:%s" % buildNo)
  print("----------------------------------")
  headerFile.close()

except:
  print("Error: could not read file %s" % args.infile)
  isOK = False

#! Replace the build number with what we have generated
if isOK:
  bnRE = re.compile(r"^const unsigned int REALITY_BUILD_NUMBER\s+=.+$", re.MULTILINE)

  replacement = "const unsigned int REALITY_BUILD_NUMBER = %s;" % buildNo
  newHeader = bnRE.sub(replacement, headerData)

  #! Save the header file with the change
  try:
    headerFile = open(args.infile,"w")
    headerFile.write(newHeader)
    headerFile.close()
  except:
    print("Error: could not write the header file %s" % args.infile)

if args.write:
  bnFile = open(args.write,"w")
  bnFile.write(buildNo)
  bnFile.close()
