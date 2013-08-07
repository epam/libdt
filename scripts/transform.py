#!/usr/bin/env python
#Creates from windowsZones.xml(http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml)
#header file with mapping which can be used to map Windows Standard Times to olsen database
#timezones names and/or territory abbreviations
#
#Tryes to get windowsZones.xml by internet, another case tryes to get it by local file which must be
#pointed as first argument of script, if argument not setted, tryes to find it by relative path
#'../resources/windowsZones.xml'

import sys
import os
import string
import urllib2
from xml.dom.minidom import *

def getXml():
    strUrl = 'http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml' 
    print '//Generated from windowsZones.xml'
    try:
        response = urllib2.urlopen(strUrl)
        xml = parseString(response.read())
        print '//Obtained by internet from http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml'
    except urllib2.URLError:
        strXmlFile = '../resources/windowsZones.xml'
        if len(sys.argv) > 1 :
                strXmlFile = sys.argv[1]
        xml = parse(strXmlFile)
        print '//Obtained from local file'
    return xml

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
    return strMapZonesJoined

print 'struct tz_unicode_mapping {char *other; char* territory; char* type;};'
print 'struct tz_unicode_mapping tz_unicode_map[] = {'
try:
    print parseMapZones(getXml())
except :
    print '#error Cant generate timezones mapping watch the script ' +\
            os.path.abspath(sys.argv[0])
print '};'
