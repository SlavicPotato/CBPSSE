#!/usr/bin/python
import cbpinstaller
import os, sys
from cbpinstaller import CBPFomodGenerator, ReleaseData
# setup all the path schnitzels
print("initializing..")

installerPath = os.path.abspath(os.path.dirname(sys.argv[0]))
installerTempPath = os.path.abspath(installerPath+"\\"+"installerTmp")
relDict = {}

for item in sys.argv[1:len(sys.argv)]:
    #format is name|dll-abs-path
    tmpEntry = item.split("|")
    relName = tmpEntry[0]
    relDll  = tmpEntry[1]

    if os.path.exists(relDll):
        relDict[relName] = ReleaseData(relDll,
                                installerTempPath+"\\"+relName)
    else:
        sys.exit("no "+relDll+"\nAborting.",1)


fomodDegenerator = CBPFomodGenerator(installerPath,installerTempPath,relDict)
fomodDegenerator.run()
