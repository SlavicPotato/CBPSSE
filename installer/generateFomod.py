#!/usr/bin/python

import os
import sys
import shutil
import CppHeaderParser
from string import Template

#some boilerplate
NAME = "PotatoCBP"
RELEASE_DLL = "..\\x64\ReleaseAVX2 MT\CBP.dll"
VERSION_H = "..\\CBP\\version.h"
CONFIG_DIR= "..\\CBP\\Config"
PLUGIN_VERSION_MAJOR = "PLUGIN_VERSION_MAJOR"
PLUGIN_VERSION_MINOR = "PLUGIN_VERSION_MINOR"
PLUGIN_VERSION_REVISION = "PLUGIN_VERSION_REVISION"
INFO_XML = "info.xml"
MODULECONFIG_XML = "moduleConfig.xml"
TEMPLATE_DIR = "templates"

# the actual installer build
TEMPDIR = "_installerTmp"
FOMOD_DIR = "fomod"
PLUGIN_DESTINATION ="base\\SKSE\Plugins"
PLUGIN_FORMAT = "zip"


#setup pathing - maybe tweak for post-build integration later
scriptPath = os.path.abspath(os.path.dirname(sys.argv[0]))
versionHeaderFile = os.path.abspath(scriptPath + "\\" + VERSION_H)
releaseDllFile = os.path.abspath(scriptPath + "\\" + RELEASE_DLL)
configPath = os.path.abspath(scriptPath + "\\" + CONFIG_DIR)
templatePath = os.path.abspath(scriptPath+"\\"+TEMPLATE_DIR)

# the actual installer build dir structs
tempPath = os.path.abspath(scriptPath+"\\"+TEMPDIR)
fomodPath = os.path.abspath(tempPath+"\\"+FOMOD_DIR)
basePluginPath = os.path.abspath(tempPath + "\\" + PLUGIN_DESTINATION)

#obtain version data
try:
    cppHeader = CppHeaderParser.CppHeader(versionHeaderFile)
except CppHeaderParser.CppParseError as e:
    print(e)
    sys.exit(1)

defineDictionary = {}
# parse define information -- rough is fine for now, just to get version out
for defineStatement in cppHeader.defines:
    ttmp = defineStatement.split(None,1)
    defineDictionary[ttmp[0]] = ttmp[1]

version = ""+( defineDictionary[PLUGIN_VERSION_MAJOR]+"."+
        defineDictionary[PLUGIN_VERSION_MINOR]+"."+
        defineDictionary[PLUGIN_VERSION_REVISION])



# build template dictionary
templateDictionary = { 'name':NAME, 'version':version}
# installer file name. tentative convention: NAME_version(.zip) automatically added
fomodInstallerFile = os.path.abspath(scriptPath+"\\"+NAME+"_"+version)



#INSTALLER WORK
#os.makedirs(basePluginPath, exist_ok=True)

#move stuff over to the installer temp dir
shutil.copytree(configPath,basePluginPath)
shutil.copy2(releaseDllFile,basePluginPath)

os.makedirs(fomodPath, exist_ok=True)

#open the template files
templateFiles = [INFO_XML,MODULECONFIG_XML]

#apply dictionary to template files
for aFile in templateFiles:
    filein = open(templatePath+"\\"+aFile)
    src = Template(filein.read())
    filein.close()
    result = src.substitute(templateDictionary)
    fileout = open(fomodPath+"\\"+aFile,"w")
    fileout.write(result)
    fileout.close()

#zip the dir - ZIP support comes standard with Python
#...but first some bookkeeping
try:
    os.remove(fomodInstallerFile)
except OSError:
    pass

shutil.make_archive(fomodInstallerFile,PLUGIN_FORMAT,tempPath)

#cleanup - maybe do better exception handling later
shutil.rmtree(tempPath)
