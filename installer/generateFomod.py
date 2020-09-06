#!/usr/bin/python

import os
import sys
import shutil
import CppHeaderParser
from string import Template





#some boilerplate
NAME = "PotatoCBP"

RELEASE_DELIM = "|"
#targetPath is used to determine release path below
RELEASE_DLL = "CBP.dll"
VERSION_H = "..\\CBP\\version.h"
CONFIG_DIR= "..\\Config"
PLUGIN_VERSION_MAJOR = "PLUGIN_VERSION_MAJOR"
PLUGIN_VERSION_MINOR = "PLUGIN_VERSION_MINOR"
PLUGIN_VERSION_REVISION = "PLUGIN_VERSION_REVISION"

#template
INFO_XML = "info.xml"
MODULECONFIG_XML = "moduleConfig.xml"
PLUGIN_XML = "plugin.xml"
TEMPLATE_DIR = "templates"

# the actual installer build
TEMPDIR = "installerTmp"
BASE_DIR = "base"
FOMOD_DIR = "fomod"
PLUGIN_DESTINATION ="SKSE\\Plugins"
PLUGIN_FORMAT = "zip"

# A simple class to hold release data
class ReleaseData:
    #@param anOrigin is the absolute path to the DLL file
    #@param aTarget is where this should go in the installer temp directory #refactor later KWEK
    def __init__(self,anOrigin,aTarget):
        self.origin = anOrigin #the abs path of the DLL file
        self.target = aTarget #the target directory


class FomodGenerator:

    #keep it simple for now--refactor later as the need arises, and the dust settles
    # @param installerPath is the absolute path of the installer dir, usually "<CBP_SLN_ROOT>\installer"
    # @param installerTempPath is the absolute path of the installer temp dir, for working. Usually "<CBP_SLN_ROOT>\installer\installerTmp"
    # @param releaseDataSet is a simple dictionary
    # releaseName -> ReleaseData Object
    def __init__(self, installerPath, installerTempPath, releaseDataSet):
        self._releaseDictionary = releaseDataSet
        if len(self._releaseDictionary) < 1:
            self.exit("Must specify at least one release",1)

        self._installerPath = installerPath
        self._installerTempPath = installerTempPath

        ############# Deriving schtuff
        self._version = self.getReleaseVersion(""+self._installerPath + "\\" + VERSION_H)
        self._cbpConfigPath = self._installerPath + "\\" + CONFIG_DIR
        self._templatePath = self._installerPath+"\\"+TEMPLATE_DIR

        # the actual installer build dir structs

        self._fomodPath = self._installerTempPath+"\\"+FOMOD_DIR
        self._basePluginPath = self._installerTempPath + "\\" + BASE_DIR
        # installer file name. tentative convention: NAME__version(.zip) automatically added
        self._fomodInstallerFile = self._installerPath+"\\"+NAME+"_"+self._version



    def run(self):
        try:
            self.cleanup()
            self.createAndSetupPaths()
            print("buildInfoXML..")
            self.buildInfoXML()
            print("buildModuleConfigXML..")
            self.buildModuleConfigXML()
            print("packing Fomod..")
            self.packFomod()
            self.cleanup(False)
        except OSError:
            #uh oh--tidy up!
            self.exit("Generator run failed",1)


#######################################
#
    # leaves the zip file if it's set to False
    def cleanup(self,cleanAll=True):
        if os.path.exists(self._installerTempPath):
            shutil.rmtree(self._installerTempPath)
        if cleanAll and os.path.exists(self._fomodInstallerFile):
            os.remove(self._fomodInstallerFile)

    def exit(self,message,exitCode):
        self.cleanup()
        print(message)
        sys.exit(exitCode)

    #parse command line arguments and initialize paths

    def getReleaseVersion(self,hFile):
        try:
            cppHeader = CppHeaderParser.CppHeader(hFile)
        except CppHeaderParser.CppParseError as e:
            self.exit(e,1)

        defineDictionary = {}
        # parse define information -- rough is fine for now, just to get _version out
        for defineStatement in cppHeader.defines:
            ttmp = defineStatement.split(None,1)
            defineDictionary[ttmp[0]] = ttmp[1]

        return ""+( defineDictionary[PLUGIN_VERSION_MAJOR]+"."+
                defineDictionary[PLUGIN_VERSION_MINOR]+"."+
                defineDictionary[PLUGIN_VERSION_REVISION])

    def createAndSetupPaths(self):
        #INSTALLER WORK
        #move stuff over to the installer temp dir
        #create separate dirs for DLLs and move them accordingly
        for relData in self._releaseDictionary.values():
            os.makedirs(relData.target, exist_ok=True)
            shutil.copy2(relData.origin,relData.target)

        #move the config dir to the SKSE plugins location, offset by self._basePluginPath
        shutil.copytree(self._cbpConfigPath,self._basePluginPath+"\\"+PLUGIN_DESTINATION)

        #prep the fomod manifest dir
        os.makedirs(self._fomodPath, exist_ok=True)

    def processTemplate(self,templateFile,templateDictionary):
        filein = open(self._templatePath+"\\"+templateFile)
        src = Template(filein.read())
        filein.close()
        return src.substitute(templateDictionary)

    def getBoilerPlateDictionary(self):
        return { "name":NAME, "version":self._version}

    def buildInfoXML(self):
        tDict = self.getBoilerPlateDictionary()
        result = self.processTemplate(INFO_XML, tDict)

        fileout = open(self._fomodPath+"\\"+INFO_XML,"w")
        fileout.write(result)
        fileout.close()

    def buildPluginEntry(self,name):
        #name,source,destination,requireed

        #all is set optional for now--
        #Fomod has logic to enforce at least one selection on the parent level
        tDict = {
            "pluginDescription":name,
            "pluginName":name,
            "pluginSource":name,
            "pluginDestination":PLUGIN_DESTINATION,
            "isPluginRequired":"Optional"
        }

        return self.processTemplate(PLUGIN_XML,tDict)


    def buildModuleConfigXML(self):
        tDict = self.getBoilerPlateDictionary()
        tDict["moduleBase"] = BASE_DIR

        pluginEntries = "";
        for relName in self._releaseDictionary.keys():
            pluginEntries += self.buildPluginEntry(relName)

        tDict["plugins"] = pluginEntries

        result = self.processTemplate(MODULECONFIG_XML,tDict)

        fileout = open(self._fomodPath+"\\"+MODULECONFIG_XML,"w")
        fileout.write(result)
        fileout.close()

    def packFomod(self):
        shutil.make_archive(self._fomodInstallerFile,PLUGIN_FORMAT,self._installerTempPath)


###################################
# Installer Work

# setup all the path schnitzels
print("initializing..")

installerPath = os.path.abspath(os.path.dirname(sys.argv[0]))
installerTempPath = os.path.abspath(installerPath+"\\"+TEMPDIR)
relDict = {}

for item in sys.argv[1:len(sys.argv)]:
    #format is name|dll-abs-path
    tmpEntry = item.split(RELEASE_DELIM)
    relName = tmpEntry[0]
    relDll  = tmpEntry[1]

    if os.path.exists(relDll):
        relDict[relName] = ReleaseData(relDll,
                                installerTempPath+"\\"+relName)
    else:
        sys.exit("no "+relDll+"\nAborting.",1)

fomodDegenerator = FomodGenerator(installerPath,installerTempPath,relDict)
fomodDegenerator.run()


#apply dictionary to template files

#zip the dir - ZIP support comes standard with Python
#...but first some bookkeeping
# try:
#     os.remove(_fomodInstallerFile)
# except OSError:
#     pass

#cleanup - maybe do better exception handling later
