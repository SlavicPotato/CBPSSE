#!/usr/bin/python

import os
import shutil
import pathlib
import CppHeaderParser
import pyfomod
import git

PACKAGE_NAME = 'CBP-A'
PACKAGE_AUTHOR = 'SlavicPotato'
PACKAGE_DESCRIPTION = 'Makes stuff jiggle'
GITROOT = '..\\'

VERSION_H = '..\\CBP\\version.h'
PLUGIN_VERSION_MAJOR = 'PLUGIN_VERSION_MAJOR'
PLUGIN_VERSION_MINOR = 'PLUGIN_VERSION_MINOR'
PLUGIN_VERSION_REVISION = 'PLUGIN_VERSION_REVISION'

PACKAGE_PATH = 'package'
OUT_PATH = 'releases'
PACKAGE_COMPRESSION_FORMAT = 'zip'

PKG_BIN_MAP = {
    'Dep-Generic': 'Generic x64',
    'Dep-AVX': 'AVX',
    'Dep-AVX2': 'AVX2'
}

class FomodGenerator:
    def __init__(self, releaseDataSet):
        assert type(releaseDataSet) == dict
        assert len(releaseDataSet)
        self._releaseDictionary = releaseDataSet

    def run(self):

        root_path = pathlib.Path(__file__).parent
        package_path = root_path / PACKAGE_PATH

        root = pyfomod.parse(package_path)
        
        root.name = PACKAGE_NAME
        root.version = self.getReleaseVersion(root_path / VERSION_H)
        root.description = PACKAGE_DESCRIPTION
        root.author = PACKAGE_AUTHOR
        
        self.copy(root, package_path)
        self.validate(root)

        pyfomod.write(root, package_path)

        self.packFomod(
            root_path / os.path.join(OUT_PATH, '{}_{}-{}'.format(
                root.name, root.version, self.getCommitSHA(root_path))), 
            package_path)

    def getCommitSHA(self, root_path):
        repo = git.Repo(root_path / GITROOT)
        commits = list(repo.iter_commits("master", max_count=1))

        assert len(commits) == 1

        return commits[0].hexsha[0:8]

    def validate(self, root):
        f = False
        for warning in root.validate():
            print('{}{}: {} - {}'.format(
                'CRITICAL: ' if warning.critical else '', 
                warning.elem, warning.title, warning.msg))

            if warning.critical:
                f = True

        if f:
            raise Exception('Bad config')

    def getOpts(self, root):
        r = {}

        for p in root.pages:
            if p.name == 'Plugin':
                assert len(p) == 1
                for g in p:
                    for o in g:
                        r[o.name] = o
                break

        return r

    def copy(self, root, root_path):
        opts = self.getOpts(root)

        if len(opts) != len(self._releaseDictionary):
            print('WARNING: fomod binary count doesn\'t match release dict')

        c = []

        for k, v in self._releaseDictionary.items():
            if k not in PKG_BIN_MAP:
                print('Unknown key: {}'.format(k))
                continue
            
            n = PKG_BIN_MAP[k]

            if not n in opts:
                print('Not tracked: {}'.format(n))
                continue

            f = opts[n].files
            assert len(f) == 1

            for l in f:
                p = os.path.join(root_path, l)
                print(v, '->', p)
                c.append((v, p))
            
        for v in c:
            shutil.copyfile(*v)
        
    def getReleaseVersion(self,hFile):
        cppHeader = CppHeaderParser.CppHeader(hFile)

        defineDictionary = {}
        # parse define information -- rough is fine for now, just to get _version out
        for defineStatement in cppHeader.defines:
            ttmp = defineStatement.split(None,1)
            defineDictionary[ttmp[0]] = ttmp[1]

        return (defineDictionary[PLUGIN_VERSION_MAJOR]+'.'+
                defineDictionary[PLUGIN_VERSION_MINOR]+'.'+
                defineDictionary[PLUGIN_VERSION_REVISION])

    def packFomod(self, file, path):
        shutil.make_archive(file, PACKAGE_COMPRESSION_FORMAT, path)

