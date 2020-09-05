#!/usr/bin/env python

import os
import shutil
import subprocess

MSBUILD_PATH = 'C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\msbuild.exe'
SLN_ROOT = 'c:\\Users\\A\\Source\\repos\\CBP'

# relative to SLN_ROOT
OUT='tmp'
FOMOD='installer\\generateFomod.py'
SLN = 'CBP.sln'
DLL='CBP.dll'

REBUILD=False
CONFIGS= ['ReleaseAVX2 MT', 'Release MT']

def cleanup(p):
    for f in os.scandir(p):
        try:
            if os.path.isdir(f):
                shutil.rmtree(f)
            elif os.path.isfile(f) or os.path.islink(f):
                os.unlink(f)

        except BaseException as e:
            print('exception during cleanup: {}'.format(e))

def prepare(p):
    if os.path.exists(p):
        if not os.path.isdir(p):
            raise Exception('Invalid output path')

        cleanup(p)
    else:
        os.mkdir(p)

def safe_mkdir(p):
    try:
        os.mkdir(p)
    except FileExistsError:
        pass

def test_file(p):
    return os.path.exists(p) and os.path.isfile(p) 

def build_package(fmscript, targets):
    cmd = [ 'python', fmscript ]
    cmd.extend(targets)

    print(cmd)

    r = subprocess.run(cmd)
    if r.returncode != 0:
        raise Exception('Could not build the package')
        
def build(cfg, bc, path, rebuild):
    args = [ '-p:Configuration={};OutDir={}'.format(cfg, path) ]
    if rebuild:
        args.append('-t:Rebuild')

    cmd = bc
    cmd.extend(args)

    print('Building {} ..'.format(cfg))

    r = subprocess.run(cmd)
    if r.returncode != 0:
        raise Exception('Build failed')

OUTPUT_PATH = os.path.join(SLN_ROOT, OUT)
basecmd = [MSBUILD_PATH, os.path.join(SLN_ROOT, SLN)]

prepare(OUTPUT_PATH)

package_targets = []

for e in CONFIGS:
    path = os.path.join(OUTPUT_PATH, e)
    safe_mkdir(path)
    build(e, basecmd, path, REBUILD)

    dll = os.path.join(path, DLL)
    if not test_file(dll):
        raise Exception('Couldn\'t find dll: {}'.format(dll))

    package_targets.append('{}|{}'.format(e, dll))

build_package(os.path.join(SLN_ROOT, FOMOD), package_targets)

print('OK')
