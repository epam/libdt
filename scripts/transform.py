#!/usr/bin/env python
# Creates from windowsZones.xml (http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml)
# header file with mapping which can be used to map Windows Standard Times to olsen database
# timezones names and/or territory abbreviations
#
# Tryes to get windowsZones.xml by internet, another case tryes to get it by local file which must be
# pointed as first argument of script, if argument not setted, tryes to find it by relative path
# '../resources/windowsZones.xml'

import sys
import os
import string
import urllib2
from xml.dom.minidom import *

def getXmlFromFile():
    strXmlFile = '../resources/windowsZones.xml'
    if len(sys.argv) > 2 :
        strXmlFile = sys.argv[2]
    xml = parse(strXmlFile)
    print '//Obtained from local file'
    return xml

def getXmlFromWeb():
    strUrl = 'http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml' 
    try:
        response = urllib2.urlopen(strUrl)
        xml = parseString(response.read())
        print '//Obtained by internet from http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml'
    except urllib2.URLError:
        xml = getXmlFromFile()
    return xml

def getXml():
    print '//Generated from windowsZones.xml'
    if len(sys.argv) > 1 :
        if sys.argv[1] == "--remote":
            return getXmlFromWeb()
    return getXmlFromFile()

def parseMapZone(mapZone):
    strType = mapZone._attrs['type'].value 
    strOther = mapZone._attrs['other'].value 
    strTerritory = mapZone._attrs['territory'].value 
    strTypes = string.split(strType, ' ')
    strMapZones = []
    for strZoneType in strTypes :
        strMapZones.append('{"' + strOther + '", "' + strTerritory + '", "' +\
        strZoneType + '"}')
    strTypes
    strMapZone = string.join(strMapZones, ',\n')   
    return strMapZone

def parseMapZones(xml):
    strMapZones = [] 
    mapZones = xml.getElementsByTagName('mapZone')
    for mapZone in mapZones : 
        strMapZones.append(parseMapZone(mapZone))
    mapZones
    strMapZonesJoined = string.join(strMapZones, ',\n')
    return strMapZonesJoined, len(mapZones)

def help():
    print 'usage: ', sys.argv[0], ' [--remote| --local [local xml file]]'

print 'struct tz_unicode_mapping {const char *other; const char* territory; const char* type;};'
print 'const struct tz_unicode_mapping tz_unicode_map[] = {'
zonesCount = 0
try:
    zonesStr, zonesCount = parseMapZones(getXml())
    print zonesStr
except :
    print '#error Cant generate timezones mapping watch the script ' +\
            os.path.abspath(sys.argv[0])
print '};'
print 'const size_t tz_unicode_map_size = ', zonesCount, ';\n'
