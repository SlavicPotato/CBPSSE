#!/usr/bin/python

import os
import sys
import shutil
import CppHeaderParser
import pyfomod


#some boilerplate #TODO: maybe put in an external file
NAME = "PotatoCBP"
AUTHOR = "SlavicPotato(dev), KwekMaster(qa,FOMod)"
WEBSITE = "https://github.com/SlavicPotato/CBPSSE"
DESCRIPTION = "PotatoCBP is a advanced version of CBP Physics for Skyrim SE by polygonhell"

#targetPath is used to determine release path below
RELEASE_DLL = "CBP.dll"
VERSION_H = "..\\CBP\\version.h"
CONFIG_DIR= "..\\Config"
PLUGIN_VERSION_MAJOR = "PLUGIN_VERSION_MAJOR"
PLUGIN_VERSION_MINOR = "PLUGIN_VERSION_MINOR"
PLUGIN_VERSION_REVISION = "PLUGIN_VERSION_REVISION"

# the actual installer build
INSTALLER_DIR = "installer"
INSTALLER_TEMP_DIR = "installerTmp"
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


class CBPFomodGenerator:

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

        # the actual installer build dir structs
        self._fomodPath = self._installerTempPath+"\\"+FOMOD_DIR
        self._basePluginPath = self._installerTempPath + "\\" + BASE_DIR
        # installer file name. tentative convention: NAME__version(.zip) automatically added
        self._fomodInstallerFile = self._installerPath+"\\"+NAME+"_"+self._version



    def run(self):
        try:
            self.cleanup()
            # TODO: log this out
            #print("creating, setup files and directories")
            self.createAndSetupPaths()
            #print("creating Fomod manifest")
            self.constructFomodManifest()
            #print("packing Fomod..")
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

    #when this gets called, an exception will be thrown!
    def exit(self,message,exitCode):
        self.cleanup()
        raise Exception(message)

    def constructPreamblePage(self):
        preamblePage = pyfomod.Page()
        preamblePage.name = "Before we begin.."

        preambleGroup = pyfomod.Group()
        preambleGroup.name = "Make sure you have all the dependencies!"
        preambleGroup.type = pyfomod.GroupType.EXACTLYONE


        preambleOption = pyfomod.Option()
        preambleOption.name = "Yes I have all the dependencies!"
        preambleOption.description = "Please make sure you have SKSE version later than 1.5.23 and SKSE Address Library Installed!"

        preambleOption.files = pyfomod.Files()
        preambleOption.files[BASE_DIR+"\\"] = "";

        preambleGroup.append(preambleOption)
        preamblePage.append(preambleGroup)

        return preamblePage

    def constructDllTypePage(self):
        dllTypePage = pyfomod.Page()
        dllTypePage.name = "Your CPU instruction set"

        dllTypeGroup = pyfomod.Group()
        dllTypeGroup.name = "Select An Option:"
        dllTypeGroup.type = pyfomod.GroupType.EXACTLYONE

        for releaseName in self._releaseDictionary.keys():
            dllTypeOption = pyfomod.Option()
            dllTypeOption.name = releaseName
            dllTypeOption.description = releaseName
            dllTypeOption.files = pyfomod.Files()
            dllTypeOption.files[releaseName+"\\"] = PLUGIN_DESTINATION;
            dllTypeGroup.append(dllTypeOption)

        dllTypePage.append(dllTypeGroup)

        return dllTypePage

    def constructFomodManifest(self):
        installerRoot = pyfomod.Root()
        installerRoot.name = NAME
        installerRoot.author = AUTHOR
        installerRoot.version = self._version
        installerRoot.description = DESCRIPTION
        installerRoot.website = WEBSITE

        installerRoot.pages.order = pyfomod.Order.EXPLICIT

        #installer will have 3 Pages
        #preamblePage
        installerRoot.pages.append(self.constructPreamblePage());
        #dllTypePage
        installerRoot.pages.append(self.constructDllTypePage());

        #write the fomod xmls using pyfomod
        pyfomod.write(installerRoot, self._installerTempPath)


    def getReleaseVersion(self,hFile):

        try:
            cppHeader = CppHeaderParser.CppHeader(hFile)
        except CppHeaderParser.CppParseError as e:
            self.exit(self,"Error occured when attempting to parse "+hFile,1)

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

#The factory method!
# SAMPLE USAGE:
#
# import installer.cbpinstaller
# ...
# package_targets = {}
#
# for e in CONFIGS:
#     package_targets[e]= "d:\\installerTest\\"+e+DLL
#
# try:
#     fomodGenerator = installer.cbpinstaller.createCBPFomodGenerator(SLN_ROOT,package_targets)
#     fomodGenerator.run()
# except Exception as e:
#     print("Oh no!",e)
#
# print('OK')

def createCBPFomodGenerator(cbp_sln_root, releases):
    relDict = {}
    installerDir = os.path.abspath(cbp_sln_root+"\\"+INSTALLER_DIR);
    installerTmpDir = os.path.abspath(cbp_sln_root+"\\"+INSTALLER_TEMP_DIR);
    for relName, relDll in releases.items():
        if os.path.exists(relDll):
            relDict[relName] = ReleaseData(relDll, installerTmpDir+"\\"+relName)
        else:
            raise Exception("The path: "+relDll+" does not exist. Aborting.")

    return CBPFomodGenerator(installerDir, installerTmpDir, relDict)
